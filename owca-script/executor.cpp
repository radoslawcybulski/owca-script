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

namespace OwcaScript::Internal {
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

    bool Executor::completed() const {
        return vm->current_stack_trace_index <= stack_top_level_index;
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

    void Executor::run_impl_opcodes_execute_compare(ExecuteBufferReader &reader, CompareKind kind) {
        auto jump_dest = reader.decode<std::uint32_t>();
        auto left = peek_value(2);
        auto right = peek_value(1);
        if (execute_compare(vm, kind, left, right)) {
            left = right;
            pop_values(1);
        }
        else {
            left = false;
            pop_values(1);
            reader.jump(jump_dest);
        }
    }
    void Executor::run_impl_opcodes(ExecutionFrame &frame, RuntimeFunction::ScriptFunction &sf)
    {
        static thread_local OwcaValue ignore_value;
        auto reader = ExecuteBufferReader{ frame.runtime_function->code, frame.code_position };
        bool exit = false;
        do {
            frame.code_position = reader.position();
            auto opcode = reader.decode<ExecuteBufferReader::Op>();
            switch(opcode) {
            case ExecuteBufferReader::Op::Class: {
                auto line = reader.line();

                auto native = reader.decode<bool>();
                auto name = reader.decode<std::string_view>();
                auto full_name = reader.decode<std::string_view>();
                auto base_class_count = reader.decode<std::uint32_t>();
                auto member_count = reader.decode<std::uint32_t>();
                auto all_variable_names = reader.decode<bool>();
                auto variable_name_count = reader.decode<std::uint32_t>();

                auto cls = vm->allocate<Class>(0, line, name, full_name, frame.runtime_function->code, base_class_count);
                //vm->set_currently_building_class(ClassToken{ cls });
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
                        if (auto impl = native_provider->native_class(full_name, ClassToken{ cls })) {
                            auto size = impl->native_storage_size();
                            cls->native_storage_pointers[cls] = { cls->native_storage_total, size };
                            cls->native_storage_total = (cls->native_storage_total + size + 15) & ~15;
                            cls->native = std::move(impl);
                        }
                    }
                    if (!cls->native) {
                        vm->throw_missing_native(std::format("missing native class {}", full_name));
                    }
                }
                break; }
            case ExecuteBufferReader::Op::ExprPopAndIgnore: {
                pop_values(1);
                break; }
            case ExecuteBufferReader::Op::ExprCompareEq: {
                run_impl_opcodes_execute_compare(reader, CompareKind::Eq);
                break; }
            case ExecuteBufferReader::Op::ExprCompareNotEq: {
                run_impl_opcodes_execute_compare(reader, CompareKind::NotEq);
                break; }
            case ExecuteBufferReader::Op::ExprCompareLessEq: {
                run_impl_opcodes_execute_compare(reader, CompareKind::LessEq);
                break; }
            case ExecuteBufferReader::Op::ExprCompareMoreEq: {
                run_impl_opcodes_execute_compare(reader, CompareKind::MoreEq);
                break; }
            case ExecuteBufferReader::Op::ExprCompareLess: {
                run_impl_opcodes_execute_compare(reader, CompareKind::Less);
                break; }
            case ExecuteBufferReader::Op::ExprCompareMore: {
                run_impl_opcodes_execute_compare(reader, CompareKind::More);
                break; }
            case ExecuteBufferReader::Op::ExprCompareIs: {
                run_impl_opcodes_execute_compare(reader, CompareKind::Is);
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
                auto [ new_str, new_str_pt ] = vm->precreate_string(size);
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
                assert(new_str.text().data() + size == new_str_pt);
                pop_values(expr_count);
                push_value(new_str);
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
                            vm->range_step_must_be_one_in_left_side_of_write_assign();
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
                                vm->throw_too_many_elements(state.value_indexes.size());
                            }
                            vm->set_identifier(state.value_indexes[index], *vv);
                            ++index;
                        }
                        if (index < state.value_indexes.size()) {
                            vm->throw_not_enough_elements(state.value_indexes.size(), index);
                        }
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
                    if (is_generator) {
                        fnc = vm->allocate<RuntimeFunction>(0, std::move(code), name, full_name, RuntimeFunction::NativeGenerator{});
                        auto &ng = std::get<RuntimeFunction::NativeGenerator>(fnc->data);
                        if (native_provider) {
                            if (auto impl = native_provider->native_generator(full_name)) {
                                ng.generator = std::move(*impl);
                            }
                        }
                        if (!ng.generator) {
                            vm->throw_missing_native(std::format("missing native generator {}", full_name));
                        }
                        ng.parameter_names.reserve(param_count);
                        for(auto i = 0u; i < param_count; ++i) {
                            auto var_name = reader.decode<std::string_view>();
                            ng.parameter_names.push_back(var_name);
                        }
                    }
                    else {
                        fnc = vm->allocate<RuntimeFunction>(0, std::move(code), name, full_name, RuntimeFunction::NativeFunction{});
                        auto &nf = std::get<RuntimeFunction::NativeFunction>(fnc->data);
                        if (native_provider) {
                            if (auto impl = native_provider->native_function(full_name)) {
                                nf.function = std::move(*impl);
                            }
                        }
                        if (!nf.function) {
                            vm->throw_missing_native(std::format("missing native function {}", full_name));
                        }
                        nf.parameter_names.reserve(param_count);
                        for(auto i = 0u; i < param_count; ++i) {
                            auto var_name = reader.decode<std::string_view>();
                            nf.parameter_names.push_back(var_name);
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
                            if (index == 0) sf.copy_from_parents.resize(size);
                            sf.copy_from_parents[index] = value;
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
                process_thrown_exception(exception.as_exception(vm));
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
                process_thrown_exception(*exception_in_progress);
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
        } while(!exit);
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

	void Executor::prepare_allocate_user_class(OwcaValue &return_value, Class *cls, std::span<OwcaValue> arguments) {
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

		auto it = cls->values.find(std::string_view{ "__init__" });
		if (it == cls->values.end()) {
			if (!arguments.empty()) {
				vm->throw_cant_call(std::format("type {} has no __init__ function defined - expected constructor's call with no parameters, instead got {} values", cls->full_name, arguments.size()));
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
					vm->throw_cant_call(std::format("type {} has __init__ variable, not a function", cls->full_name));
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
}