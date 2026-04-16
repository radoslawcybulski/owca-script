#include "stdafx.h"
#include "executor.h"
#include "vm.h"
#include "object.h"
#include "runtime_function.h"
#include "iterator.h"
#include "exec_buffer.h"
#include "executor_compare.h"
#include "range.h"
#include "string.h"
#include "array.h"
#include "tuple.h"
#include "dictionary.h"
#include "ast_function.h"
#include "exception.h"
#include <utility>

namespace OwcaScript::Internal {
    static thread_local OwcaValue temporary_exception_return_value;

    Executor::Executor(VM *vm) : vm(vm), stack_top_level_index(vm->current_stack_trace_index) {
    }

    ExecutionFrame &Executor::currently_executing_frame() {
        return vm->stacktrace[vm->current_stack_trace_index - 1];
    }
    ExecutionFrame &Executor::just_executed_executing_frame() {
        return vm->stacktrace[vm->current_stack_trace_index];
    }
    ExecutionFrame &Executor::push_new_frame() {
        if (vm->stacktrace.size() == vm->current_stack_trace_index) {
            vm->stacktrace.emplace_back();
        }
        else {
            vm->stacktrace[vm->current_stack_trace_index].clear();
        }
        ++vm->current_stack_trace_index;
        return currently_executing_frame();
    }
    void Executor::pop_frame() {
        auto &frame = currently_executing_frame();
        if (frame.constructor_move_self_to_return_value) {
            assert(frame.return_value);
            *frame.return_value = std::move(frame.values[0]);
        }
        if (frame.is_iterator) {
            assert(frame.return_value);
            assert(frame.iterator_object);
            auto iter = frame.iterator_object->internal_value();

            if (frame.return_value->kind() == OwcaValueKind::Completed) {
                iter->generator = {};
                iter->completed = true;
            }
            else {
                std::swap(iter->frame, frame);
            }
        }
        --vm->current_stack_trace_index;
    }
    void Executor::process_thrown_exception()
    {
        assert(exception_in_progress);
        while(!completed()) {
            auto &frame = currently_executing_frame();
            while(!frame.states.empty()) {
                if (auto state = std::get_if<ExecutionFrame::TryCatchState>(&frame.states.back())) {
                    frame.code_position = state->catches_pos;
                    return;
                }
                frame.states.pop_back();
            }
        }
        auto z = *exception_in_progress;
        exception_in_progress.reset();
        throw ;
    }
    bool Executor::completed() const {
        return vm->current_stack_trace_index <= stack_top_level_index;
    }

    void Executor::push_value(OwcaValue value) {
        vm->stacktrace[vm->current_stack_trace_index - 1].temporaries.push_back(std::move(value));
    }
    void Executor::pop_values(size_t count) {
        assert(count <= vm->stacktrace[vm->current_stack_trace_index - 1].temporaries.size());
        vm->stacktrace[vm->current_stack_trace_index - 1].temporaries.resize(vm->stacktrace[vm->current_stack_trace_index - 1].temporaries.size() - count);
    }
    OwcaValue &Executor::peek_value(size_t offset) const {
        assert(offset <= vm->stacktrace[vm->current_stack_trace_index - 1].temporaries.size());
        auto index = vm->stacktrace[vm->current_stack_trace_index - 1].temporaries.size() - offset;
        return vm->stacktrace[vm->current_stack_trace_index - 1].temporaries[index];
    }
    OwcaValue &Executor::peek_value_and_make_top(size_t offset) const {
        assert(offset <= vm->stacktrace[vm->current_stack_trace_index - 1].temporaries.size());
        auto index = vm->stacktrace[vm->current_stack_trace_index - 1].temporaries.size() - offset;
        vm->stacktrace[vm->current_stack_trace_index - 1].temporaries.resize(index + 1);
        return vm->stacktrace[vm->current_stack_trace_index - 1].temporaries[index];
    }
    std::span<OwcaValue> Executor::peek_values(size_t offset, size_t count) const {
        assert(offset >= count);
        assert(offset <= vm->stacktrace[vm->current_stack_trace_index - 1].temporaries.size());
        auto index = vm->stacktrace[vm->current_stack_trace_index - 1].temporaries.size() - offset;
        return { vm->stacktrace[vm->current_stack_trace_index - 1].temporaries.data() + index, count };
    }
    
	static std::tuple<Number, Number, Number> parse_key(VM *vm, OwcaValue v, OwcaValue key, Number size) {
		return key.visit(
			[&](Number o) -> std::tuple<Number, Number, Number> {
				auto v = key.as_int(vm);
				if (v < 0) v += size;
				if (v < 0 || v >= size) {
					vm->throw_index_out_of_range(std::format("index value {} is out of range for object of size {}", key, size));
				}
				return std::make_tuple(v, v + 1, 0);
			},
			[&](OwcaRange o) -> std::tuple<Number, Number, Number> {
				auto lower = o.lower();
				auto upper = o.upper();
				auto step = o.step();
				if (lower < 0) lower += size;
				if (upper < 0) upper += size;
				if (step > 0) {
					if (lower >= upper) return { 0, 0, 1 };
					if (upper <= 0) return { 0, 0, 1 };
					if (lower >= size) return { 0, 0, 1 };
					if (lower < 0) {
						auto skip = std::max(Number(0), std::floor(-lower / step) - 1);
						lower += skip * step;
					}
					if (step == 1 && lower < 0) {
						lower = 0;
						if (upper < 0) upper = 0;
					}
					if (upper > size) upper = size;
					return std::make_tuple(lower, upper, step);
				}
				else {
					if (lower <= upper) return { 0, 0, 1 };
					if (upper >= size) return { 0, 0, 1 };
					if (lower < 0) return { 0, 0, 1 };
					if (lower > size) {
						auto skip = std::max(Number(0), std::floor((lower - size) / -step) - 1);
						lower -= skip * step;
					}
					if (upper < 0) upper = Number{ -1 };
					return std::make_tuple(lower, upper, step);
				}
			},
			[&](const auto&) -> std::tuple<Number, Number, Number> {
				vm->throw_value_not_indexable(v.type(), key.type());
			}
		);
	}
	static size_t verify_key(VM *vm, Number v, size_t size, OwcaValue orig_key, std::string_view name) {
		if (v < 0 || v >= (Number)size)
			vm->throw_index_out_of_range(std::format("index value {} is out of range for {} of size {}", orig_key, name, size));
		auto v2 = (size_t)v;
		if (v2 != v)
			vm->throw_index_out_of_range(std::format("index value {} is out of range for {} of size {} - size_t overflows", orig_key, name, size));
		return v2;
	}
	static std::pair<size_t, size_t> verify_key(VM *vm, OwcaRange k, size_t size, OwcaValue orig_key, std::string_view name) {
		auto v1 = k.lower();
		auto v2 = k.upper();
		if (v2 <= v1)
			return { 0, 0 };
		if (v1 < 0) v1 = 0;
		if (v2 > (Number)size) v2 = size;
		size_t v3 = (size_t)v1, v4 = (size_t)v2;
		if (v3 != v1 || v2 != v4) {
			vm->throw_index_out_of_range(std::format("index values {} is out of range for array of size {} - size_t overflows", orig_key, size));
		}
		return { v3, v4 };
	}

