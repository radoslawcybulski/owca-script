#include "execution_frame.h"
#include "owca_exception.h"
#include "owca_iterator.h"
#include "owca_map.h"
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
#include <chrono>
#include <exception>
#include <iomanip>
#include <utility>

//#define OWCA_SCRIPT_EXEC_LOG
//#define MEASURE

namespace OwcaScript::Internal {
    static thread_local OwcaValue temporary_exception_return_value;

    Executor::Executor(VM *vm) : vm(vm), stack_top_level_index(currently_executing_frame_index()) {
    }

    ExecutionFrame &Executor::currently_executing_frame() {
        return *vm->stacktrace.back();
    }
    void Executor::push_new_frame(std::unique_ptr<ExecutionFrame> frame) {
#ifdef OWCA_SCRIPT_EXEC_LOG
        std::cout << "Pushing frame " << (void*)frame.get() << "\n";
#endif
        vm->stacktrace.push_back(std::move(frame));
    }
    void Executor::pop_frame() {
        exit = true;
        auto frame = std::move(vm->stacktrace.back());
        vm->stacktrace.pop_back();
#ifdef OWCA_SCRIPT_EXEC_LOG
        std::cout << "Popping frame " << (void*)frame.get() << std::endl;
#endif
        if (frame->constructor_move_self_to_return_value) {
            assert(frame->return_value);
            *frame->return_value = std::move(frame->values_[0]);
        }
        if (frame->is_iterator) {
            assert(frame->return_value);
            assert(frame->iterator_object);
            auto iter = frame->iterator_object->internal_value();

            if (frame->return_value->kind() == OwcaValueKind::Completed) {
                iter->generator = {};
                iter->completed = true;
            }
            else {
                iter->frame = std::move(frame);
                return;
            }
        }
        if (frame->dict_output) {
            frame->runtime_function->visit(
                [&](RuntimeFunction::ScriptFunction& sf) -> void {
                    for (auto i = 0u; i < sf.identifier_names.size(); ++i) {
                        auto key = vm->create_string_from_view(sf.identifier_names[i]);
                        (*frame->dict_output)[key] = frame->get_identifier(i);
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
#ifdef OWCA_SCRIPT_EXEC_LOG
        std::cout << "Deleting frame " << (void*)frame.get() << std::endl;
#endif
    }
    void Executor::process_thrown_exception(ExecuteBufferReader::Position *pos)
    {
        assert(currently_executing_frame().exception_in_progress);
        OwcaException exception = *currently_executing_frame().exception_in_progress;
        while(!completed()) {
            auto &frame = currently_executing_frame();
            frame.exception_in_progress = exception;
            while(frame.has_state()) {
                if (auto state = frame.try_state<ExecutionFrame::TryCatchState>()) {
                    frame.code_position = state->catches_pos;
                    if (pos) *pos = ExecuteBufferReader::Position{ state->catches_pos };
#ifdef OWCA_SCRIPT_EXEC_LOG
                    std::cout << __FILE__ << ":" << __LINE__ << ": setting code position (" << (void*)&frame.code_position << ") to " << frame.code_position << std::endl;
#endif
                    return;
                }
                frame.pop_state();
            }
            pop_frame();
            pos = nullptr;
            exit = true;
        }
        throw exception;
    }
    size_t Executor::currently_executing_frame_index() const {
        return vm->stacktrace.size();
    }
    bool Executor::completed() const {
        return currently_executing_frame_index() <= stack_top_level_index;
    }
    
	std::optional<std::tuple<Number, Number, Number>> Executor::parse_key(VM *vm, OwcaValue v, OwcaValue key, Number size) {
		return key.visit(
			[&](Number o) -> std::optional<std::tuple<Number, Number, Number>> {
				auto v = key.as_int(vm);
				if (v < 0) v += size;
				if (v < 0 || v >= size) {
					prepare_throw_index_out_of_range(std::format("index value {} is out of range for object of size {}", key, size));
                    return std::nullopt;
				}
				return std::make_tuple(v, v + 1, 0);
			},
			[&](OwcaRange o) -> std::optional<std::tuple<Number, Number, Number>> {
				auto lower = o.lower();
				auto upper = o.upper();
				auto step = o.step();
				if (lower < 0) lower += size;
				if (upper < 0) upper += size;
				if (step > 0) {
					if (lower >= upper) return std::tuple<Number, Number, Number>{ 0, 0, 1 };
					if (upper <= 0) return std::tuple<Number, Number, Number>{ 0, 0, 1 };
					if (lower >= size) return std::tuple<Number, Number, Number>{ 0, 0, 1 };
					if (lower < 0) {
						auto skip = std::max(Number(0), std::floor(-lower / step) - 1);
						lower += skip * step;
					}
					if (step == 1 && lower < 0) {
						lower = 0;
						if (upper < 0) upper = 0;
					}
					if (upper > size) upper = size;
					return std::tuple<Number, Number, Number>{ lower, upper, step };
				}
				else {
					if (lower <= upper) return std::tuple<Number, Number, Number>{ 0, 0, 1 };
					if (upper >= size) return std::tuple<Number, Number, Number>{ 0, 0, 1 };
					if (lower < 0) return std::tuple<Number, Number, Number>{ 0, 0, 1 };
					if (lower > size) {
						auto skip = std::max(Number(0), std::floor((lower - size) / -step) - 1);
						lower -= skip * step;
					}
					if (upper < 0) upper = Number{ -1 };
					return std::tuple<Number, Number, Number>{ lower, upper, step };
				}
			},
			[&](const auto&) -> std::optional<std::tuple<Number, Number, Number>> {
				prepare_throw_value_not_indexable(v.type(), key.type());
                return std::nullopt;
			}
		);
	}
	std::optional<size_t> Executor::verify_key(VM *vm, Number v, size_t size, OwcaValue orig_key, std::string_view name) {
		if (v < 0 || v >= (Number)size) {
			prepare_throw_index_out_of_range(std::format("index value {} is out of range for {} of size {}", orig_key, name, size));
            return std::nullopt;
		}
		auto v2 = (size_t)v;
		if (v2 != v) {
			prepare_throw_index_out_of_range(std::format("index value {} is out of range for {} of size {} - size_t overflows", orig_key, name, size));
            return std::nullopt;
		}
		return v2;
	}
	std::optional<std::pair<size_t, size_t>> Executor::verify_key(VM *vm, OwcaRange k, size_t size, OwcaValue orig_key, std::string_view name) {
		auto v1 = k.lower();
		auto v2 = k.upper();
		if (v2 <= v1)
			return std::pair<size_t, size_t>{ 0, 0 };
		if (v1 < 0) v1 = 0;
		if (v2 > (Number)size) v2 = size;
		size_t v3 = (size_t)v1, v4 = (size_t)v2;
		if (v3 != v1 || v2 != v4) {
			prepare_throw_index_out_of_range(std::format("index values {} is out of range for array of size {} - size_t overflows", orig_key, size));
            return std::nullopt;
		}
		return std::pair<size_t, size_t>{ v3, v4 };
	}

    bool Executor::run_impl_opcodes_execute_compare(ExecutionFrame &frame, ExecuteBufferReader::StartOfCode start_code, ExecuteBufferReader::Position &pos, CompareKind kind) {
        auto jump_dest = ExecuteBufferReader::decode<std::uint32_t>(start_code, pos, {});
        const auto last = ExecuteBufferReader::decode<bool>(start_code, pos, {});
        auto &left = frame.peek_value(2);
        auto right = frame.peek_value(1);
        auto res = execute_compare(vm, kind, left, right);
        switch(res) {
        case CompareResult::True:
            left = last ? OwcaValue{ true } : right;
            frame.pop_values(1);
            return false;
        case CompareResult::False:
            left = false;
            frame.pop_values(1);
            pos = ExecuteBufferReader::Position{ jump_dest };
            return false;
        case CompareResult::NotExecuted:
            prepare_throw_cant_compare(kind, left.type(), right.type());
            return true;
        }
        assert(false);
        return true;
    }

    void Executor::run_impl_opcodes(ExecutionFrame &frame, RuntimeFunction::ScriptFunction &sf)
    {
        static thread_local OwcaValue ignore_value;
        auto &code_object = frame.runtime_function->code;
        auto code_pos = ExecuteBufferReader::Position{ frame.code_position };
        auto start_code = ExecuteBufferReader::StartOfCode{ frame.runtime_function->code.code() };
        auto data_kinds = code_object.data_kinds();
#ifdef MEASURE        
        std::array<std::uint64_t, (size_t)Internal::ExecuteOp::_Count> times;
        std::array<std::uint64_t, (size_t)Internal::ExecuteOp::_Count> counts;

        for(auto &t : times) t = 0;
        for(auto &c : counts) c = 0;
#endif

        exit = false;
        do {
            //std::cout << "Running opcode at position " << reader.position() << std::endl;
#ifdef OWCA_SCRIPT_EXEC_LOG
            auto line = code_object.get_line_by_position(code_pos.pos);
#endif
            auto opcode = ExecuteBufferReader::decode<ExecuteBufferReader::Op>(start_code, code_pos, data_kinds);
            // static std::chrono::high_resolution_clock::time_point last_time = std::chrono::high_resolution_clock::now();
            // auto now = std::chrono::high_resolution_clock::now();
            // auto df = now - last_time;
            // last_time = now;
            // std::cout << std::setw(10) << (std::chrono::duration_cast<std::chrono::nanoseconds>(df).count()) << " ns ";
#ifdef OWCA_SCRIPT_EXEC_LOG
            std::cout << "Running opcode at line " << std::setw(4) << line.line << " position " << std::setw(5) << code_pos.pos << " temporaries " << std::setw(2) << frame.temporary_count() << 
                " states `" << frame.state_debug() << "` opcode " << std::setw(25) << to_string(opcode);
            if (frame.exception_in_progress) std::cout << " (exception in progress)";
            std::cout << std::endl;
#endif
            //last_time = std::chrono::high_resolution_clock::now();
#ifdef MEASURE            
            auto start = std::chrono::high_resolution_clock::now();
#endif
            switch(opcode) {
            case ExecuteBufferReader::Op::_Count:
                assert(false);
                break;
            case ExecuteBufferReader::Op::ClassInit: {
                auto line = code_object.get_line_by_position(code_pos.pos - 1);
                auto name = ExecuteBufferReader::decode<std::string_view>(start_code, code_pos, data_kinds);
                auto full_name = ExecuteBufferReader::decode<std::string_view>(start_code, code_pos, data_kinds);
                auto cls = vm->allocate<Class>(0, line, name, full_name, frame.runtime_function->code);
                frame.push_state<ExecutionFrame::ClassState>(name, full_name, cls);
                break; }
            case ExecuteBufferReader::Op::ClassCreate: {
                auto &state = frame.state<ExecutionFrame::ClassState>();

                auto cls = state.cls;
                frame.pop_state();

                auto native = ExecuteBufferReader::decode<bool>(start_code, code_pos, data_kinds);
                auto base_class_count = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                auto member_count = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                auto all_variable_names = ExecuteBufferReader::decode<bool>(start_code, code_pos, data_kinds);
                auto variable_name_count = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);

                auto base_classes = frame.peek_values(base_class_count, base_class_count);
                auto members = frame.peek_values(member_count + base_class_count, member_count);
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
                        auto var_name = ExecuteBufferReader::decode<std::string_view>(start_code, code_pos, data_kinds);
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
                        prepare_throw_missing_native(std::format("missing native class {}", cls->full_name));
                        break;
                    }
                }
                frame.pop_values(base_class_count + member_count);
                frame.push_value(OwcaClass{ cls });
                break; }
            case ExecuteBufferReader::Op::ExprPopAndIgnore: {
                frame.pop_values(1);
                break; }
            case ExecuteBufferReader::Op::ExprCompareEq: {
                exit = run_impl_opcodes_execute_compare(frame, start_code, code_pos, CompareKind::Eq);
                break; }
            case ExecuteBufferReader::Op::ExprCompareNotEq: {
                exit = run_impl_opcodes_execute_compare(frame, start_code, code_pos, CompareKind::NotEq);
                break; }
            case ExecuteBufferReader::Op::ExprCompareLessEq: {
                exit = run_impl_opcodes_execute_compare(frame, start_code, code_pos, CompareKind::LessEq);
                break; }
            case ExecuteBufferReader::Op::ExprCompareMoreEq: {
                exit = run_impl_opcodes_execute_compare(frame, start_code, code_pos, CompareKind::MoreEq);
                break; }
            case ExecuteBufferReader::Op::ExprCompareLess: {
                exit = run_impl_opcodes_execute_compare(frame, start_code, code_pos, CompareKind::Less);
                break; }
            case ExecuteBufferReader::Op::ExprCompareMore: {
                exit = run_impl_opcodes_execute_compare(frame, start_code, code_pos, CompareKind::More);
                break; }
            case ExecuteBufferReader::Op::ExprCompareIs: {
                exit = run_impl_opcodes_execute_compare(frame, start_code, code_pos, CompareKind::Is);
                break; }
            case ExecuteBufferReader::Op::ExprConstantEmpty: {
                frame.push_value(OwcaEmpty{});
                break; }
            case ExecuteBufferReader::Op::ExprConstantBool: {
                auto value = ExecuteBufferReader::decode<bool>(start_code, code_pos, data_kinds);
                frame.push_value(value);
                break; }
            case ExecuteBufferReader::Op::ExprConstantFloat: {
                auto value = ExecuteBufferReader::decode<Number>(start_code, code_pos, data_kinds);
                frame.push_value(value);
                break; }
            case ExecuteBufferReader::Op::ExprConstantString: {
                auto value = ExecuteBufferReader::decode<std::string_view>(start_code, code_pos, data_kinds);
                frame.push_value(vm->create_string_from_view(value));
                break; }
            case ExecuteBufferReader::Op::ExprConstantStringInterpolated: {
                auto strings = ExecuteBufferReader::decode<std::string_view>(start_code, code_pos, data_kinds);
                auto expr_count = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                size_t size = strings.size();
                auto values = frame.peek_values(expr_count, expr_count);
                for(auto i = 0u; i < expr_count; ++i) {
                    size += values[i].as_string(vm).size();
                }
                auto new_str = vm->precreate_string(size);
                auto new_str_pt = new_str->pointer();
                const char *strings_ptr = strings.data();
                for(auto i = 0u; i < expr_count; ++i) {
                    auto sz = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                    std::memcpy(new_str_pt, strings_ptr, sz);
                    new_str_pt += sz;
                    strings_ptr += sz;
                    auto str = values[i].as_string(vm);
                    std::memcpy(new_str_pt, str.text().data(), str.size());
                    new_str_pt += str.size();
                }
                auto remaining = strings.data() + strings.size() - strings_ptr;
                std::memcpy(new_str_pt, strings_ptr, remaining);
                new_str_pt += remaining;
                assert(new_str_pt == new_str->pointer() + new_str->size());
                frame.pop_values(expr_count);
                frame.push_value(OwcaString{ new_str });
                break; }
            case ExecuteBufferReader::Op::ExprIdentifierRead: {
                auto index = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                frame.push_value(frame.get_identifier(index));
                break; }
            case ExecuteBufferReader::Op::ExprIdentifierWrite: {
                auto index = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                auto &val = frame.peek_value(1);
                frame.set_identifier(index, val);
                break; }
            case ExecuteBufferReader::Op::ExprIdentifierFunctionWrite: {
                auto index = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                auto &val = frame.peek_value(1);
                frame.set_identifier_function(vm, index, val);
                break; }
            case ExecuteBufferReader::Op::ExprMemberRead: {
                auto self = frame.peek_value(1);
                auto member = ExecuteBufferReader::decode<std::string_view>(start_code, code_pos, data_kinds);
                frame.peek_value(1) = vm->member(self, member);
                break; }
            case ExecuteBufferReader::Op::ExprMemberWrite: {
                auto val_to_write = frame.peek_value(1);
                auto self = frame.peek_value(2);
                auto member = ExecuteBufferReader::decode<std::string_view>(start_code, code_pos, data_kinds);
                vm->member(self, member, val_to_write);
                frame.peek_value_and_make_top(2) = val_to_write;
                break; }
            case ExecuteBufferReader::Op::ExprOper1BinNeg: {
                auto &left = frame.peek_value(1);
                left = -(std::int64_t)left.as_float(vm);
                break; }
            case ExecuteBufferReader::Op::ExprOper1LogNot: {
                auto &left = frame.peek_value(1);
                left = !left.is_true();
                break; }
            case ExecuteBufferReader::Op::ExprOper1Negate: {
                auto &left = frame.peek_value(1);
                left = -left.as_float(vm);
                break; }
            case ExecuteBufferReader::Op::ExprRetTrueAndJumpIfTrue: {
                auto jump_dest = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                if (frame.peek_value(1).is_true()) {
                    code_pos.pos = jump_dest;
                }
                else {
                    frame.pop_values(1);
                }
                break; }
            case ExecuteBufferReader::Op::ExprRetFalseAndJumpIfFalse: {
                auto jump_dest = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                if (!frame.peek_value(1).is_true()) {
                    code_pos.pos = jump_dest;
                }
                else {
                    frame.pop_values(1);
                }
                break; }
            case ExecuteBufferReader::Op::ExprToString: {
                auto v = frame.peek_value(1);
                if (v.kind() == OwcaValueKind::String) {
                    break;
                }
                frame.peek_value(1) = vm->create_string_from_view(v.to_string());
                break; }
            case ExecuteBufferReader::Op::ExprToIterator: {
                auto &val = frame.peek_value(1);
                if (val.kind() != OwcaValueKind::Iterator) {
                    auto func = vm->try_member(val, "__iter__");
                    if (!func) {
                        prepare_throw_not_iterable(val.type());
                        break;
                    }
                    prepare_execute_call(val, *func, {});
                }
                break; }
            case ExecuteBufferReader::Op::ExprOper2BinOr: {
                run_impl_opcodes_execute_expr_oper2<TagBinOr>(frame);
                break; }
            case ExecuteBufferReader::Op::ExprOper2BinAnd: {
                run_impl_opcodes_execute_expr_oper2<TagBinAnd>(frame);
                break; }
            case ExecuteBufferReader::Op::ExprOper2BinXor: {
                run_impl_opcodes_execute_expr_oper2<TagBinXor>(frame);
                break; }
            case ExecuteBufferReader::Op::ExprOper2BinLShift: {
                run_impl_opcodes_execute_expr_oper2<TagBinLShift>(frame);
                break; }
            case ExecuteBufferReader::Op::ExprOper2BinRShift: {
                run_impl_opcodes_execute_expr_oper2<TagBinRShift>(frame);
                break; }
            case ExecuteBufferReader::Op::ExprOper2Add: {
                run_impl_opcodes_execute_expr_oper2<TagAdd>(frame);
                break; }
            case ExecuteBufferReader::Op::ExprOper2Sub: {
                run_impl_opcodes_execute_expr_oper2<TagSub>(frame);
                break; }
            case ExecuteBufferReader::Op::ExprOper2Mul: {
                run_impl_opcodes_execute_expr_oper2<TagMul>(frame);
                break; }
            case ExecuteBufferReader::Op::ExprOper2Div: {
                run_impl_opcodes_execute_expr_oper2<TagDiv>(frame);
                break; }
            case ExecuteBufferReader::Op::ExprOper2Mod: {
                run_impl_opcodes_execute_expr_oper2<TagMod>(frame);
                break; }
            case ExecuteBufferReader::Op::ExprOper2MakeRange: {
                auto mode = ExecuteBufferReader::decode<std::uint8_t>(start_code, code_pos, data_kinds);
                Number first, second, third;
                if (mode & 4) {
                    third = frame.peek_value(1).as_float(vm);
                    frame.pop_values(1);
                }
                else {
                    third = 1;
                }
                if (mode & 2) {
                    second = frame.peek_value(1).as_float(vm);
                    frame.pop_values(1);
                }
                else {
                    second = std::numeric_limits<Number>::max();
                }
                if (mode & 1) {   
                    first = frame.peek_value(1).as_float(vm);
                    frame.pop_values(1);
                }
                else {
                    first = 0;
                }
                if (third == 0) {
                    prepare_throw_range_step_is_zero();
                    break;
                }
                auto ret = vm->allocate<Range>(0);
                ret->from = first;
                ret->to = second;
                ret->step = third;
                frame.push_value(OwcaRange{ ret });
                break; }
            case ExecuteBufferReader::Op::ExprOper2IndexRead: {
                auto key = frame.peek_value(1);
                auto v = frame.peek_value(2);

                auto orig_key = key;

                frame.peek_value_and_make_top(2) = v.visit(
                    [&](const OwcaString& o) -> OwcaValue {
                        const auto size = (Number)o.internal_value()->size();
                        if (size != o.internal_value()->size()) {
                            prepare_throw_index_out_of_range(std::format("string size {} is too large for Number size to properly handle indexing", o.internal_value()->size()));
                            return {};
                        }
                        auto r = parse_key(vm, key, key, size);
                        if (!r) {
                            return {};
                        }
                        auto [ lower, upper, step ] = *r;
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
                            prepare_throw_index_out_of_range(std::format("array size {} is too large for Number size to properly handle indexing", o.internal_value()->values.size()));
                            return {};
                        }

                        auto r = parse_key(vm, key, key, size);
                        if (!r) {
                            return {};
                        }
                        auto [ lower, upper, step ] = *r;
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
                            prepare_throw_index_out_of_range(std::format("tuple size {} is too large for Number size to properly handle indexing", o.internal_value()->values.size()));
                            assert(exit);
                            return {};
                        }

                        auto r = parse_key(vm, key, key, size);
                        if (!r) {
                            assert(exit);
                            return {};
                        }
                        auto [ lower, upper, step ] = *r;
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
                        prepare_throw_value_not_indexable(v.type());
                        assert(exit);
                        return {};
                    }
                );
                break; }
            case ExecuteBufferReader::Op::ExprOper2IndexWrite: {
                auto value = frame.peek_value(1);
                auto key = frame.peek_value(2);
                auto v = frame.peek_value(3);

                auto orig_key = key;

                frame.peek_value_and_make_top(3) = v.visit(
                    [&](const OwcaMap& o) -> OwcaValue {
                        o.internal_value()->dict.write(key, value);
                        return value;
                    },
                    [&](OwcaArray o) -> OwcaValue {
                        const auto size = (Number)o.internal_value()->values.size();
                        if (size != o.internal_value()->values.size()) {
                            prepare_throw_index_out_of_range(std::format("array size {} is too large for Number size to properly handle indexing", o.internal_value()->values.size()));
                            assert(exit);
                            return {};
                        }
                        auto r = parse_key(vm, key, key, size);
                        if (!r) {
                            assert(exit);
                            return {};
                        }
                        auto [ lower, upper, step ] = *r;
                        if (step == 0) {
                            o[lower] = value;
                            return value;
                        }
                        if (step != 1) {
                            prepare_throw_range_step_must_be_one_in_left_side_of_write_assign();
                            assert(exit);
                            return {};
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
                        prepare_throw_readonly("tuple is readonly");
                        assert(exit);
                        return {};
                    },
                    [&](const auto&) -> OwcaValue {
                        prepare_throw_value_not_indexable(v.type());
                        assert(exit);
                        return {};
                    }
                );
                break; }
            case ExecuteBufferReader::Op::ExprOperXCall: {
                auto size = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                auto args = frame.peek_values(size - 1, size - 1);
                auto &fnc = frame.peek_value_and_make_top(size);
                prepare_execute_call(fnc, fnc, args);
                break; }
            case ExecuteBufferReader::Op::ExprOperXCreateArray: {
                auto size = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                auto args = frame.peek_values(size, size);
                auto arguments = std::deque<OwcaValue>{ args.begin(), args.end() };
                frame.peek_value_and_make_top(size) = vm->create_array(std::move(arguments));
                break; }
            case ExecuteBufferReader::Op::ExprOperXCreateTuple: {
                auto size = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                auto args = frame.peek_values(size, size);
                auto arguments = std::vector<OwcaValue>{ args.begin(), args.end() };
                frame.peek_value_and_make_top(size) = vm->create_tuple(std::move(arguments));
                break; }
            case ExecuteBufferReader::Op::ExprOperXCreateSet: {
                auto size = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                auto args = frame.peek_values(size, size);
                frame.peek_value_and_make_top(size) = vm->create_set(args);
                break; }
            case ExecuteBufferReader::Op::ExprOperXCreateMap: {
                auto size = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                auto args = frame.peek_values(size, size);
                frame.peek_value_and_make_top(size) = vm->create_map(args);
                break; }
            case ExecuteBufferReader::Op::ForInit: {
                auto iterator = frame.peek_value(1).as_iterator(vm);
                frame.pop_values(1);
                auto &state = frame.push_state<ExecutionFrame::ForState>(iterator);
                state.end_position = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                state.loop_index = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                state.loop_control_depth = ExecuteBufferReader::decode<std::uint8_t>(start_code, code_pos, data_kinds);
                state.continue_position = code_pos.pos;
                break; }
            case ExecuteBufferReader::Op::ForCondition: {
                auto &state = frame.state<ExecutionFrame::ForState>();
                state.index++;
                if (state.loop_index != std::numeric_limits<std::uint32_t>::max()) {
                    frame.set_identifier(state.loop_index, state.index);
                }
                frame.push_value({});
                prepare_resume_generator(frame.peek_value(1), state.iterator);
                break; }
            case ExecuteBufferReader::Op::ForNext: {
                auto val = frame.peek_value(1);
                if (val.kind() == OwcaValueKind::Completed) {
                    auto &state = frame.state<ExecutionFrame::ForState>();
                    code_pos = ExecuteBufferReader::Position{ state.end_position };
                    frame.pop_values(1);
                }
                break; }
            case ExecuteBufferReader::Op::ForCompleted: {
                auto &state = frame.state<ExecutionFrame::ForState>();
                frame.pop_state();
                break; }
            case ExecuteBufferReader::Op::Function: {
                auto name = ExecuteBufferReader::decode<std::string_view>(start_code, code_pos, data_kinds);
                auto full_name = ExecuteBufferReader::decode<std::string_view>(start_code, code_pos, data_kinds);
                auto is_native = ExecuteBufferReader::decode<bool>(start_code, code_pos, data_kinds);
                auto is_generator = ExecuteBufferReader::decode<bool>(start_code, code_pos, data_kinds);
                auto is_method = ExecuteBufferReader::decode<bool>(start_code, code_pos, data_kinds);
                auto param_count = ExecuteBufferReader::decode<std::uint16_t>(start_code, code_pos, data_kinds);
                auto value_count = ExecuteBufferReader::decode<std::uint16_t>(start_code, code_pos, data_kinds);
                auto temporaries_count = ExecuteBufferReader::decode<std::uint16_t>(start_code, code_pos, data_kinds);
                auto state_count = ExecuteBufferReader::decode<std::uint16_t>(start_code, code_pos, data_kinds);
                std::vector<std::string_view> identifier_names;
                identifier_names.resize(value_count);
                for(auto &n : identifier_names) n = ExecuteBufferReader::decode<std::string_view>(start_code, code_pos, data_kinds);

                RuntimeFunction *fnc = nullptr;
                if (is_native) {
                    auto &native_provider = code_object.native_code_provider();
                    std::optional<ClassToken> class_;
                    if (frame.has_state()) {
                        if (auto state = frame.try_state<ExecutionFrame::ClassState>()) {
                            class_ = ClassToken{ state->cls };
                        }
                    }
                    if (is_generator) {
                        fnc = vm->allocate<RuntimeFunction>(0, code_object, name, full_name, RuntimeFunction::NativeGenerator{});
                        auto &ng = std::get<RuntimeFunction::NativeGenerator>(fnc->data);
                        ng.parameter_names = std::move(identifier_names);
                        if (native_provider) {
                            if (auto impl = native_provider->native_generator(full_name, class_, FunctionToken{ fnc }, ng.parameter_names)) {
                                ng.generator = std::move(*impl);
                            }
                        }
                        if (!ng.generator) {
                            prepare_throw_missing_native(std::format("missing native generator {}", full_name));
                            assert(exit);
                            break;
                        }
                    }
                    else {
                        fnc = vm->allocate<RuntimeFunction>(0, code_object, name, full_name, RuntimeFunction::NativeFunction{});
                        auto &nf = std::get<RuntimeFunction::NativeFunction>(fnc->data);
                        nf.parameter_names = std::move(identifier_names);
                        if (native_provider) {
                            if (auto impl = native_provider->native_function(full_name, class_, FunctionToken{ fnc }, nf.parameter_names)) {
                                nf.function = std::move(*impl);
                            }
                        }
                        if (!nf.function) {
                            prepare_throw_missing_native(std::format("missing native function {}", full_name));
                            assert(exit);
                            break;
                        }
                    }
                }
                else {
                    fnc = vm->allocate<RuntimeFunction>(0, code_object, name, full_name, RuntimeFunction::ScriptFunction{});
                    auto &sf = std::get<RuntimeFunction::ScriptFunction>(fnc->data);
                    sf.is_generator = is_generator;
                    sf.identifier_names = std::move(identifier_names);
                    auto copy_from_parent_count = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                    sf.copy_from_parents.reserve(copy_from_parent_count);
                    for(auto i = 0u; i < copy_from_parent_count; ++i) {
                        auto index_in_parent = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                        auto identifier_index = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                        sf.copy_from_parents.push_back({ index_in_parent, identifier_index });
                    }
                    auto next = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                    sf.entry_point = code_pos.pos;
                    code_pos = ExecuteBufferReader::Position{ next };

                    sf.values_from_parents.reserve(sf.copy_from_parents.size());
                    for(auto c : sf.copy_from_parents) {
             			sf.values_from_parents.push_back(frame.get_identifier(c.index_in_parent));                        
                    }
                }
                fnc->param_count = param_count;
                fnc->max_temporaries = temporaries_count;
                fnc->max_states = state_count;
                fnc->max_values = value_count;
                fnc->is_method = is_method;
                auto rfs = VM::get(vm).allocate<RuntimeFunctions>(0, name, full_name);
                rfs->functions[fnc->param_count] = fnc;
                frame.push_value(OwcaFunctions{ rfs });
                break; }
            case ExecuteBufferReader::Op::If: {
                auto val = frame.peek_value(1).is_true();
                auto else_position = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                if (!val) {
                    code_pos = ExecuteBufferReader::Position{ else_position };
                }
                frame.pop_values(1);
                break; }
            case ExecuteBufferReader::Op::LoopControlBreak: {
                auto depth = ExecuteBufferReader::decode<std::uint8_t>(start_code, code_pos, data_kinds);
                while(true) {
                    assert(frame.has_state());
                    if (auto s = frame.try_state<ExecutionFrame::ForState>()) {
                        if (s->loop_control_depth == depth) {
                            code_pos = ExecuteBufferReader::Position{ s->end_position };
                            break;
                        }
                    }
                    else if (auto s = frame.try_state<ExecutionFrame::WhileState>()) {
                        if (s->loop_control_depth == depth) {
                            code_pos = ExecuteBufferReader::Position{ s->end_position };
                            break;
                        }
                    }
                    frame.pop_state();
                }
                break; }
            case ExecuteBufferReader::Op::LoopControlContinue: {
                auto depth = ExecuteBufferReader::decode<std::uint8_t>(start_code, code_pos, data_kinds);
                while(true) {
                    assert(frame.has_state());
                    if (auto s = frame.try_state<ExecutionFrame::ForState>()) {
                        if (s->loop_control_depth == depth) {
                            code_pos = ExecuteBufferReader::Position{ s->continue_position };
                            break;
                        }
                    }
                    else if (auto s = frame.try_state<ExecutionFrame::WhileState>()) {
                    if (s->loop_control_depth == depth) {
                            code_pos = ExecuteBufferReader::Position{ s->continue_position };
                            break;
                        }
                    }
                    frame.pop_state();
                }
                break; }
            case ExecuteBufferReader::Op::Return: {
                if (frame.is_iterator) {
                    *frame.return_value = OwcaCompleted{};
                    frame.iterator_object->internal_value()->completed = true;
                }
                else {
                    *frame.return_value = OwcaEmpty{};
                }
                pop_frame();
                assert(exit);
                return; }
            case ExecuteBufferReader::Op::ReturnValue: {
                *frame.return_value = frame.peek_value(1);
                frame.pop_values(1);
                if (frame.is_iterator) {
                    frame.iterator_object->internal_value()->completed = true;
                }
                pop_frame();
                assert(exit);
                return; }
            case ExecuteBufferReader::Op::Throw: {
                auto exception = frame.peek_value(1);
                frame.pop_values(1);
                frame.exception_in_progress = exception.as_exception(vm);
                process_thrown_exception(&code_pos);
                break; }
            case ExecuteBufferReader::Op::TryInit: {
                auto &state = frame.push_state<ExecutionFrame::TryCatchState>();
                state.parent_exception = frame.exception_in_progress;
                frame.exception_in_progress.reset();
                state.begin_pos = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                state.end_pos = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                state.catches_pos = code_pos.pos;
                code_pos = ExecuteBufferReader::Position{ state.begin_pos };
                state.temporary_ptr = frame.temporary_;
                break; }
            case ExecuteBufferReader::Op::TryCompleted: {
                auto &state = frame.state<ExecutionFrame::TryCatchState>();
                frame.exception_in_progress = state.parent_exception;
                frame.pop_state();
                break; }
            case ExecuteBufferReader::Op::TryCatchType: {
                auto values = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                auto ident = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                auto skip_jump = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);

                auto exc_types = frame.peek_values(values, values);
                frame.pop_values(values);
                assert(frame.exception_in_progress);

                bool found = false;
                for(auto e : exc_types) {
                    auto exc_type = e.as_class(vm);
                    if (frame.exception_in_progress->type().has_base_class(exc_type)) {
                        found = true;
                        break;
                    }
                }
                if (found) {
                    if (ident != std::numeric_limits<std::uint32_t>::max()) {
                        frame.set_identifier(ident, *frame.exception_in_progress);
                    }
                }
                else {
                    code_pos = ExecuteBufferReader::Position{ skip_jump };
                }
                break; }
            case ExecuteBufferReader::Op::TryCatchTypeCompleted: {
                process_thrown_exception(&code_pos);
                break; }
            case ExecuteBufferReader::Op::TryBlockCompleted: {
                assert(frame.exception_in_progress);
                frame.exception_in_progress = std::nullopt;
                auto pos = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                code_pos = ExecuteBufferReader::Position{ pos };
                break; }
            case ExecuteBufferReader::Op::WhileInit: {
                auto &state = frame.push_state<ExecutionFrame::WhileState>();
                state.end_position = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                state.loop_index = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                state.loop_control_depth = ExecuteBufferReader::decode<std::uint8_t>(start_code, code_pos, data_kinds);
                state.continue_position = code_pos.pos;
                break; }
            case ExecuteBufferReader::Op::WhileCondition: {
                auto &state = frame.state<ExecutionFrame::WhileState>();
                state.index++;
                if (state.loop_index != std::numeric_limits<std::uint32_t>::max()) {
                    frame.set_identifier(state.loop_index, state.index);
                }
                break; }
            case ExecuteBufferReader::Op::WhileNext: {
                auto &state = frame.state<ExecutionFrame::WhileState>();

                auto value = frame.peek_value(1).as_bool(vm);
                frame.pop_values(1);
                if (!value) {
                    code_pos = ExecuteBufferReader::Position{ state.end_position };
                }
                break; }
            case ExecuteBufferReader::Op::WhileCompleted: {
                frame.pop_state();
                break; }
            case ExecuteBufferReader::Op::WithInitPrepare: {
                auto &state = frame.push_state<ExecutionFrame::WithState>();
                auto &obj = frame.peek_value(1);
                state.context = obj;
                auto mbm = vm->member(obj, "__enter__");
                prepare_execute_call(obj, mbm, {});
                assert(exit);
                break; }
            case ExecuteBufferReader::Op::WithInit: {
                auto &state = frame.state<ExecutionFrame::WithState>();
                state.entered = true;
                auto index = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                if (index != std::numeric_limits<std::uint32_t>::max()) {
                    frame.set_identifier(index, frame.peek_value(1));
                }
                frame.pop_values(1);
                break; }
            case ExecuteBufferReader::Op::WithCompleted: {
                auto &state = frame.state<ExecutionFrame::WithState>();
                if (state.entered) {
                    auto mbm = vm->member(state.context, "__exit__");
                    prepare_execute_call(ignore_value, mbm, {});
                    assert(exit);
                }
                frame.pop_state();
                break; }
            case ExecuteBufferReader::Op::Yield: {
                assert(frame.runtime_function->is_generator());
                *frame.return_value = frame.peek_value(1);
                frame.code_position = code_pos.pos;
                frame.pop_values(1);
                pop_frame();
                assert(exit);
                return; }
            case ExecuteBufferReader::Op::Jump: {
                auto dest = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                code_pos = ExecuteBufferReader::Position{ dest };
                break; }
            }
            frame.code_position = code_pos.pos;
#ifdef MEASURE
            auto end = std::chrono::high_resolution_clock::now();
            times[(size_t)opcode] += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            counts[(size_t)opcode]++;
#endif
            //std::cout << "setting code position (" << (void*)&frame.code_position << ") to " << frame.code_position << std::endl;
        } while(!exit);

#ifdef MEASURE
        std::cout << "\n\n";
        for(auto i = 0u; i < (size_t)ExecuteBufferReader::Op::_Count; ++i) {
            auto t = times[i];
            auto c = counts[i];
            if (t > 0) {
                std::cout << std::setw(40) << to_string((Internal::ExecuteOp)i) << " " << (t / c) << " (count " << c << ")\n";
            }
        }
#endif
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
            prepare_throw_division_by_zero();
            assert(exit);
            return 0;
        }
        return left / right;
    }
    Number Executor::expr_oper_2(Executor::TagMod, Number left, Number right) {
        if (right == 0) {
            prepare_throw_division_by_zero();
            assert(exit);
            return 0;
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
    Number Executor::expr_oper_2(TagBinOr, Number left, Number right) {
        return (std::uint64_t)left | (std::uint64_t)right;
    }
    Number Executor::expr_oper_2(TagBinAnd, Number left, Number right) {
        return (std::uint64_t)left & (std::uint64_t)right;
    }
    Number Executor::expr_oper_2(TagBinXor, Number left, Number right) {
        return (std::uint64_t)left ^ (std::uint64_t)right;
    }
    Number Executor::expr_oper_2(TagBinLShift, Number left, Number right) {
        return (std::uint64_t)left << (std::uint64_t)right;
    }
    Number Executor::expr_oper_2(TagBinRShift, Number left, Number right) {
        return (std::uint64_t)left >> (std::uint64_t)right;
    }

    template <typename A, typename B, typename C> OwcaEmpty Executor::expr_oper_2(A, B b, C c) {
        prepare_throw_unsupported_operation_2(tag_name<A>, OwcaValue{ b }.type(), OwcaValue{ c }.type());
        assert(exit);
        return {};
    }
    template <typename Tag> void Executor::run_impl_opcodes_execute_expr_oper2(ExecutionFrame &frame) {
        auto right = frame.peek_value(1);
        auto left = frame.peek_value(2);
        auto &ret = frame.peek_value_and_make_top(2);
        ret = left.visit([&](auto left_val) -> OwcaValue {
            return right.visit([&](auto right_val) -> OwcaValue {
                return expr_oper_2(Tag{}, left_val, right_val);
            });
        });
    }
    void Executor::run() {
        while(!completed()) {
            auto &frame = currently_executing_frame();
#ifdef OWCA_SCRIPT_EXEC_LOG
            std::cout << "Executing frame at depth " << currently_executing_frame_index() << " (" << (void*)&frame << ") function " << frame.runtime_function->full_name;
            if (auto q = std::get_if<RuntimeFunction::ScriptFunction>(&frame.runtime_function->data)) {
                std::cout << " with code position " << frame.code_position << " (" << (void*)&frame.code_position << ")";
            }
            std::cout << std::endl;
#endif
            visit_variant(
                frame.runtime_function->data,
                [&](RuntimeFunction::ScriptFunction& sf) -> void {
                    if (frame.is_iterator && !frame.iterator_object) {
                        auto obj = vm->allocate<Iterator>(0, false);
                        frame.iterator_object = obj;
                        *frame.return_value = OwcaIterator{ obj };
                        pop_frame();
                        return;
                    }
                    auto frame_index = currently_executing_frame_index();
                    run_impl_opcodes(frame, sf);
#ifdef OWCA_SCRIPT_EXEC_LOG
                    std::cout << "Suspended frame at depth " << frame_index << std::endl;
#endif
                },
                [&](RuntimeFunction::NativeFunction& nf) -> void {
                    try {
                        *frame.return_value = nf.function(vm, std::span{ frame.values_, frame.max_values });
                        pop_frame();
                    }
                    catch(OwcaException e) {
                        frame.exception_in_progress = e;
                        process_thrown_exception(nullptr);
                    }
                    catch(const std::exception &e) {
                        prepare_throw_cpp_exception(std::format("C++ exception during execution of native generator: {}", e.what()));
                    }
                    catch(...) {
                        prepare_throw_cpp_exception("Unknown C++ exception during execution of native generator");
                    }
                },
                [&](RuntimeFunction::NativeGenerator& ngf) -> void {
                    if (!frame.iterator_object) {
                        frame.iterator_object = vm->allocate<Iterator>(0, true);
                        frame.iterator_object->internal_value()->generator = ngf.generator(vm, std::span{ frame.values_, frame.max_values });
                        *frame.return_value = *frame.iterator_object;
                        pop_frame();
                        return;
                    }
                    if (frame.iterator_object->internal_value()->completed) {
                        *frame.return_value = OwcaCompleted{};
                        pop_frame();
                        return;
                    }
                    try {
                        frame.iterator_object->internal_value()->first_time = false;
                        auto val = frame.iterator_object->internal_value()->generator->next();
                        if (!val) {
                            frame.iterator_object->internal_value()->completed = true;
                            *frame.return_value = frame.iterator_object->internal_value()->last_value = OwcaCompleted{};
                            pop_frame();
                            return;
                        }
                        *frame.return_value = frame.iterator_object->internal_value()->last_value = *val;
                        if (frame.return_value->kind() == OwcaValueKind::Completed) {
                            frame.iterator_object->internal_value()->completed = true;
                        }
                        pop_frame();
                        return;
                    }
                    catch(OwcaException e) {
                        frame.exception_in_progress = e;
                        process_thrown_exception(nullptr);
                    }
                    catch(const std::exception &e) {
                        prepare_throw_cpp_exception(std::format("C++ exception during execution of native generator: {}", e.what()));
                    }
                    catch(...) {
                        prepare_throw_cpp_exception("Unknown C++ exception during execution of native generator");
                    }
                }
            );
        }
    }
    void Executor::run_and_throw() {
        auto return_value = currently_executing_frame().return_value;
        run();
        auto exc = return_value->as_exception(vm);
        throw exc;
    }
    void Executor::prepare_execute_code_block(OwcaValue &return_value, const OwcaCode &oc) {
        auto frame = ExecutionFrame::create(0, 1, 0);
        frame->initialize_code_block(return_value, vm, oc);
        push_new_frame(std::move(frame));
    }
    void Executor::prepare_execute_main_function(OwcaValue &return_value, RuntimeFunctions* runtime_functions, std::optional<OwcaMap> arguments) {
        assert(runtime_functions->functions.size() == 1);
        auto it = runtime_functions->functions.find(0u);
        assert(it != runtime_functions->functions.end());
        auto frame = ExecutionFrame::create(it->second);
        frame->initialize_main_block_function(return_value, vm, runtime_functions, arguments);
        push_new_frame(std::move(frame));
    }

	OwcaValue Executor::execute_code_block(const OwcaCode &oc, std::optional<OwcaMap> values, OwcaMap *dict_output)
	{
        OwcaValue temp;
		
        prepare_execute_code_block(temp, oc);
        run();
		auto fnc = temp.as_functions(vm);
        assert(fnc.internal_self_object() == nullptr);
		prepare_execute_main_function(temp, fnc.internal_value(), values);
        currently_executing_frame().dict_output = dict_output;
        run();
        return temp;
    }

    void Executor::prepare_resume_generator(OwcaValue &return_value, OwcaIterator oi)
    {
		if (oi.internal_value()->completed) {
            return_value = OwcaCompleted{};
			return;
		}
        push_new_frame(std::move(oi.internal_value()->frame));
        currently_executing_frame().return_value = &return_value;
        oi.internal_value()->first_time = false;
        exit = true;
    }

    OwcaValue Executor::resume_generator(OwcaIterator oi)
	{
		if (oi.internal_value()->completed) {
			return OwcaCompleted{};
		}
		prepare_resume_generator(oi.internal_value()->last_value, oi);
		run();
		return oi.internal_value()->last_value;
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
            if (currently_executing_frame_index() > 1) {
                oe.internal_value()->parent_exception = vm->stacktrace[currently_executing_frame_index() - 2]->exception_in_progress;
                vm->stacktrace[currently_executing_frame_index() - 2]->exception_in_progress.reset();
            }
        }
		auto it = cls->values.find(std::string_view{ "__init__" });
		if (it == cls->values.end()) {
			if (!arguments.empty()) {
				prepare_throw_cant_call(std::format("type {} has no __init__ function defined - expected constructor's call with no parameters, instead got {} values", cls->full_name, arguments.size()));
                assert(exit);
                return;
			}
            return_value = obj;
		}
		else {
			visit_variant(it->second,
				[&](RuntimeFunctions *rf) -> void {
                    if (prepare_execute_function(return_value, rf, obj, arguments)) {
                        auto &frame = currently_executing_frame();
                        if (!cls->reload_self)
                            frame.constructor_move_self_to_return_value = true;
                    }
				},
				[&](Class* var) -> void {
					prepare_throw_cant_call(std::format("type {} has __init__ variable, not a function", cls->full_name));
                    assert(exit);
                    return;
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

    bool Executor::prepare_execute_function(OwcaValue &return_value, RuntimeFunctions* runtime_functions, std::optional<OwcaValue> self_value, std::span<OwcaValue> arguments)
    {
		auto it = runtime_functions->functions.find(arguments.size() + (self_value ? 1 : 0));
		if (it == runtime_functions->functions.end()) {
			it = runtime_functions->functions.find(arguments.size());
			if (it == runtime_functions->functions.end() || it->second->is_method) {
				auto tmp = std::string{ "function " };
				tmp += runtime_functions->name;
				prepare_throw_not_callable_wrong_number_of_params(std::move(tmp), arguments.size());
                return false;
			}
		}

        auto frame = ExecutionFrame::create(it->second);
        frame->initialize_execute_function(return_value, vm, it->second, self_value, arguments);
        push_new_frame(std::move(frame));
        exit = true;
        return true;
    }

    void Executor::prepare_execute_call(OwcaValue &return_value, OwcaValue func, std::span<OwcaValue> arguments)
    {
		return func.visit(
			[&](OwcaIterator oi) -> void {
				if (!arguments.empty()) {
                    prepare_throw_not_callable_wrong_number_of_params("generator", (unsigned int)arguments.size());
                }
                else {
                    prepare_resume_generator(return_value, oi);
                }
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
				prepare_throw_not_callable(func.type());
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




    void Executor::prepare_throw_cpp_exception(std::string_view msg)
    {
        OwcaValue temp_arg = vm->create_string_from_view(msg);
        prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
    }
	void Executor::prepare_throw_too_many_elements(size_t expected)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("too many values to unpack (expected {})", expected));
        prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}
	void Executor::prepare_throw_not_enough_elements(size_t expected, size_t got)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("not enough values to unpack (expected {}, got {})", expected, got));
        prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}
	void Executor::prepare_throw_dictionary_changed(bool is_dict)
	{
        OwcaValue temp_arg = is_dict ? vm->create_string_from_view("dictionary changed during iteration") : vm->create_string_from_view("set changed during iteration");
        prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}
	void Executor::prepare_throw_not_implemented(std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
        prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}
	void Executor::prepare_throw_range_step_is_zero()
	{
        OwcaValue temp_arg = vm->create_string_from_view("range step is zero");
        prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}
	void Executor::prepare_throw_division_by_zero()
	{
        OwcaValue temp_arg = vm->create_string_from_view("division by zero");
        prepare_allocate_user_class(temporary_exception_return_value, vm->c_math_exception, std::span{ &temp_arg, 1 }, true);
	}
	void Executor::prepare_throw_mod_division_by_zero()
	{
        OwcaValue temp_arg = vm->create_string_from_view("modulo by zero");
        prepare_allocate_user_class(temporary_exception_return_value, vm->c_math_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_cant_convert_to_float_message(std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
        prepare_allocate_user_class(temporary_exception_return_value, vm->c_math_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_cant_convert_to_float(std::string_view type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("can't convert value of type `{}` to floating point", type));
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_math_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_cant_convert_to_integer(Number val)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("can't convert {} to integer", val));
        prepare_allocate_user_class(temporary_exception_return_value, vm->c_math_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_cant_convert_to_integer(std::string_view type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("can't convert {} to integer", type));
        prepare_allocate_user_class(temporary_exception_return_value, vm->c_math_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_not_a_number(std::string_view type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} is not a number", type));
        prepare_allocate_user_class(temporary_exception_return_value, vm->c_math_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_overflow(std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
        prepare_allocate_user_class(temporary_exception_return_value, vm->c_math_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_range_step_must_be_one_in_left_side_of_write_assign()
	{
        OwcaValue temp_arg = vm->create_string_from_view("step of a range must be 1 in left side of write assignment");
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_cant_compare(CompareKind kind, std::string_view left, std::string_view right)
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
        prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_string_too_large(size_t size)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("string is too large ({} bytes)", size));
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}
	void Executor::prepare_throw_index_out_of_range(std::string msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_value_not_indexable(std::string_view type, std::string_view key_type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} is not indexable with key {}", type, key_type));
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_missing_member(std::string_view type, std::string_view ident)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} doesn't have a member {}", type, ident));
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_cant_call(std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_not_callable(std::string_view type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} is not callable", type));
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}
	
	void Executor::prepare_throw_not_callable_wrong_number_of_params(std::string_view type, unsigned int params)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} is not callable - wrong number of parameters ({})", type, params));
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_wrong_type(std::string_view type, std::string_view expected)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("wrong type {} - expected {}", type, expected));
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_wrong_type(std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_unsupported_operation_2(std::string_view oper, std::string_view left, std::string_view right)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("can't execute {} {} {}", left, oper, right));
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_invalid_operand_for_mul_string(std::string_view val)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("can't multiply string by {}", val));
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_missing_key(std::string_view key)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("missing key {}", key));
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_not_hashable(std::string_view type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} is not hashable", type));
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_value_cant_have_fields(std::string_view type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} can't have fields", type));
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_missing_native(std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_not_iterable(std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_readonly(std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_cant_return_value_from_generator()
	{
        OwcaValue temp_arg = vm->create_string_from_view("can't return value from generator");
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}

	void Executor::prepare_throw_container_is_empty()
	{
        OwcaValue temp_arg = vm->create_string_from_view("container is empty");
		prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
	}





	void Executor::throw_too_many_elements(size_t expected)
	{
        prepare_throw_too_many_elements(expected);
        run_and_throw();
	}
	void Executor::throw_not_enough_elements(size_t expected, size_t got)
	{
        prepare_throw_not_enough_elements(expected, got);
        run_and_throw();
	}
	void Executor::throw_dictionary_changed(bool is_dict)
	{
        prepare_throw_dictionary_changed(is_dict);
        run_and_throw();
	}
	void Executor::throw_not_implemented(std::string_view msg)
	{
        prepare_throw_not_implemented(msg);
        run_and_throw();
	}
	void Executor::throw_range_step_is_zero()
	{
        prepare_throw_range_step_is_zero();
        run_and_throw();
	}
	void Executor::throw_division_by_zero()
	{
        prepare_throw_division_by_zero();
        run_and_throw();
	}
	void Executor::throw_mod_division_by_zero()
	{
        prepare_throw_mod_division_by_zero();
        run_and_throw();
	}

	void Executor::throw_cant_convert_to_float_message(std::string_view msg)
	{
        prepare_throw_cant_convert_to_float_message(msg);
        run_and_throw();
	}

	void Executor::throw_cant_convert_to_float(std::string_view type)
	{
        prepare_throw_cant_convert_to_float(type);
        run_and_throw();
	}

	void Executor::throw_cant_convert_to_integer(Number val)
	{
        prepare_throw_cant_convert_to_integer(val);
        run_and_throw();
	}

	void Executor::throw_cant_convert_to_integer(std::string_view type)
	{
        prepare_throw_cant_convert_to_integer(type);
        run_and_throw();
	}

	void Executor::throw_not_a_number(std::string_view type)
	{
        prepare_throw_not_a_number(type);
        run_and_throw();
	}

	void Executor::throw_overflow(std::string_view msg)
	{
        prepare_throw_overflow(msg);
        run_and_throw();
	}

	void Executor::throw_range_step_must_be_one_in_left_side_of_write_assign()
	{
        prepare_throw_range_step_must_be_one_in_left_side_of_write_assign();
        run_and_throw();
	}

	void Executor::throw_cant_compare(CompareKind kind, std::string_view left, std::string_view right)
	{
        prepare_throw_cant_compare(kind, left, right);
        run_and_throw();
	}

	void Executor::throw_string_too_large(size_t size)
	{
        prepare_throw_string_too_large(size);
        run_and_throw();
	}
	void Executor::throw_index_out_of_range(std::string msg)
	{
        prepare_throw_index_out_of_range(msg);
        run_and_throw();
	}

	void Executor::throw_value_not_indexable(std::string_view type, std::string_view key_type)
	{
        prepare_throw_value_not_indexable(type, key_type);
        run_and_throw();
	}

	void Executor::throw_missing_member(std::string_view type, std::string_view ident)
	{
        prepare_throw_missing_member(type, ident);
        run_and_throw();
	}

	void Executor::throw_cant_call(std::string_view msg)
	{
        prepare_throw_cant_call(msg);
        run_and_throw();
	}

	void Executor::throw_not_callable(std::string_view type)
	{
        prepare_throw_not_callable(type);
        run_and_throw();
	}
	
	void Executor::throw_not_callable_wrong_number_of_params(std::string_view type, unsigned int params)
	{
        prepare_throw_not_callable_wrong_number_of_params(type, params);
        run_and_throw();
	}

	void Executor::throw_wrong_type(std::string_view type, std::string_view expected)
	{
        prepare_throw_wrong_type(type, expected);
        run_and_throw();
	}

	void Executor::throw_wrong_type(std::string_view msg)
	{
        prepare_throw_wrong_type(msg);
        run_and_throw();
	}

	void Executor::throw_unsupported_operation_2(std::string_view oper, std::string_view left, std::string_view right)
	{
        prepare_throw_unsupported_operation_2(oper, left, right);
        run_and_throw();
	}

	void Executor::throw_invalid_operand_for_mul_string(std::string_view val)
	{
        prepare_throw_invalid_operand_for_mul_string(val);
        run_and_throw();
	}

	void Executor::throw_missing_key(std::string_view key)
	{
        prepare_throw_missing_key(key);
        run_and_throw();
	}

	void Executor::throw_not_hashable(std::string_view type)
	{
        prepare_throw_not_hashable(type);
        run_and_throw();
	}

	void Executor::throw_value_cant_have_fields(std::string_view type)
	{
        prepare_throw_value_cant_have_fields(type);
        run_and_throw();
	}

	void Executor::throw_missing_native(std::string_view msg)
	{
        prepare_throw_missing_native(msg);
        run_and_throw();
	}

	void Executor::throw_not_iterable(std::string_view msg)
	{
        prepare_throw_not_iterable(msg);
        run_and_throw();
	}

	void Executor::throw_readonly(std::string_view msg)
	{
        prepare_throw_readonly(msg);
        run_and_throw();
	}

	void Executor::throw_cant_return_value_from_generator()
	{
        prepare_throw_cant_return_value_from_generator();
        run_and_throw();
	}

	void Executor::throw_container_is_empty()
	{
        prepare_throw_container_is_empty();
        run_and_throw();
	}
}