    bool Executor::run_impl_opcodes_execute_compare(ExecuteBufferReader &reader, CompareKind kind) {
        auto jump_dest = reader.decode<std::uint32_t>();
        auto left = peek_value(2);
        auto right = peek_value(1);
        auto res = execute_compare(vm, kind, left, right);
        switch(res) {
        case CompareResult::True:
            left = right;
            pop_values(1);
            return false;
        case CompareResult::False:
            left = false;
            pop_values(1);
            reader.jump(jump_dest);
            return false;
        case CompareResult::NotExecuted:
            vm->throw_cant_compare(kind, left.type(), right.type());
            return true;
        }
        assert(false);
        return true;
    }
    void Executor::run_impl_opcodes(ExecutionFrame &frame, RuntimeFunction::ScriptFunction &sf)
    {
        static thread_local OwcaValue ignore_value;
        auto reader = ExecuteBufferReader{ frame.runtime_function->code, frame.code_position };
        bool exit = false;
        do {
            auto opcode = reader.decode<ExecuteBufferReader::Op>();
            switch(opcode) {
            case ExecuteBufferReader::Op::ClassInit: {
                auto line = reader.line();
                auto name = reader.decode<std::string_view>();
                auto full_name = reader.decode<std::string_view>();
                auto cls = vm->allocate<Class>(0, line, name, full_name, frame.runtime_function->code);
                frame.states.push_back(ExecutionFrame::ClassState{ name, full_name, cls });
                break; }
            case ExecuteBufferReader::Op::ClassCreate: {
                assert(!frame.states.empty());
                assert(std::holds_alternative<ExecutionFrame::ClassState>(frame.states.back()));
                auto &state = std::get<ExecutionFrame::ClassState>(frame.states.back());

                auto cls = state.cls;
                frame.states.pop_back();

                auto native = reader.decode<bool>();
                auto base_class_count = reader.decode<std::uint32_t>();
                auto member_count = reader.decode<std::uint32_t>();
                auto all_variable_names = reader.decode<bool>();
                auto variable_name_count = reader.decode<std::uint32_t>();

                auto base_classes = peek_values(base_class_count, base_class_count);
                auto members = peek_values(member_count + base_class_count, member_count);
                for(auto b : base_classes) {
                    cls->initialize_add_base_class(vm, b.as_class(vm));
                }
                for(auto f : members) {
                    cls->initialize_add_function(vm, f.as_functions(vm));
                }
                if (all_variable_names) {
                    cls->initialize_set_all_variables();
                }
                else {
                    for(auto i = 0u; i < variable_name_count; ++i) {
                        auto var_name = reader.decode<std::string_view>();
                        cls->initialize_add_variable(var_name);
                    }
                }

                cls->finalize_initializing(vm);

                if (native) {
                    auto &native_provider = cls->code.native_code_provider();
                    if (native_provider) {
                        if (auto impl = native_provider->native_class(cls->full_name, ClassToken{ cls })) {
                            auto size = impl->native_storage_size();
                            cls->native_storage_pointers[cls] = { cls->native_storage_total, size };
                            cls->native_storage_total = (cls->native_storage_total + size + 15) & ~15;
                            cls->native = std::move(impl);
                        }
                    }
                    if (!cls->native) {
                        vm->throw_missing_native(std::format("missing native class {}", cls->full_name));
                    }
                }
                pop_values(base_class_count + member_count);
                push_value(OwcaClass{ cls });
                break; }
            case ExecuteBufferReader::Op::ExprPopAndIgnore: {
                pop_values(1);
                break; }
            case ExecuteBufferReader::Op::ExprCompareEq: {
                exit = run_impl_opcodes_execute_compare(reader, CompareKind::Eq);
                break; }
            case ExecuteBufferReader::Op::ExprCompareNotEq: {
                exit = run_impl_opcodes_execute_compare(reader, CompareKind::NotEq);
                break; }
            case ExecuteBufferReader::Op::ExprCompareLessEq: {
                exit = run_impl_opcodes_execute_compare(reader, CompareKind::LessEq);
                break; }
            case ExecuteBufferReader::Op::ExprCompareMoreEq: {
                exit = run_impl_opcodes_execute_compare(reader, CompareKind::MoreEq);
                break; }
            case ExecuteBufferReader::Op::ExprCompareLess: {
                exit = run_impl_opcodes_execute_compare(reader, CompareKind::Less);
                break; }
            case ExecuteBufferReader::Op::ExprCompareMore: {
                exit = run_impl_opcodes_execute_compare(reader, CompareKind::More);
                break; }
            case ExecuteBufferReader::Op::ExprCompareIs: {
                exit = run_impl_opcodes_execute_compare(reader, CompareKind::Is);
                break; }
            case ExecuteBufferReader::Op::ExprCompareCompleted: {
                peek_value(1) = true;
                break; }
            case ExecuteBufferReader::Op::ExprConstantEmpty: {
                push_value(OwcaEmpty{});
                break; }
            case ExecuteBufferReader::Op::ExprConstantBool: {
                auto value = reader.decode<bool>();
                push_value(value);
                break; }
            case ExecuteBufferReader::Op::ExprConstantFloat: {
                auto value = reader.decode<Number>();
                push_value(value);
                break; }
            case ExecuteBufferReader::Op::ExprConstantString: {
                auto value = reader.decode<std::string_view>();
                push_value(vm->create_string_from_view(value));
                break; }
            case ExecuteBufferReader::Op::ExprConstantStringInterpolated: {
                auto strings = reader.decode<std::string_view>();
                auto expr_count = reader.decode<std::uint32_t>();
                size_t size = strings.size();
                auto values = peek_values(expr_count, expr_count);
                for(auto i = 0u; i < expr_count; ++i) {
                    size += values[i].as_string(vm).size();
                }
                auto new_str = vm->precreate_string(size);
                auto new_str_pt = new_str->pointer();
                const char *strings_ptr = strings.data();
                for(auto i = 0u; i <= expr_count; ++i) {
                    if (i > 0) {
                        auto str = values[expr_count - i].as_string(vm);
                        std::memcpy(new_str_pt, str.text().data(), str.size());
                        new_str_pt += str.size();
                    }
                    auto sz = reader.decode<std::uint32_t>();
                    std::memcpy(new_str_pt, strings_ptr, sz);
                    new_str_pt += sz;
                    strings_ptr += sz;
                }
                assert(new_str_pt == new_str->pointer() + new_str->size());
                pop_values(expr_count);
                push_value(OwcaString{ new_str });
                break; }
            case ExecuteBufferReader::Op::ExprIdentifierRead: {
                auto index = reader.decode<std::uint32_t>();
                push_value(frame.values[index]);
                break; }
            case ExecuteBufferReader::Op::ExprIdentifierWrite: {
                auto index = reader.decode<std::uint32_t>();
                auto &val = peek_value(1);
                frame.values[index] = val;
                break; }
            case ExecuteBufferReader::Op::ExprMemberRead: {
                auto self = peek_value(1);
                auto member = reader.decode<std::string_view>();
                peek_value(1) = vm->member(self, member);
                break; }
            case ExecuteBufferReader::Op::ExprMemberWrite: {
                auto val_to_write = peek_value(1);
                auto self = peek_value(2);
                auto member = reader.decode<std::string_view>();
                vm->member(self, member, val_to_write);
                peek_value_and_make_top(2) = val_to_write;
                break; }
            case ExecuteBufferReader::Op::ExprOper1BinNeg: {
                auto &left = peek_value(1);
                left = -(std::int64_t)left.as_float(vm);
                break; }
            case ExecuteBufferReader::Op::ExprOper1LogNot: {
                auto &left = peek_value(1);
                left = !left.is_true();
                break; }
            case ExecuteBufferReader::Op::ExprOper1Negate: {
                auto &left = peek_value(1);
                left = -left.as_float(vm);
                break; }
            case ExecuteBufferReader::Op::ExprRetTrueAndJumpIfTrue: {
                auto jump_dest = reader.decode<std::uint32_t>();
                if (peek_value(1).is_true()) {
                    reader.jump(jump_dest);
                }
                else {
                    pop_values(1);
                }
                break; }
            case ExecuteBufferReader::Op::ExprRetFalseAndJumpIfFalse: {
                auto jump_dest = reader.decode<std::uint32_t>();
                if (!peek_value(1).is_true()) {
                    reader.jump(jump_dest);
                }
                else {
                    pop_values(1);
                }
                break; }
            case ExecuteBufferReader::Op::ExprToString: {
                auto v = peek_value(1);
                if (v.kind() == OwcaValueKind::String) {
                    break;
                }
                peek_value(1) = vm->create_string_from_view(v.to_string());
                break; }
            case ExecuteBufferReader::Op::ExprOper2BinOr: {
                run_impl_opcodes_execute_expr_oper2<TagBinOr>(reader);
                break; }
            case ExecuteBufferReader::Op::ExprOper2BinAnd: {
                run_impl_opcodes_execute_expr_oper2<TagBinAnd>(reader);
                break; }
            case ExecuteBufferReader::Op::ExprOper2BinXor: {
                run_impl_opcodes_execute_expr_oper2<TagBinXor>(reader);
                break; }
            case ExecuteBufferReader::Op::ExprOper2BinLShift: {
                run_impl_opcodes_execute_expr_oper2<TagBinLShift>(reader);
                break; }
            case ExecuteBufferReader::Op::ExprOper2BinRShift: {
                run_impl_opcodes_execute_expr_oper2<TagBinRShift>(reader);
                break; }
            case ExecuteBufferReader::Op::ExprOper2Add: {
                run_impl_opcodes_execute_expr_oper2<TagAdd>(reader);
                break; }
            case ExecuteBufferReader::Op::ExprOper2Sub: {
                run_impl_opcodes_execute_expr_oper2<TagSub>(reader);
                break; }
            case ExecuteBufferReader::Op::ExprOper2Mul: {
                run_impl_opcodes_execute_expr_oper2<TagMul>(reader);
                break; }
            case ExecuteBufferReader::Op::ExprOper2Div: {
                run_impl_opcodes_execute_expr_oper2<TagDiv>(reader);
                break; }
            case ExecuteBufferReader::Op::ExprOper2Mod: {
                run_impl_opcodes_execute_expr_oper2<TagMod>(reader);
                break; }
            case ExecuteBufferReader::Op::ExprOper2MakeRange: {
                auto third = peek_value(1);
                auto second = peek_value(2);
                auto first = peek_value(3);
                if (first.kind() == OwcaValueKind::Empty) {
                    first = 0;
                }
                if (second.kind() == OwcaValueKind::Empty) {
                    second = std::numeric_limits<Number>::max();
                }
                if (third.kind() == OwcaValueKind::Empty) {
                    third = 1;
                }
                if (third.as_float(vm) == 0) {
                    vm->throw_range_step_is_zero();
                }
                auto ret = vm->allocate<Range>(0);
                ret->from = first.as_float(vm);
                ret->to = second.as_float(vm);
                ret->step = third.as_float(vm);
                push_value(OwcaRange{ ret });
                break; }
            case ExecuteBufferReader::Op::ExprOper2IndexRead: {
                auto key = peek_value(1);
                auto v = peek_value(2);

                auto orig_key = key;

                peek_value_and_make_top(2) = v.visit(
                    [&](const OwcaString& o) -> OwcaValue {
                        const auto size = (Number)o.internal_value()->size();
                        if (size != o.internal_value()->size()) {
                            vm->throw_index_out_of_range(std::format("string size {} is too large for Number size to properly handle indexing", o.internal_value()->size()));
                        }
                        auto [ lower, upper, step ] = parse_key(vm, key, key, size);
                        if (step == 0) return o.substr(lower, lower + 1);
                        if (step == 1) return o.substr(lower, upper);
                        std::string result;
                        result.reserve((size_t)std::abs(std::ceil((upper - lower + step) / step)));
                        RangeIterator iter(lower, upper, step);
                        for(RangeIterator iter(lower, upper, step); !iter.done(); iter.next()) {
                            result += o.text()[iter.get()];
                        }
                        return vm->create_string_from_view(result);
                    },
                    [&](const OwcaArray& o) -> OwcaValue {
                        const auto size = (Number)o.internal_value()->values.size();
                        if (size != o.internal_value()->values.size()) {
                            vm->throw_index_out_of_range(std::format("array size {} is too large for Number size to properly handle indexing", o.internal_value()->values.size()));
                        }

                        auto [ lower, upper, step ] = parse_key(vm, key, key, size);
                        if (step == 0) return o[lower];
                        if (step == 1) return vm->create_array(o.internal_value()->sub_deque(lower, upper));
                        std::deque<OwcaValue> result;
                        RangeIterator iter(lower, upper, step);
                        for(RangeIterator iter(lower, upper, step); !iter.done(); iter.next()) {
                            result.push_back(o[iter.get()]);
                        }
                        return vm->create_array(std::move(result));
                    },
                    [&](const OwcaTuple& o) -> OwcaValue {
                        const auto size = (Number)o.internal_value()->values.size();
                        if (size != o.internal_value()->values.size()) {
                            vm->throw_index_out_of_range(std::format("tuple size {} is too large for Number size to properly handle indexing", o.internal_value()->values.size()));
                        }

                        auto [ lower, upper, step ] = parse_key(vm, key, key, size);
                        if (step == 0) return o[lower];
                        if (step == 1) return vm->create_tuple(o.internal_value()->sub_array(lower, upper));
                        std::vector<OwcaValue> result;
                        result.reserve((size_t)std::abs(std::ceil((upper - lower + step) / step)));
                        RangeIterator iter(lower, upper, step);
                        for(RangeIterator iter(lower, upper, step); !iter.done(); iter.next()) {
                            result.push_back(o[iter.get()]);
                        }
                        return vm->create_tuple(std::move(result));
                    },
                    [&](const OwcaMap& o) -> OwcaValue {
                        return o.internal_value()->dict.read(key);
                    },
                    [&](const auto&) -> OwcaValue {
                        vm->throw_value_not_indexable(v.type());
                    }
                );
                break; }
            case ExecuteBufferReader::Op::ExprOper2IndexWrite: {
                auto value = peek_value(1);
                auto key = peek_value(2);
                auto v = peek_value(3);

                auto orig_key = key;

                peek_value_and_make_top(3) = v.visit(
                    [&](const OwcaMap& o) -> OwcaValue {
                        o.internal_value()->dict.write(key, value);
                        return value;
                    },
                    [&](OwcaArray o) -> OwcaValue {
                        const auto size = (Number)o.internal_value()->values.size();
                        if (size != o.internal_value()->values.size()) {
                            vm->throw_index_out_of_range(std::format("array size {} is too large for Number size to properly handle indexing", o.internal_value()->values.size()));
                        }
                        auto [ lower, upper, step ] = parse_key(vm, key, key, size);
                        if (step == 0) {
                            o[lower] = value;
                            return value;
                        }
                        if (step != 1) {
                            vm->throw_range_step_must_be_one_in_left_side_of_write_assign();
                        }

                        auto iter = vm->create_iterator(value);
                        auto write = lower;
                        auto &values = o.internal_value()->values;
                        std::vector<OwcaValue> temp;

                        for(auto val = iter.next(); val.kind() != OwcaValueKind::Completed; val = iter.next()) {
                            assert(write <= upper);
                            if (write < upper) {
                                values[write++] = val;
                            }
                            else {
                                assert(write == upper);
                                temp.push_back(val);
                            }
                        }
                        if (write < upper) {
                            assert(temp.empty());

                            for(size_t i = upper; i < values.size(); ++i) {
                                values[write++] = values[i];
                            }
                            assert(write < values.size());
                            values.resize(write);
                        }
                        else if (!temp.empty()) {
                            auto old_size = values.size();
                            values.resize(values.size() + temp.size());
                            auto new_size = values.size();
                            for(size_t i = old_size; i > upper; --i, --new_size) {
                                values[new_size] = values[i];
                            }
                            for(auto q : temp) {
                                values[write++] = q;
                            }
                            assert(write == new_size);
                        }
                        return value;
                    },
                    [&](OwcaTuple o) -> OwcaValue {
                        vm->throw_readonly("tuple is readonly");
                    },
                    [&](const auto&) -> OwcaValue {
                        vm->throw_value_not_indexable(v.type());
                    }
                );
                break; }
            case ExecuteBufferReader::Op::ExprOperXCall: {
                auto size = reader.decode<std::uint32_t>();
                auto &fnc = peek_value(size);
                auto args = peek_values(size - 1, size - 1);
                prepare_execute_call(fnc, fnc, args);
                exit = true;
                break; }
            case ExecuteBufferReader::Op::ExprOperXCreateArray: {\
                auto size = reader.decode<std::uint32_t>();
                auto args = peek_values(size, size);
                auto arguments = std::deque<OwcaValue>{ args.begin(), args.end() };
                peek_value_and_make_top(size) = vm->create_array(std::move(arguments));
                break; }
            case ExecuteBufferReader::Op::ExprOperXCreateTuple: {
                auto size = reader.decode<std::uint32_t>();
                auto args = peek_values(size, size);
                auto arguments = std::vector<OwcaValue>{ args.begin(), args.end() };
                peek_value_and_make_top(size) = vm->create_tuple(std::move(arguments));
                break; }
            case ExecuteBufferReader::Op::ExprOperXCreateSet: {
                auto size = reader.decode<std::uint32_t>();
                auto args = peek_values(size, size);
                peek_value_and_make_top(size) = vm->create_set(args);
                break; }
            case ExecuteBufferReader::Op::ExprOperXCreateMap: {
                auto size = reader.decode<std::uint32_t>();
                auto args = peek_values(size, size);
                peek_value_and_make_top(size) = vm->create_map(args);
                break; }
            case ExecuteBufferReader::Op::ForInit: {
                auto iterator = peek_value(1).as_iterator(vm);
                pop_values(1);
                frame.states.push_back(ExecutionFrame::ForState{ iterator });
                auto &state = std::get<ExecutionFrame::ForState>(frame.states.back());
                state.end_position = reader.decode<std::uint32_t>();
                state.loop_index = reader.decode<std::uint32_t>();
                reader.decode_span_helper<std::uint32_t>(
                    [&](size_t index, size_t size, std::uint32_t value) {
                        if (index == 0) state.value_indexes.resize(size);
                        state.value_indexes[index] = value;
                    }
                );
                state.loop_control_depth = reader.decode<std::uint8_t>();
                state.continue_position = reader.position();
                break; }
            case ExecuteBufferReader::Op::ForCondition: {
                assert(!frame.states.empty());
                assert(std::holds_alternative<ExecutionFrame::ForState>(frame.states.back()));
                auto &state = std::get<ExecutionFrame::ForState>(frame.states.back());
                state.index++;
                if (state.loop_index != std::numeric_limits<std::uint32_t>::max()) {
                    vm->set_identifier(state.loop_index, state.index);
                }
                break; }
            case ExecuteBufferReader::Op::ForNext: {
                assert(!frame.states.empty());
                assert(std::holds_alternative<ExecutionFrame::ForState>(frame.states.back()));
                auto &state = std::get<ExecutionFrame::ForState>(frame.states.back());

                auto val = state.iterator.next();
                if (val.kind() == OwcaValueKind::Completed) {
                    reader.jump(state.end_position);
                }
                else {
                    if (state.value_indexes.size() == 1) {
                        vm->set_identifier(state.value_indexes[0], val);
                    }
                    else {
                        auto iter = vm->iterate_value(val);
                        size_t index = 0;

                        while(auto vv = iter.next()) {
                            if (index >= state.value_indexes.size()) {
                                prepare_throw_too_many_elements(state.value_indexes.size());
                                exit = true;
                            }
                            vm->set_identifier(state.value_indexes[index], *vv);
                            ++index;
                        }
                        if (index < state.value_indexes.size()) {
                            prepare_throw_not_enough_elements(state.value_indexes.size(), index);
                            exit = true;
                        }
                        if (exit) break;
                    }
                }
                break; }
            case ExecuteBufferReader::Op::ForCompleted: {
                assert(!frame.states.empty());
                assert(std::holds_alternative<ExecutionFrame::ForState>(frame.states.back()));
                auto &state = std::get<ExecutionFrame::ForState>(frame.states.back());
                reader.jump(state.continue_position);
                frame.states.pop_back();
                break; }
            case ExecuteBufferReader::Op::Function: {
                auto name = reader.decode<std::string_view>();
                auto full_name = reader.decode<std::string_view>();
                auto is_native = reader.decode<bool>();
                auto is_generator = reader.decode<bool>();
                auto is_method = reader.decode<bool>();
                auto param_count = reader.decode<std::uint32_t>();

                auto code = reader.code();
                RuntimeFunction *fnc = nullptr;
                if (is_native) {
                    auto &native_provider = code.native_code_provider();
                    std::optional<ClassToken> class_;
                    if (!frame.states.empty()) {
                        if (std::holds_alternative<ExecutionFrame::ClassState>(frame.states.back())) {
                            auto &state = std::get<ExecutionFrame::ClassState>(frame.states.back());
                            class_ = ClassToken{ state.cls };
                        }
                    }
                    if (is_generator) {
                        fnc = vm->allocate<RuntimeFunction>(0, std::move(code), name, full_name, RuntimeFunction::NativeGenerator{});
                        auto &ng = std::get<RuntimeFunction::NativeGenerator>(fnc->data);
                        ng.parameter_names.reserve(param_count);
                        for(auto i = 0u; i < param_count; ++i) {
                            auto var_name = reader.decode<std::string_view>();
                            ng.parameter_names.push_back(var_name);
                        }
                        if (native_provider) {
                            if (auto impl = native_provider->native_generator(full_name, class_, FunctionToken{ fnc }, ng.parameter_names)) {
                                ng.generator = std::move(*impl);
                            }
                        }
                        if (!ng.generator) {
                            vm->throw_missing_native(std::format("missing native generator {}", full_name));
                        }
                    }
                    else {
                        fnc = vm->allocate<RuntimeFunction>(0, std::move(code), name, full_name, RuntimeFunction::NativeFunction{});
                        auto &nf = std::get<RuntimeFunction::NativeFunction>(fnc->data);
                        nf.parameter_names.reserve(param_count);
                        for(auto i = 0u; i < param_count; ++i) {
                            auto var_name = reader.decode<std::string_view>();
                            nf.parameter_names.push_back(var_name);
                        }
                        if (native_provider) {
                            if (auto impl = native_provider->native_function(full_name, class_, FunctionToken{ fnc }, nf.parameter_names)) {
                                nf.function = std::move(*impl);
                            }
                        }
                        if (!nf.function) {
                            vm->throw_missing_native(std::format("missing native function {}", full_name));
                        }
                    }
                }
                else {
                    fnc = vm->allocate<RuntimeFunction>(0, std::move(code), name, full_name, RuntimeFunction::ScriptFunction{});
                    auto &sf = std::get<RuntimeFunction::ScriptFunction>(fnc->data);
                    fnc->param_count = param_count;
                    fnc->is_method = is_method;
                    reader.decode_span_helper<AstFunction::CopyFromParent>(
                        [&](size_t index, size_t size, AstFunction::CopyFromParent value) {
                            if (index == 0) sf.copy_from_parents.reserve(size);
                            sf.copy_from_parents.push_back(value);
                        }
                    );
                    reader.decode_span_helper<std::string_view>(
                        [&](size_t index, size_t size, std::string_view value) {
                            if (index == 0) sf.identifier_names.resize(size);
                            sf.identifier_names[index] = value;
                        }
                    );
                    auto next = reader.decode<std::uint32_t>();
                    sf.max_stack_size = reader.decode<std::uint32_t>();
                    sf.max_storage_size = reader.decode<std::uint32_t>();
                    sf.entry_point = reader.position();
                    reader.jump(next);

                    sf.values_from_parents.reserve(sf.copy_from_parents.size());
                    for(auto c : sf.copy_from_parents) {
             			sf.values_from_parents.push_back(vm->get_identifier(c.index_in_parent));                        
                    }
                }
                auto rfs = VM::get(vm).allocate<RuntimeFunctions>(0, name, full_name);
                rfs->functions[fnc->param_count] = fnc;
                push_value(OwcaFunctions{ rfs });
                break; }
            case ExecuteBufferReader::Op::If: {
                auto val = peek_value(1).as_bool(vm);
                auto else_position = reader.decode<std::uint32_t>();
                if (!val) {
                    reader.jump(else_position);
                }
                pop_values(1);
                break; }
            case ExecuteBufferReader::Op::LoopControlBreak: {
                auto depth = reader.decode<std::uint8_t>();
                while(true) {
                    assert(!frame.states.empty());
                    auto found = visit_variant(frame.states.back(),
                        [&](const ExecutionFrame::ForState& s) {
                            if (s.loop_control_depth == depth) {
                                reader.jump(s.end_position);
                                return true;
                            }
                            return false;
                        },
                        [&](const auto& s) { return false; }
                    );
                    if (found) break;
                    frame.states.pop_back();
                }
                break; }
            case ExecuteBufferReader::Op::LoopControlContinue: {
                auto depth = reader.decode<std::uint8_t>();
                while(true) {
                    assert(!frame.states.empty());
                    auto found = visit_variant(frame.states.back(),
                        [&](const ExecutionFrame::ForState& s) {
                            if (s.loop_control_depth == depth) {
                                reader.jump(s.continue_position);
                                return true;
                            }
                            return false;
                        },
                        [&](const auto& s) { return false; }
                    );
                    if (found) break;
                    frame.states.pop_back();
                }
                break; }
            case ExecuteBufferReader::Op::Return: {
                *frame.return_value = OwcaEmpty{};
                pop_frame();
                exit = true;
                break; }
            case ExecuteBufferReader::Op::ReturnValue: {
                *frame.return_value = peek_value(1);
                pop_values(1);
                pop_frame();
                exit = true;
                break; }
            case ExecuteBufferReader::Op::Throw: {
                auto exception = peek_value(1);
                pop_values(1);
                exception_in_progress = exception.as_exception(vm);
                process_thrown_exception();
                exit = true;
                break; }
            case ExecuteBufferReader::Op::TryInit: {
                assert(!exception_in_progress);
                frame.states.push_back(ExecutionFrame::TryCatchState{});
                auto &state = std::get<ExecutionFrame::TryCatchState>(frame.states.back());
                state.begin_pos = reader.decode<std::uint32_t>();
                state.end_pos = reader.decode<std::uint32_t>();
                state.catches_pos = reader.position();
                reader.jump(state.begin_pos);
                state.temporaries_size = frame.temporaries.size();
                break; }
            case ExecuteBufferReader::Op::TryCompleted: {
                assert(!frame.states.empty());
                assert(!exception_in_progress);
                assert(std::holds_alternative<ExecutionFrame::TryCatchState>(frame.states.back()));
                frame.states.pop_back();
                break; }
            case ExecuteBufferReader::Op::TryCatchType: {
                auto values = reader.decode<std::uint32_t>();
                auto ident = reader.decode<std::uint32_t>();
                auto skip_jump = reader.decode<std::uint32_t>();

                auto exc_types = peek_values(values, values);
                assert(exception_in_progress);

                bool found = false;
                for(auto e : exc_types) {
                    auto exc_type = e.as_class(vm);
                    if (exception_in_progress->type().has_base_class(exc_type)) {
                        found = true;
                        break;
                    }
                }
                if (found) {
                    if (ident != std::numeric_limits<std::uint32_t>::max()) {
                        vm->set_identifier(ident, *exception_in_progress);
                    }
                }
                else {
                    reader.jump(skip_jump);
                }
                break; }
            case ExecuteBufferReader::Op::TryCatchTypeCompleted: {
                process_thrown_exception();
                exit = true;
                break; }
            case ExecuteBufferReader::Op::TryBlockCompleted: {
                assert(exception_in_progress);
                exception_in_progress = std::nullopt;
                break; }
            case ExecuteBufferReader::Op::WhileInit: {
                frame.states.push_back(ExecutionFrame::WhileState{});
                auto &state = std::get<ExecutionFrame::WhileState>(frame.states.back());
                state.end_position = reader.decode<std::uint32_t>();
                state.loop_index = reader.decode<std::uint32_t>();
                state.loop_control_depth = reader.decode<std::uint8_t>();
                state.continue_position = reader.position();
                break; }
            case ExecuteBufferReader::Op::WhileCondition: {
                assert(!frame.states.empty());
                assert(std::holds_alternative<ExecutionFrame::WhileState>(frame.states.back()));
                auto &state = std::get<ExecutionFrame::WhileState>(frame.states.back());
                state.index++;
                if (state.loop_index != std::numeric_limits<std::uint32_t>::max()) {
                    vm->set_identifier(state.loop_index, state.index);
                }
                break; }
            case ExecuteBufferReader::Op::WhileNext: {
                assert(!frame.states.empty());
                assert(std::holds_alternative<ExecutionFrame::WhileState>(frame.states.back()));
                auto &state = std::get<ExecutionFrame::WhileState>(frame.states.back());

                auto value = peek_value(1).as_bool(vm);
                pop_values(1);
                if (!value) {
                    reader.jump(state.end_position);
                }
                break; }
            case ExecuteBufferReader::Op::WhileCompleted: {
                assert(!frame.states.empty());
                assert(std::holds_alternative<ExecutionFrame::WhileState>(frame.states.back()));
                frame.states.pop_back();
                break; }
            case ExecuteBufferReader::Op::WithInitPrepare: {
                auto &obj = peek_value(1);
                auto mbm = vm->member(obj, "__enter__");
                prepare_execute_call(obj, mbm, {});
                exit = true;
                break; }
            case ExecuteBufferReader::Op::WithInit: {
                frame.states.push_back(ExecutionFrame::WithState{});
                auto &state = std::get<ExecutionFrame::WithState>(frame.states.back());
                auto index = reader.decode<std::uint32_t>();
                state.context = peek_value(1);
                if (index != std::numeric_limits<std::uint32_t>::max()) {
                    vm->set_identifier(index, state.context);
                }
                pop_values(1);
                break; }
            case ExecuteBufferReader::Op::WithCompleted: {
                assert(!frame.states.empty());
                assert(std::holds_alternative<ExecutionFrame::WithState>(frame.states.back()));
                auto &state = std::get<ExecutionFrame::WithState>(frame.states.back());
                auto mbm = vm->member(state.context, "__exit__");
                prepare_execute_call(ignore_value, mbm, {});
                exit = true;
                break; }
            case ExecuteBufferReader::Op::Yield: {
                assert(frame.runtime_function->is_generator());
                *frame.return_value = peek_value(1);
                pop_values(1);
                pop_frame();
                exit = true;
                break; }
            case ExecuteBufferReader::Op::Jump: {
                auto dest = reader.decode<std::uint32_t>();
                reader.jump(dest);
                break; }
            }
            frame.code_position = reader.position();
        } while(!exit);
    }

    template <typename Tag> std::string_view tag_name = "unknown";
    template <> std::string_view tag_name<Executor::TagAdd> = "addition";
    template <> std::string_view tag_name<Executor::TagSub> = "subtraction";
    template <> std::string_view tag_name<Executor::TagMul> = "multiplication";
    template <> std::string_view tag_name<Executor::TagDiv> = "division";
    template <> std::string_view tag_name<Executor::TagMod> = "modulus";

    Number Executor::expr_oper_2(Executor::TagAdd, Number left, Number right) {
        return left + right;
    }
    OwcaArray Executor::expr_oper_2(TagAdd, OwcaArray left, OwcaArray right) {
        auto ret = vm->allocate<Array>(0);
        ret->values = left.internal_value()->values;
        for(auto &q : right.internal_value()->values) {
            ret->values.push_back(q);
        }
        return OwcaArray{ ret };
    }
    OwcaTuple Executor::expr_oper_2(TagAdd, OwcaTuple left, OwcaTuple right) {
        auto ret = vm->allocate<Tuple>(0);
        ret->values.reserve(left.internal_value()->values.size() + right.internal_value()->values.size());
        for(auto &q : left.internal_value()->values) {
            ret->values.push_back(q);
        }
        for(auto &q : right.internal_value()->values) {
            ret->values.push_back(q);
        }
        return OwcaTuple{ ret };
    }

    OwcaString Executor::expr_oper_2(Executor::TagAdd, OwcaString left, OwcaString right) {
        return vm->create_string(left, right);
    }
    Number Executor::expr_oper_2(Executor::TagSub, Number left, Number right) {
        return left - right;
    }
    Number Executor::expr_oper_2(Executor::TagMul, Number left, Number right) {
        return left * right;
    }
    Number Executor::expr_oper_2(Executor::TagDiv, Number left, Number right) {
        if (right == 0) {
            vm->throw_division_by_zero();
        }
        return left / right;
    }
    Number Executor::expr_oper_2(Executor::TagMod, Number left, Number right) {
        if (right == 0) {
            vm->throw_division_by_zero();
        }
        return (std::int64_t)left % (std::int64_t)right;
    }
    OwcaString Executor::expr_oper_2(Executor::TagMul, OwcaString left, Number right) {
        return vm->create_string(left, right);
    }
    OwcaString Executor::expr_oper_2(Executor::TagMul, Number left, OwcaString right) {
        return vm->create_string(right, left);
    }
    OwcaArray Executor::expr_oper_2(TagMul, OwcaArray left, Number right) {
        auto ret = vm->allocate<Array>(0);
        for(auto i = 0u; i < right; ++i) {
            ret->values.insert(ret->values.end(), left.internal_value()->values.begin(), left.internal_value()->values.end());
        }
        return OwcaArray{ ret };
    }
    OwcaArray Executor::expr_oper_2(TagMul, Number left, OwcaArray right) {
            auto ret = vm->allocate<Array>(0);
            for(auto i = 0u; i < left; ++i) {
                ret->values.insert(ret->values.end(), right.internal_value()->values.begin(), right.internal_value()->values.end());
            }
            return OwcaArray{ ret };
    }
    OwcaTuple Executor::expr_oper_2(TagMul, OwcaTuple left, Number right) {
            auto ret = vm->allocate<Tuple>(0);
            ret->values.reserve((size_t)(right * left.internal_value()->values.size()));
            for(auto i = 0u; i < right; ++i) {
                ret->values.insert(ret->values.end(), left.internal_value()->values.begin(), left.internal_value()->values.end());
            }
            return OwcaTuple{ ret };
    }
    OwcaTuple Executor::expr_oper_2(TagMul, Number left, OwcaTuple right) {
            auto ret = vm->allocate<Tuple>(0);
            ret->values.reserve((size_t)(left * right.internal_value()->values.size()));
            for(auto i = 0u; i < left; ++i) {
                ret->values.insert(ret->values.end(), right.internal_value()->values.begin(), right.internal_value()->values.end());
            }
            return OwcaTuple{ ret };
    }

    template <typename A, typename B, typename C> OwcaEmpty Executor::expr_oper_2(A, B b, C c) {
        vm->throw_unsupported_operation_2(tag_name<A>, OwcaValue{ b }.type(), OwcaValue{ c }.type());
        return {};
    }
    template <typename Tag> void Executor::run_impl_opcodes_execute_expr_oper2(ExecuteBufferReader &reader) {
        auto right = peek_value(1);
        auto left = peek_value(2);
        auto val = left.visit([&](auto left_val) -> OwcaValue {
            return right.visit([&](auto right_val) -> OwcaValue {
                return expr_oper_2(Tag{}, left_val, right_val);
            });
        });
        peek_value_and_make_top(2) = val;
    }
    void Executor::run_impl() {
        while(!completed()) {
            auto &frame = currently_executing_frame();
            visit_variant(
                frame.runtime_function->data,
                [&](RuntimeFunction::ScriptFunction& sf) -> void {
                    run_impl_opcodes(frame, sf);
                },
                [&](RuntimeFunction::NativeFunction& nf) -> void {
                    *frame.return_value = nf.function(vm, frame.values);
                    pop_frame();
                },
                [&](RuntimeFunction::NativeGenerator& ngf) -> void {
                    auto generator = ngf.generator(vm, frame.values);
                    auto obj = vm->allocate<Iterator>(0, false);
                    obj->generator = std::move(generator);
                    *frame.return_value = OwcaIterator{ obj };
                    pop_frame();
                }
            );
        }
    }
    void Executor::run_and_throw() {
        run();
        throw temporary_exception_return_value.as_exception(vm);
    }
    void Executor::run(OwcaMap *dict_output) {
        run_impl();
        if (dict_output) {
            auto &old_frame = just_executed_executing_frame();
            old_frame.runtime_function->visit(
                [&](RuntimeFunction::ScriptFunction& sf) -> void {
                    for (auto i = 0u; i < sf.identifier_names.size(); ++i) {
                        auto key = vm->create_string_from_view(sf.identifier_names[i]);
                        (*dict_output)[key] = old_frame.values[i];
                    }
                },
                [&](RuntimeFunction::NativeFunction&) -> void {
                    assert(false);
                },
                [&](RuntimeFunction::NativeGenerator&) -> void {
                    assert(false);
                }
            );
        }
    }
    void Executor::prepare_execute_code_block(OwcaValue &return_value, const OwcaCode &oc) {
        auto &frame = push_new_frame();
        frame.initialize_code_block(return_value, vm, oc);
    }
    void Executor::prepare_execute_main_function(OwcaValue &return_value, RuntimeFunctions* runtime_functions, std::optional<OwcaMap> arguments) {
        auto &frame = push_new_frame();
        frame.initialize_main_block_function(return_value, vm, runtime_functions, arguments);
    }

	OwcaValue Executor::execute_code_block(const OwcaCode &oc, std::optional<OwcaMap> values, OwcaMap *dict_output)
	{
        OwcaValue temp;
		
        prepare_execute_code_block(temp, oc);
        run();
		auto fnc = temp.as_functions(vm);
        assert(fnc.internal_self_object() == nullptr);
		prepare_execute_main_function(temp, fnc.internal_value(), values);
		run(dict_output);
        return temp;
    }

    void Executor::prepare_resume_generator(OwcaValue &return_value, OwcaIterator oi)
    {
		if (oi.internal_value()->completed) {
            return_value = OwcaCompleted{};
			return;
		}
        auto &frame = push_new_frame();
        frame = std::move(oi.internal_value()->frame);
        frame.return_value = &return_value;
    }

    OwcaValue Executor::resume_generator(OwcaIterator oi)
	{
		if (oi.internal_value()->completed) {
			return OwcaCompleted{};
		}
		OwcaValue value;
		prepare_resume_generator(value, oi);
		run();
		return value;
    }

	void Executor::prepare_allocate_user_class(OwcaValue &return_value, Class *cls, std::span<OwcaValue> arguments, bool exception_for_throwing_construction) {
		OwcaValue obj;
		
		if (cls->allocator_override) {
			obj = cls->allocator_override();
		}
		else if (cls->reload_self) {
			obj = {};
		}
		else {
			obj = OwcaObject{ vm->allocate<Object>(cls->native_storage_total, cls) };
		}
        if (exception_for_throwing_construction) {
            auto oe = OwcaValue{ obj }.as_exception(vm);
            oe.internal_value()->parent_exception = exception_in_progress;
            exception_in_progress.reset();
        }
		auto it = cls->values.find(std::string_view{ "__init__" });
		if (it == cls->values.end()) {
			if (!arguments.empty()) {
				throw_cant_call(std::format("type {} has no __init__ function defined - expected constructor's call with no parameters, instead got {} values", cls->full_name, arguments.size()));
			}
            return_value = obj;
		}
		else {
			visit_variant(it->second,
				[&](RuntimeFunctions *rf) -> void {
                    auto &frame = prepare_execute_function(return_value, rf, obj, arguments);
					if (!cls->reload_self)
                        frame.constructor_move_self_to_return_value = true;
				},
				[&](Class* var) -> void {
					throw_cant_call(std::format("type {} has __init__ variable, not a function", cls->full_name));
				}
			);
		}
	}
    OwcaValue Executor::allocate_user_class(Class *cls, std::span<OwcaValue> arguments) {
        OwcaValue return_value;
        prepare_allocate_user_class(return_value, cls, arguments);
        run();
        return return_value;
    }

    ExecutionFrame &Executor::prepare_execute_function(OwcaValue &return_value, RuntimeFunctions* runtime_functions, std::optional<OwcaValue> self_value, std::span<OwcaValue> arguments)
    {
		auto it = runtime_functions->functions.find(arguments.size() + (self_value ? 1 : 0));
		if (it == runtime_functions->functions.end()) {
			it = runtime_functions->functions.find(arguments.size());
			if (it == runtime_functions->functions.end() || it->second->is_method) {
				auto tmp = std::string{ "function " };
				tmp += runtime_functions->name;
				vm->throw_not_callable_wrong_number_of_params(std::move(tmp), arguments.size());
			}
		}

        auto &frame = push_new_frame();
        frame.initialize_execute_function(return_value, vm, runtime_functions, it->second, self_value, arguments);
        return frame;
    }

    void Executor::prepare_execute_call(OwcaValue &return_value, OwcaValue func, std::span<OwcaValue> arguments)
    {
		return func.visit(
			[&](OwcaIterator oi) -> void {
				if (!arguments.empty())
					vm->throw_not_callable_wrong_number_of_params("generator", (unsigned int)arguments.size());
                prepare_resume_generator(return_value, oi);
			},
			[&](OwcaFunctions of) -> void {
				auto runtime_functions = func.as_functions(vm).internal_value();
                prepare_execute_function(return_value, runtime_functions, of.self(), arguments);
			},
			[&](OwcaClass oc) -> void {
				auto cls = func.as_class(vm).internal_value();
				prepare_allocate_user_class(return_value, cls, arguments);
			},
			[&](const auto&) -> void {
				vm->throw_not_callable(func.type());
			}
		);
    }

    OwcaValue Executor::execute_call(OwcaValue func, std::span<OwcaValue> arguments)
    {
        OwcaValue return_value;
        prepare_execute_call(return_value, func, arguments);
        run();
        return return_value;
    }
    void Executor::construct_exception_and_throw(Class *cls, std::string_view arg) {
        OwcaValue ret;
        OwcaValue temp_arg = vm->create_string_from_view(arg);
        prepare_allocate_user_class(ret, cls, std::span{ &temp_arg, 1 }, true);
        run();
        throw ret.as_exception(vm);
    }




	void Executor::prepare_throw_too_many_elements(OwcaValue &return_value, size_t expected)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("too many values to unpack (expected {})", expected));
        prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}
	void Executor::prepare_throw_not_enough_elements(OwcaValue &return_value, size_t expected, size_t got)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("not enough values to unpack (expected {}, got {})", expected, got));
        prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}
	void Executor::prepare_throw_dictionary_changed(OwcaValue &return_value, bool is_dict)
	{
        OwcaValue temp_arg = is_dict ? vm->create_string_from_view("dictionary changed during iteration") : vm->create_string_from_view("set changed during iteration");
        prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}
	void Executor::prepare_throw_not_implemented(OwcaValue &return_value, std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
        prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}
	void Executor::prepare_throw_range_step_is_zero(OwcaValue &return_value)
	{
        OwcaValue temp_arg = vm->create_string_from_view("range step is zero");
        prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}
	void Executor::prepare_throw_division_by_zero(OwcaValue &return_value)
	{
        OwcaValue temp_arg = vm->create_string_from_view("division by zero");
        prepare_allocate_user_class(return_value, vm->c_math_exception, std::span{ &temp_arg, 1 }, true);
	}
	void Executor::prepare_throw_mod_division_by_zero(OwcaValue &return_value)
	{
        OwcaValue temp_arg = vm->create_string_from_view("modulo by zero");
        prepare_allocate_user_class(return_value, vm->c_math_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_cant_convert_to_float_message(OwcaValue &return_value, std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
        prepare_allocate_user_class(return_value, vm->c_math_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_cant_convert_to_float(OwcaValue &return_value, std::string_view type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("can't convert value of type `{}` to floating point", type));
		prepare_allocate_user_class(return_value, vm->c_math_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_cant_convert_to_integer(OwcaValue &return_value, Number val)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("can't convert {} to integer", val));
        prepare_allocate_user_class(return_value, vm->c_math_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_cant_convert_to_integer(OwcaValue &return_value, std::string_view type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("can't convert {} to integer", type));
        prepare_allocate_user_class(return_value, vm->c_math_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_not_a_number(OwcaValue &return_value, std::string_view type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} is not a number", type));
        prepare_allocate_user_class(return_value, vm->c_math_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_overflow(OwcaValue &return_value, std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
        prepare_allocate_user_class(return_value, vm->c_math_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_range_step_must_be_one_in_left_side_of_write_assign(OwcaValue &return_value)
	{
        OwcaValue temp_arg = vm->create_string_from_view("step of a range must be 1 in left side of write assignment");
		prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_cant_compare(OwcaValue &return_value, CompareKind kind, std::string_view left, std::string_view right)
	{
		const char *oper;
		switch(kind) {
		case CompareKind::Is: oper = "is"; break;
		case CompareKind::Eq: oper = "=="; break;
		case CompareKind::NotEq: oper = "!="; break;
		case CompareKind::LessEq: oper = "<=>"; break;
		case CompareKind::MoreEq: oper = ">="; break;
		case CompareKind::Less: oper = "<"; break;
		case CompareKind::More: oper = ">"; break;
		}
        OwcaValue temp_arg = vm->create_string_from_view(std::format("can't execute {} {} {}", left, oper, right));
        prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_string_too_large(OwcaValue &return_value, size_t size)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("string is too large ({} bytes)", size));
		prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}
	void Executor::prepare_throw_index_out_of_range(OwcaValue &return_value, std::string msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
		prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_value_not_indexable(OwcaValue &return_value, std::string_view type, std::string_view key_type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} is not indexable with key {}", type, key_type));
		prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_missing_member(OwcaValue &return_value, std::string_view type, std::string_view ident)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} doesn't have a member {}", type, ident));
		prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_cant_call(OwcaValue &return_value, std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
		prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_not_callable(OwcaValue &return_value, std::string_view type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} is not callable", type));
		prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}
	
	void Executor::prepare_throw_not_callable_wrong_number_of_params(OwcaValue &return_value, std::string_view type, unsigned int params)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} is not callable - wrong number of parameters ({})", type, params));
		prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_wrong_type(OwcaValue &return_value, std::string_view type, std::string_view expected)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("wrong type {} - expected {}", type, expected));
		prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_wrong_type(OwcaValue &return_value, std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
		prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_unsupported_operation_2(OwcaValue &return_value, std::string_view oper, std::string_view left, std::string_view right)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("can't execute {} {} {}", left, oper, right));
		prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_invalid_operand_for_mul_string(OwcaValue &return_value, std::string_view val)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("can't multiply string by {}", val));
		prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_missing_key(OwcaValue &return_value, std::string_view key)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("missing key {}", key));
		prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_not_hashable(OwcaValue &return_value, std::string_view type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} is not hashable", type));
		prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_value_cant_have_fields(OwcaValue &return_value, std::string_view type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} can't have fields", type));
		prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_missing_native(OwcaValue &return_value, std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
		prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_not_iterable(OwcaValue &return_value, std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
		prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_readonly(OwcaValue &return_value, std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
		prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_cant_return_value_from_generator(OwcaValue &return_value)
	{
        OwcaValue temp_arg = vm->create_string_from_view("can't return value from generator");
		prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_container_is_empty(OwcaValue &return_value)
	{
        OwcaValue temp_arg = vm->create_string_from_view("container is empty");
		prepare_allocate_user_class(return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}





	void Executor::throw_too_many_elements(size_t expected)
	{
        prepare_throw_too_many_elements(temporary_exception_return_value, expected);
        run();
        std::unreachable();
	}
	void Executor::throw_not_enough_elements(size_t expected, size_t got)
	{
        prepare_throw_not_enough_elements(expected, got);
        run();
        std::unreachable();
	}
	void Executor::throw_dictionary_changed(bool is_dict)
	{
        prepare_throw_dictionary_changed(is_dict);
        run();
        std::unreachable();
	}
	void Executor::throw_not_implemented(std::string_view msg)
	{
        prepare_throw_not_implemented(msg);
        run();
        std::unreachable();
	}
	void Executor::throw_range_step_is_zero()
	{
        prepare_throw_range_step_is_zero();
        run();
        std::unreachable();
	}
	void Executor::throw_division_by_zero()
	{
        prepare_throw_division_by_zero();
        run();
        std::unreachable();
	}
	void Executor::throw_mod_division_by_zero()
	{
        prepare_throw_mod_division_by_zero();
        run();
        std::unreachable();
	}

	void Executor::throw_cant_convert_to_float_message(std::string_view msg)
	{
        prepare_throw_cant_convert_to_float_message(msg);
        run();
        std::unreachable();
	}

	void Executor::throw_cant_convert_to_float(std::string_view type)
	{
        prepare_throw_cant_convert_to_float(type);
        run();
        std::unreachable();
	}

	void Executor::throw_cant_convert_to_integer(Number val)
	{
        prepare_throw_cant_convert_to_integer(val);
        run();
        std::unreachable();
	}

	void Executor::throw_cant_convert_to_integer(std::string_view type)
	{
        prepare_throw_cant_convert_to_integer(type);
        run();
        std::unreachable();
	}

	void Executor::throw_not_a_number(std::string_view type)
	{
        prepare_throw_not_a_number(type);
        run();
        std::unreachable();
	}

	void Executor::throw_overflow(std::string_view msg)
	{
        prepare_throw_overflow(msg);
        run();
        std::unreachable();
	}

	void Executor::throw_range_step_must_be_one_in_left_side_of_write_assign()
	{
        prepare_throw_range_step_must_be_one_in_left_side_of_write_assign();
        run();
        std::unreachable();
	}

	void Executor::throw_cant_compare(CompareKind kind, std::string_view left, std::string_view right)
	{
        prepare_throw_cant_compare(kind, left, right);
        run();
        std::unreachable();
	}

	void Executor::throw_string_too_large(size_t size)
	{
        prepare_throw_string_too_large(size);
        run();
        std::unreachable();
	}
	void Executor::throw_index_out_of_range(std::string msg)
	{
        prepare_throw_index_out_of_range(msg);
        run();
        std::unreachable();
	}

	void Executor::throw_value_not_indexable(std::string_view type, std::string_view key_type)
	{
        prepare_throw_value_not_indexable(type, key_type);
        run();
        std::unreachable();
	}

	void Executor::throw_missing_member(std::string_view type, std::string_view ident)
	{
        prepare_throw_missing_member(type, ident);
        run();
        std::unreachable();
	}

	void Executor::throw_cant_call(std::string_view msg)
	{
        prepare_throw_cant_call(msg);
        run();
        std::unreachable();
	}

	void Executor::throw_not_callable(std::string_view type)
	{
        prepare_throw_not_callable(type);
        run();
        std::unreachable();
	}
	
	void Executor::throw_not_callable_wrong_number_of_params(std::string_view type, unsigned int params)
	{
        prepare_throw_not_callable_wrong_number_of_params(type, params);
        run();
        std::unreachable();
	}

	void Executor::throw_wrong_type(std::string_view type, std::string_view expected)
	{
        prepare_throw_wrong_type(type, expected);
        run();
        std::unreachable();
	}

	void Executor::throw_wrong_type(std::string_view msg)
	{
        prepare_throw_wrong_type(msg);
        run();
        std::unreachable();
	}

	void Executor::throw_unsupported_operation_2(std::string_view oper, std::string_view left, std::string_view right)
	{
        prepare_throw_unsupported_operation_2(oper, left, right);
        run();
        std::unreachable();
	}

	void Executor::throw_invalid_operand_for_mul_string(std::string_view val)
	{
        prepare_throw_invalid_operand_for_mul_string(val);
        run();
        std::unreachable();
	}

	void Executor::throw_missing_key(std::string_view key)
	{
        prepare_throw_missing_key(key);
        run();
        std::unreachable();
	}

	void Executor::throw_not_hashable(std::string_view type)
	{
        prepare_throw_not_hashable(type);
        run();
        std::unreachable();
	}

	void Executor::throw_value_cant_have_fields(std::string_view type)
	{
        prepare_throw_value_cant_have_fields(type);
        run();
        std::unreachable();
	}

	void Executor::throw_missing_native(std::string_view msg)
	{
        prepare_throw_missing_native(msg);
        run();
        std::unreachable();
	}

	void Executor::throw_not_iterable(std::string_view msg)
	{
        prepare_throw_not_iterable(msg);
        run();
        std::unreachable();
	}

	void Executor::throw_readonly(std::string_view msg)
	{
        prepare_throw_readonly(msg);
        run();
        std::unreachable();
	}

	void Executor::throw_cant_return_value_from_generator()
	{
        prepare_throw_cant_return_value_from_generator();
        run();
        std::unreachable();
	}

	void Executor::throw_container_is_empty()
	{
        prepare_throw_container_is_empty();
        run();
        std::unreachable();
	}
}