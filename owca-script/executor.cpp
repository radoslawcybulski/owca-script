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

#ifdef DEBUG
#define OWCA_SCRIPT_EXEC_LOG
#endif

//#define MEASURE

namespace OwcaScript::Internal {
    Executor::Executor(VM *vm) : vm(vm), values_vector(1024 * 1024), states_vector(1024 * 16), temporary_ptr_current_top(values_vector.data()), states_ptr_current_top(states_vector.data()) {
    }

//     ExecutionFrame &Executor::currently_executing_frame() {
//         return *vm->current_frame();
//     }
// //     void Executor::push_new_frame(std::unique_ptr<ExecutionFrame> frame) {
// // #ifdef OWCA_SCRIPT_EXEC_LOG
// //         std::cout << "Pushing frame " << (void*)frame.get() << "\n";
// // #endif
// //         vm->stacktrace.push_back(std::move(frame));
// //     }
//     void Executor::pop_frame() {
//         exit = true;
//         auto frame = vm->current_frame();
//         assert(frame);
//         vm->pop_frame(frame);
// #ifdef OWCA_SCRIPT_EXEC_LOG
//         std::cout << "Popping frame " << (void*)frame << std::endl;
// #endif
//         if (frame->constructor_move_self_to_return_value) {
//             assert(frame->return_value);
//             *frame->return_value = std::move(frame->values_[0]);
//         }
//         if (frame->is_iterator) {
//             assert(frame->return_value);
//             assert(frame->iterator_object);
//             auto iter = frame->iterator_object->internal_value();

//             if (frame->return_value->kind() == OwcaValueKind::Completed) {
//                 iter->generator = {};
//                 iter->completed = true;
//                 iter->frame.reset();
//             }
//             exit = true;
//             return;
//         }
//         if (frame->dict_output) {
//             frame->runtime_function->visit(
//                 [&](RuntimeFunction::ScriptFunction& sf) -> void {
//                     for (auto i = 0u; i < sf.identifier_names.size(); ++i) {
//                         auto key = vm->create_string_from_view(sf.identifier_names[i]);
//                         (*frame->dict_output)[key] = frame->get_identifier(i);
//                     }
//                 },
//                 [&](RuntimeFunction::NativeFunction&) -> void {
//                     assert(false);
//                 },
//                 [&](RuntimeFunction::NativeGenerator&) -> void {
//                     assert(false);
//                 }
//             );
//         }
// #ifdef OWCA_SCRIPT_EXEC_LOG
//         std::cout << "Deleting frame " << (void*)frame << std::endl;
// #endif
//         vm->deallocate_stack_frame(frame);
//     }

#define POP_STATE() --states_ptr;
#define STATE(tp) std::get<tp>(*(states_ptr.states_type_ptr - 1))
#define PUSH_STATE(tp) do { *states_ptr.states_type_ptr = (tp); ++states_ptr; } while(0)
#define TRY_STATE(tp) (std::get_if<tp>(states_ptr.states_type_ptr - 1))
#define HAS_STATE() (!states_ptr.empty())
#define PEEK_VALUES(offset, count) std::span<OwcaValue>{ temporary_ptr[{ (offset), (count) }] }
#define PEEK_VALUE(offset) temporary_ptr[(offset)]
#define POP_VALUES(count) do { temporary_ptr = temporary_ptr - (count); } while(0)
#define PUSH_VALUE(val) do { temporary_ptr[0] = (val); ++temporary_ptr; } while(0)
#define LOCAL_VAR(index) (locals_ptr[index])

    void Executor::process_thrown_exception(ExecuteBufferReader::Position *code_pos, StatesTypePtr &states_ptr, OwcaException exception)
    {
        while(HAS_STATE()) {
            if (auto state = TRY_STATE(TryState)) {
                *code_pos = state->catches_pos;
                exception_being_thrown = exception;
#ifdef OWCA_SCRIPT_EXEC_LOG
                std::cout << __FILE__ << ":" << __LINE__ << ": setting code position (" << (void*)code_pos << ") to " << (void*)code_pos->value() << std::endl;
#endif
                return;
            }
            POP_STATE();
        }
        throw exception;
    }
    // size_t Executor::currently_executing_frame_index() const {
    //     return vm->frame_count;
    // }
    // bool Executor::completed() const {
    //     return currently_executing_frame_index() <= stack_top_level_index;
    // }
    
	std::tuple<Number, Number, Number> Executor::parse_key(VM *vm, OwcaValue v, OwcaValue key, Number size) {
		return key.visit(
			[&](Number o) -> std::tuple<Number, Number, Number> {
				auto v = key.as_int(vm);
				if (v < 0) v += size;
				if (v < 0 || v >= size) {
					throw_index_out_of_range(std::format("index value {} is out of range for object of size {}", key, size));
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
					return { lower, upper, step };
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
					return { lower, upper, step };
				}
			},
			[&](const auto&) -> std::tuple<Number, Number, Number> {
				throw_value_not_indexable(v.type(), key.type());
			}
		);
	}
	size_t Executor::verify_key(VM *vm, Number v, size_t size, OwcaValue orig_key, std::string_view name) {
		if (v < 0 || v >= (Number)size) {
			throw_index_out_of_range(std::format("index value {} is out of range for {} of size {}", orig_key, name, size));
		}
		auto v2 = (size_t)v;
		if (v2 != v) {
			throw_index_out_of_range(std::format("index value {} is out of range for {} of size {} - size_t overflows", orig_key, name, size));
		}
		return v2;
	}
	std::pair<size_t, size_t> Executor::verify_key(VM *vm, OwcaRange k, size_t size, OwcaValue orig_key, std::string_view name) {
		auto v1 = k.lower();
		auto v2 = k.upper();
		if (v2 <= v1)
			return std::pair<size_t, size_t>{ 0, 0 };
		if (v1 < 0) v1 = 0;
		if (v2 > (Number)size) v2 = size;
		size_t v3 = (size_t)v1, v4 = (size_t)v2;
		if (v3 != v1 || v2 != v4) {
			throw_index_out_of_range(std::format("index values {} is out of range for array of size {} - size_t overflows", orig_key, size));
		}
		return std::pair<size_t, size_t>{ v3, v4 };
	}

    ExecuteBufferReader::Position Executor::run_impl_opcodes_execute_compare(TemporariesPtr temporary_ptr, StartOfCode start_code, ExecuteBufferReader::Position code_pos, CompareKind kind, const std::unordered_map<const unsigned char *, Internal::DataKind> &data_kinds) {
        auto jump_dest = ExecuteBufferReader::decode_jump(start_code, code_pos, data_kinds);
        const auto last = ExecuteBufferReader::decode<bool>(start_code, code_pos, data_kinds);
        auto &left = PEEK_VALUE(2);
        auto right = PEEK_VALUE(1);
        auto res = execute_compare(vm, kind, left, right);
        switch(res) {
        case CompareResult::True:
            left = last ? OwcaValue{ true } : right;
            return code_pos;
        case CompareResult::False:
            left = false;
            code_pos = ExecuteBufferReader::Position{ jump_dest };
            return code_pos;
        case CompareResult::NotExecuted:
            throw_cant_compare(kind, left.type(), right.type());
            assert(false);
        }
        assert(false);
        return code_pos;
    }

    OwcaValue Executor::set_identifier_function(OwcaValue target, OwcaValue value) {
        assert(value.kind() == OwcaValueKind::Functions);
        auto fnc = value.as_functions(vm);
        if (target.kind() == OwcaValueKind::Functions) {
            auto dst_fnc = target.as_functions(vm);
            for(auto i = 0u; i < fnc.internal_value()->functions.size(); ++i) {
                if (fnc.internal_value()->functions[i]) {
                    dst_fnc.internal_value()->functions[i] = fnc.internal_value()->functions[i];
                }
            }
            return target;
        }
        return value;
    }
    
    OwcaValue Executor::index_write(OwcaValue self, OwcaValue key, OwcaValue value) {
        return self.visit(
            [&](const OwcaMap& o) -> OwcaValue {
                o.internal_value()->dict.write(key, value);
                return value;
            },
            [&](OwcaArray o) -> OwcaValue {
                const auto size = (Number)o.internal_value()->values.size();
                if (size != o.internal_value()->values.size()) {
                    throw_index_out_of_range(std::format("array size {} is too large for Number size to properly handle indexing", o.internal_value()->values.size()));
                }
                auto [ lower, upper, step ] = parse_key(vm, key, key, size);
                if (step == 0) {
                    o[lower] = value;
                    return value;
                }
                if (step != 1) {
                    throw_range_step_must_be_one_in_left_side_of_write_assign();
                }

                auto iter = vm->create_iterator(value);
                auto write = lower;
                auto &values = o.internal_value()->values;
                std::vector<OwcaValue> temp;
                while(auto val = iter.next()) {
                    assert(write <= upper);
                    if (write < upper) {
                        values[write++] = *val;
                    }
                    else {
                        assert(write == upper);
                        temp.push_back(*val);
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
                throw_readonly("tuple is readonly");
            },
            [&](const auto&) -> OwcaValue {
                throw_value_not_indexable(self.type());
            }
        );
    }
    OwcaValue Executor::index_read(OwcaValue self, OwcaValue key) {
        return self.visit(
            [&](const OwcaString& o) -> OwcaValue {
                const auto size = (Number)o.internal_value()->size();
                if (size != o.internal_value()->size()) {
                    throw_index_out_of_range(std::format("string size {} is too large for Number size to properly handle indexing", o.internal_value()->size()));
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
                    throw_index_out_of_range(std::format("array size {} is too large for Number size to properly handle indexing", o.internal_value()->values.size()));
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
                    throw_index_out_of_range(std::format("tuple size {} is too large for Number size to properly handle indexing", o.internal_value()->values.size()));
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
                throw_value_not_indexable(self.type());
            }
        );
    }

    OwcaValue Executor::create_function(StartOfCode start_code, ExecuteBufferReader::Position &code_pos, LocalsPtr locals_ptr, StatesTypePtr states_ptr, const std::unordered_map<const unsigned char *, Internal::DataKind> &data_kinds)
    {
        auto &code_object = stacktrace.back().runtime_function->code;
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
            if (HAS_STATE()) {
                if (auto state = TRY_STATE(ClassState)) {
                    class_ = ClassToken{ state->cls };
                }
            }
            if (is_generator) {
                fnc = vm->allocate<RuntimeFunction>(0, code_object, name, full_name, RuntimeFunction::NativeGenerator{});
                auto &ng = std::get<RuntimeFunction::NativeGenerator>(fnc->data);
                ng.parameter_names = std::move(identifier_names);
                ng.line = code_object.get_line_by_position(code_pos).line;
                if (native_provider) {
                    if (auto impl = native_provider->native_generator(full_name, class_, FunctionToken{ fnc }, ng.parameter_names)) {
                        ng.generator = std::move(*impl);
                    }
                }
                if (!ng.generator) {
                    throw_missing_native(std::format("missing native generator {}", full_name));
                }
            }
            else {
                fnc = vm->allocate<RuntimeFunction>(0, code_object, name, full_name, RuntimeFunction::NativeFunction{});
                auto &nf = std::get<RuntimeFunction::NativeFunction>(fnc->data);
                nf.parameter_names = std::move(identifier_names);
                nf.line = code_object.get_line_by_position(code_pos).line;
                if (native_provider) {
                    if (auto impl = native_provider->native_function(full_name, class_, FunctionToken{ fnc }, nf.parameter_names)) {
                        nf.function = std::move(*impl);
                    }
                }
                if (!nf.function) {
                    throw_missing_native(std::format("missing native function {}", full_name));
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
            auto z = ExecuteBufferReader::decode_jump(start_code, code_pos, data_kinds);
            sf.entry_point = code_pos;
            code_pos = z;

            sf.values_from_parents.reserve(sf.copy_from_parents.size());
            for(auto c : sf.copy_from_parents) {
                sf.values_from_parents.push_back(LOCAL_VAR(c.index_in_parent));
            }
        }
        fnc->param_count = param_count;
        fnc->max_temporaries = temporaries_count;
        fnc->max_states = state_count;
        fnc->max_values = value_count;
        fnc->is_method = is_method;
        auto rfs = VM::get(vm).allocate<RuntimeFunctions>(0, name, full_name);
        rfs->functions[fnc->param_count] = fnc;
        return OwcaFunctions{ rfs };
    }

    std::tuple<OwcaValue, Executor::TemporariesPtr, Executor::StatesTypePtr, ExecuteBufferReader::Position> Executor::run_opcodes(const LocalsPtr locals_ptr, TemporariesPtr temporary_ptr, StatesTypePtr states_ptr, StartOfCode start_code, ExecuteBufferReader::Position code_pos)
    {
        assert(!stacktrace.empty());
        const size_t stacktrace_index = stacktrace.size() - 1;
#ifdef OWCA_SCRIPT_EXEC_LOG
        auto &code_object = stacktrace.back().runtime_function->code;
        auto temporary_ptr_start = temporary_ptr;
        auto states_ptr_start = states_ptr;
        while(!states_ptr_start.empty()) {
            --states_ptr_start;
        }
        const auto &data_kinds = code_object.data_kinds();
#else
        const std::unordered_map<const unsigned char *, Internal::DataKind> &data_kinds = {};
#endif        
#ifdef MEASURE        
        std::array<std::uint64_t, (size_t)Internal::ExecuteOp::_Count> times;
        std::array<std::uint64_t, (size_t)Internal::ExecuteOp::_Count> counts;

        for(auto &t : times) t = 0;
        for(auto &c : counts) c = 0;
#endif

restart:
        try {
            for(;;) {
#ifdef MEASURE            
                auto start = std::chrono::high_resolution_clock::now();
#endif
                //std::cout << "Running opcode at position " << reader.position() << std::endl;
#ifdef OWCA_SCRIPT_EXEC_LOG
                auto line = code_object.get_line_by_position(code_pos);
#endif
                auto opcode = ExecuteBufferReader::decode<ExecuteBufferReader::Op>(start_code, code_pos, data_kinds);
                // static std::chrono::high_resolution_clock::time_point last_time = std::chrono::high_resolution_clock::now();
                // auto now = std::chrono::high_resolution_clock::now();
                // auto df = now - last_time;
                // last_time = now;
                // std::cout << std::setw(10) << (std::chrono::duration_cast<std::chrono::nanoseconds>(df).count()) << " ns ";
#ifdef OWCA_SCRIPT_EXEC_LOG
                std::string states_debug;
                for(auto s = states_ptr_start.states_type_ptr; s < states_ptr.states_type_ptr; ++s) {
                    visit_variant(*s,
                        [&](const EmptyState&) { states_debug += "E"; },
                        [&](const ClassState& c) { states_debug += "S"; },
                        [&](const ForState& c) { states_debug += "F"; },
                        [&](const WhileState& c) { states_debug += "W"; },
                        [&](const TryState& t) { states_debug += "T"; },
                        [&](const CatchState& t) { states_debug += "C"; },
                        [&](const WithState& t) { states_debug += "H"; }
                    );
                }
                std::cout << "Running opcode at line " << std::setw(4) << line.line << " position " << std::setw(5) << (code_pos.value() - stacktrace.back().runtime_function->code.code().data() - 1) << " temporaries " << std::setw(2) << (temporary_ptr.temporaries_ptr - temporary_ptr_start.temporaries_ptr) << 
                    " states " << std::setw(6) << states_debug << " opcode " << std::setw(30) << to_string(opcode);
                if (exception_being_thrown) std::cout << " (exception in progress)";
                if (exception_being_handled) std::cout << " (exception being handled)";
                std::cout << std::endl;
#endif

                //last_time = std::chrono::high_resolution_clock::now();
                switch(opcode) {
                case ExecuteBufferReader::Op::_Count:
                    assert(false);
                    break;
                case ExecuteBufferReader::Op::ClassInit: {
                    auto &code_object = stacktrace.back().runtime_function->code;
                    auto line = code_object.get_line_by_position(code_pos - 1);
                    auto name = ExecuteBufferReader::decode<std::string_view>(start_code, code_pos, data_kinds);
                    auto full_name = ExecuteBufferReader::decode<std::string_view>(start_code, code_pos, data_kinds);
                    auto cls = vm->allocate<Class>(0, line, name, full_name, code_object);
                    PUSH_STATE(ClassState{});
                    STATE(ClassState).cls = cls;
                    break; }
                case ExecuteBufferReader::Op::ClassCreate: {
                    auto cls = STATE(ClassState).cls;
                    POP_STATE();

                    auto native = ExecuteBufferReader::decode<bool>(start_code, code_pos, data_kinds);
                    auto base_class_count = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                    auto member_count = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                    auto all_variable_names = ExecuteBufferReader::decode<bool>(start_code, code_pos, data_kinds);
                    auto variable_name_count = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);

                    auto base_classes = PEEK_VALUES(base_class_count, base_class_count);
                    auto members = PEEK_VALUES(member_count + base_class_count, member_count);
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
                            throw_missing_native(std::format("missing native class {}", cls->full_name));
                        }
                    }
                    POP_VALUES(base_class_count + member_count);
                    PUSH_VALUE(OwcaClass{ cls });
                    break; }
                case ExecuteBufferReader::Op::ExprPopAndIgnore: {
                    POP_VALUES(1);
                    break; }
                case ExecuteBufferReader::Op::ExprCompareEq: {
                    code_pos = run_impl_opcodes_execute_compare(temporary_ptr, start_code, code_pos, CompareKind::Eq, data_kinds);
                    POP_VALUES(1);
                    break; }
                case ExecuteBufferReader::Op::ExprCompareNotEq: {
                    code_pos = run_impl_opcodes_execute_compare(temporary_ptr, start_code, code_pos, CompareKind::NotEq, data_kinds);
                    POP_VALUES(1);
                    break; }
                case ExecuteBufferReader::Op::ExprCompareLessEq: {
                    code_pos = run_impl_opcodes_execute_compare(temporary_ptr, start_code, code_pos, CompareKind::LessEq, data_kinds);
                    POP_VALUES(1);
                    break; }
                case ExecuteBufferReader::Op::ExprCompareMoreEq: {
                    code_pos = run_impl_opcodes_execute_compare(temporary_ptr, start_code, code_pos, CompareKind::MoreEq, data_kinds);
                    POP_VALUES(1);
                    break; }
                case ExecuteBufferReader::Op::ExprCompareLess: {
                    code_pos = run_impl_opcodes_execute_compare(temporary_ptr, start_code, code_pos, CompareKind::Less, data_kinds);
                    POP_VALUES(1);
                    break; }
                case ExecuteBufferReader::Op::ExprCompareMore: {
                    code_pos = run_impl_opcodes_execute_compare(temporary_ptr, start_code, code_pos, CompareKind::More, data_kinds);
                    POP_VALUES(1);
                    break; }
                case ExecuteBufferReader::Op::ExprCompareIs: {
                    code_pos = run_impl_opcodes_execute_compare(temporary_ptr, start_code, code_pos, CompareKind::Is, data_kinds);
                    POP_VALUES(1);
                    break; }
                case ExecuteBufferReader::Op::ExprConstantEmpty: {
                    PUSH_VALUE(OwcaEmpty{});
                    break; }
                case ExecuteBufferReader::Op::ExprConstantBool: {
                    auto value = ExecuteBufferReader::decode<bool>(start_code, code_pos, data_kinds);
                    PUSH_VALUE(value);
                    break; }
                case ExecuteBufferReader::Op::ExprConstantFloat: {
                    auto value = ExecuteBufferReader::decode<Number>(start_code, code_pos, data_kinds);
                    PUSH_VALUE(value);
                    break; }
                case ExecuteBufferReader::Op::ExprConstantString: {
                    auto value = ExecuteBufferReader::decode<std::string_view>(start_code, code_pos, data_kinds);
                    PUSH_VALUE(vm->create_string_from_view(value));
                    break; }
                case ExecuteBufferReader::Op::ExprConstantStringInterpolated: {
                    auto strings = ExecuteBufferReader::decode<std::string_view>(start_code, code_pos, data_kinds);
                    auto expr_count = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                    size_t size = strings.size();
                    auto values = PEEK_VALUES(expr_count, expr_count);
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
                    POP_VALUES(expr_count);
                    PUSH_VALUE(OwcaString{ new_str });
                    break; }
                case ExecuteBufferReader::Op::ExprIdentifierRead: {
                    auto index = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                    PUSH_VALUE(LOCAL_VAR(index));
                    break; }
                case ExecuteBufferReader::Op::ExprIdentifierWrite: {
                    auto index = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                    LOCAL_VAR(index) = PEEK_VALUE(1);
                    break; }
                case ExecuteBufferReader::Op::ExprIdentifierFunctionWrite: {
                    auto index = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                    auto &val = PEEK_VALUE(1);
                    auto &tgt = LOCAL_VAR(index);
                    tgt = set_identifier_function(tgt, val);
                    val = tgt;
                    break; }
                case ExecuteBufferReader::Op::ExprMemberRead: {
                    auto self = PEEK_VALUE(1);
                    auto member = ExecuteBufferReader::decode<std::string_view>(start_code, code_pos, data_kinds);
                    PEEK_VALUE(1) = vm->member(self, member);
                    break; }
                case ExecuteBufferReader::Op::ExprMemberWrite: {
                    auto val_to_write = PEEK_VALUE(1);
                    auto self = PEEK_VALUE(2);
                    auto member = ExecuteBufferReader::decode<std::string_view>(start_code, code_pos, data_kinds);
                    vm->member(self, member, val_to_write);
                    PEEK_VALUE(2) = val_to_write;
                    POP_VALUES(1);
                    break; }
                case ExecuteBufferReader::Op::ExprOper1BinNeg: {
                    auto &left = PEEK_VALUE(1);
                    left = -(std::int64_t)left.as_float(vm);
                    break; }
                case ExecuteBufferReader::Op::ExprOper1LogNot: {
                    auto &left = PEEK_VALUE(1);
                    left = !left.is_true();
                    break; }
                case ExecuteBufferReader::Op::ExprOper1Negate: {
                    auto &left = PEEK_VALUE(1);
                    left = -left.as_float(vm);
                    break; }
                case ExecuteBufferReader::Op::ExprRetTrueAndJumpIfTrue: {
                    auto jump_dest = ExecuteBufferReader::decode_jump(start_code, code_pos, data_kinds);
                    if (PEEK_VALUE(1).is_true()) {
                        code_pos = jump_dest;
                    }
                    else {
                        POP_VALUES(1);
                    }
                    break; }
                case ExecuteBufferReader::Op::ExprRetFalseAndJumpIfFalse: {
                    auto jump_dest = ExecuteBufferReader::decode_jump(start_code, code_pos, data_kinds);
                    if (!PEEK_VALUE(1).is_true()) {
                        code_pos = jump_dest;
                    }
                    else {
                        POP_VALUES(1);
                    }
                    break; }
                case ExecuteBufferReader::Op::ExprToString: {
                    auto v = PEEK_VALUE(1);
                    if (v.kind() == OwcaValueKind::String) {
                        break;
                    }
                    PEEK_VALUE(1) = vm->create_string_from_view(v.to_string());
                    break; }
                case ExecuteBufferReader::Op::ExprToIterator: {
                    auto &val = PEEK_VALUE(1);
                    if (val.kind() != OwcaValueKind::Iterator) {
                        auto func = vm->try_member(val, "__iter__");
                        if (!func) {
                            throw_not_iterable(val.type());
                        }
                        val = *func;
                        val = execute_call_from_values(temporary_ptr, states_ptr, 1);
                    }
                    break; }
                case ExecuteBufferReader::Op::ExprOper2BinOr: {
                    run_impl_opcodes_execute_expr_oper2<TagBinOr>(temporary_ptr);
                    break; }
                case ExecuteBufferReader::Op::ExprOper2BinAnd: {
                    run_impl_opcodes_execute_expr_oper2<TagBinAnd>(temporary_ptr);
                    break; }
                case ExecuteBufferReader::Op::ExprOper2BinXor: {
                    run_impl_opcodes_execute_expr_oper2<TagBinXor>(temporary_ptr);
                    break; }
                case ExecuteBufferReader::Op::ExprOper2BinLShift: {
                    run_impl_opcodes_execute_expr_oper2<TagBinLShift>(temporary_ptr);
                    break; }
                case ExecuteBufferReader::Op::ExprOper2BinRShift: {
                    run_impl_opcodes_execute_expr_oper2<TagBinRShift>(temporary_ptr);
                    break; }
                case ExecuteBufferReader::Op::ExprOper2Add: {
                    run_impl_opcodes_execute_expr_oper2<TagAdd>(temporary_ptr);
                    break; }
                case ExecuteBufferReader::Op::ExprOper2Sub: {
                    run_impl_opcodes_execute_expr_oper2<TagSub>(temporary_ptr);
                    break; }
                case ExecuteBufferReader::Op::ExprOper2Mul: {
                    run_impl_opcodes_execute_expr_oper2<TagMul>(temporary_ptr);
                    break; }
                case ExecuteBufferReader::Op::ExprOper2Div: {
                    run_impl_opcodes_execute_expr_oper2<TagDiv>(temporary_ptr);
                    break; }
                case ExecuteBufferReader::Op::ExprOper2Mod: {
                    run_impl_opcodes_execute_expr_oper2<TagMod>(temporary_ptr);
                    break; }
                case ExecuteBufferReader::Op::ExprOper2MakeRange: {
                    auto mode = ExecuteBufferReader::decode<std::uint8_t>(start_code, code_pos, data_kinds);
                    Number first, second, third;
                    if (mode & 4) {
                        third = PEEK_VALUE(1).as_float(vm);
                        POP_VALUES(1);
                    }
                    else {
                        third = 1;
                    }
                    if (mode & 2) {
                        second = PEEK_VALUE(1).as_float(vm);
                        POP_VALUES(1);
                    }
                    else {
                        second = std::numeric_limits<Number>::max();
                    }
                    if (mode & 1) {   
                        first = PEEK_VALUE(1).as_float(vm);
                        POP_VALUES(1);
                    }
                    else {
                        first = 0;
                    }
                    if (third == 0) {
                        throw_range_step_is_zero();
                    }
                    auto ret = vm->allocate<Range>(0);
                    ret->from = first;
                    ret->to = second;
                    ret->step = third;
                    PUSH_VALUE(OwcaRange{ ret });
                    break; }
                case ExecuteBufferReader::Op::ExprOper2IndexRead: {
                    auto key = PEEK_VALUE(1);
                    auto self = PEEK_VALUE(2);
                    auto &ret = PEEK_VALUE(2);
                    POP_VALUES(1);
                    ret = index_read(self, key);
                    break; }
                case ExecuteBufferReader::Op::ExprOper2IndexWrite: {
                    auto value = PEEK_VALUE(1);
                    auto key = PEEK_VALUE(2);
                    auto &self = PEEK_VALUE(3);
                    POP_VALUES(2);
                    self = index_write(self, key, value);
                    break; }
                case ExecuteBufferReader::Op::ExprOperXCall: {
                    auto size = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                    auto val = execute_call_from_values(temporary_ptr, states_ptr, size);
                    POP_VALUES(size);
                    PUSH_VALUE(val);
                    break; }
                case ExecuteBufferReader::Op::ExprOperXCreateArray: {
                    auto size = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                    auto args = PEEK_VALUES(size, size);
                    auto arguments = std::deque<OwcaValue>{ args.begin(), args.end() };
                    POP_VALUES(size);
                    PUSH_VALUE(vm->create_array(std::move(arguments)));
                    break; }
                case ExecuteBufferReader::Op::ExprOperXCreateTuple: {
                    auto size = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                    auto args = PEEK_VALUES(size, size);
                    auto arguments = std::vector<OwcaValue>{ args.begin(), args.end() };
                    POP_VALUES(size);
                    PUSH_VALUE(vm->create_tuple(std::move(arguments)));
                    break; }
                case ExecuteBufferReader::Op::ExprOperXCreateSet: {
                    auto size = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                    auto args = PEEK_VALUES(size, size);
                    POP_VALUES(size);
                    PUSH_VALUE(vm->create_set(args));
                    break; }
                case ExecuteBufferReader::Op::ExprOperXCreateMap: {
                    auto size = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                    auto args = PEEK_VALUES(size, size);
                    POP_VALUES(size);
                    PUSH_VALUE(vm->create_map(args));
                    break; }
                case ExecuteBufferReader::Op::ForInit: {
                    auto iterator = PEEK_VALUE(1).as_iterator(vm);
                    POP_VALUES(1);
                    PUSH_STATE(ForState{ iterator });
                    auto &state = STATE(ForState);
                    state.end_position = ExecuteBufferReader::decode_jump(start_code, code_pos, data_kinds);
                    state.loop_index = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                    state.loop_control_depth = ExecuteBufferReader::decode<std::uint8_t>(start_code, code_pos, data_kinds);
                    state.continue_position = code_pos;
                    break; }
                case ExecuteBufferReader::Op::ForCondition: {
                    auto &state = STATE(ForState);
                    if (state.iterator.completed()) [[unlikely]] {
                        code_pos = ExecuteBufferReader::Position{ state.end_position };
                        break;
                    }
                    state.index++;
                    if (state.loop_index != std::numeric_limits<std::uint32_t>::max()) {
                        LOCAL_VAR(state.loop_index) = state.index;
                    }
                    auto val = continue_iterator(state.iterator);
                    if (!val) [[unlikely]] {
                        code_pos = ExecuteBufferReader::Position{ state.end_position };
                        break;
                    }
                    PUSH_VALUE(*val);
                    break; }
                case ExecuteBufferReader::Op::ForNext: {
                    auto val = PEEK_VALUE(1);
                    if (val.kind() == OwcaValueKind::Completed) {
                        auto &state = STATE(ForState);
                        code_pos = ExecuteBufferReader::Position{ state.end_position };
                        POP_VALUES(1);
                    }
                    break; }
                case ExecuteBufferReader::Op::ForCompleted: {
                    auto &state = STATE(ForState);
                    POP_STATE();
                    break; }
                case ExecuteBufferReader::Op::Function: {
                    PUSH_VALUE(create_function(start_code, code_pos, locals_ptr, states_ptr, data_kinds));
                    break; }
                case ExecuteBufferReader::Op::If: {
                    auto val = PEEK_VALUE(1).is_true();
                    POP_VALUES(1);
                    auto else_position = ExecuteBufferReader::decode_jump(start_code, code_pos, data_kinds);
                    if (!val) {
                        code_pos = else_position;
                    }
                    break; }
                case ExecuteBufferReader::Op::LoopControlBreak: {
                    auto depth = ExecuteBufferReader::decode<std::uint8_t>(start_code, code_pos, data_kinds);
                    while(true) {
                        assert(HAS_STATE());
                        if (auto s = TRY_STATE(ForState)) {
                            if (s->loop_control_depth == depth) {
                                code_pos = ExecuteBufferReader::Position{ s->end_position };
                                break;
                            }
                        }
                        else if (auto s = TRY_STATE(WhileState)) {
                            if (s->loop_control_depth == depth) {
                                code_pos = ExecuteBufferReader::Position{ s->end_position };
                                break;
                            }
                        }
                        else if (auto s = TRY_STATE(WithState)) {
                            assert(false);
                        }
                        else if (auto s = TRY_STATE(CatchState)) {
                            assert(false);
                        }
                        POP_STATE();
                    }
                    break; }
                case ExecuteBufferReader::Op::LoopControlContinue: {
                    auto depth = ExecuteBufferReader::decode<std::uint8_t>(start_code, code_pos, data_kinds);
                    while(true) {
                        assert(HAS_STATE());
                        if (auto s = TRY_STATE(ForState)) {
                            if (s->loop_control_depth == depth) {
                                code_pos = ExecuteBufferReader::Position{ s->continue_position };
                                break;
                            }
                        }
                        else if (auto s = TRY_STATE(WhileState)) {
                        if (s->loop_control_depth == depth) {
                                code_pos = ExecuteBufferReader::Position{ s->continue_position };
                                break;
                            }
                        }
                        else if (auto s = TRY_STATE(WithState)) {
                            assert(false);
                        }
                        else if (auto s = TRY_STATE(CatchState)) {
                            exception_being_handled = s->original_exception_being_handled;
                        }
                        // TODO: handle exception
                        POP_STATE();
                    }
                    break; }
                case ExecuteBufferReader::Op::ReturnCloseIterator: {
                    return { OwcaCompleted{}, temporary_ptr, states_ptr, code_pos };
                    }
                case ExecuteBufferReader::Op::Return: {
                    return { OwcaEmpty{}, temporary_ptr, states_ptr, code_pos };
                    }
                case ExecuteBufferReader::Op::ReturnValue: {
                    auto val = PEEK_VALUE(1);
                    POP_VALUES(1);
                    return { val, temporary_ptr, states_ptr, code_pos };
                    }
                case ExecuteBufferReader::Op::Throw: {
                    auto exception = PEEK_VALUE(1);
                    POP_VALUES(1);
                    throw exception.as_exception(vm);
                    }
                case ExecuteBufferReader::Op::TryInit: {
                    PUSH_STATE(TryState{temporary_ptr});
                    auto &state = STATE(TryState);
                    state.begin_position = ExecuteBufferReader::decode_jump(start_code, code_pos, data_kinds);
                    state.end_position = ExecuteBufferReader::decode_jump(start_code, code_pos, data_kinds);
                    state.catches_pos = code_pos;
                    code_pos = ExecuteBufferReader::Position{ state.begin_position };
                    state.temporary_ptr = temporary_ptr;
                    state.original_exception_being_handled = exception_being_handled;
                    break; }
                case ExecuteBufferReader::Op::TryCompleted: {
                    auto &state = STATE(TryState);
                    assert(exception_being_handled == state.original_exception_being_handled);
                    POP_STATE();
                    break; }
                case ExecuteBufferReader::Op::TryCatchType: {
                    auto values = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                    auto ident = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                    auto skip_jump = ExecuteBufferReader::decode_jump(start_code, code_pos, data_kinds);

                    auto exc_types = PEEK_VALUES(values, values);
                    POP_VALUES(values);
                    assert(exception_being_thrown);

                    bool found = false;
                    for(auto e : exc_types) {
                        auto exc_type = e.as_class(vm);
                        if (exception_being_thrown->type().has_base_class(exc_type)) {
                            found = true;
                            break;
                        }
                    }
                    if (found) {
                        if (ident != std::numeric_limits<std::uint32_t>::max()) {
                            LOCAL_VAR(ident) = *exception_being_thrown;
                        }
                        auto &state = STATE(TryState);
                        auto original_exception_being_handled = state.original_exception_being_handled;
                        POP_STATE();
                        PUSH_STATE(CatchState{});
                        auto &state2 = STATE(CatchState);
                        state2.exception_being_handled = exception_being_handled = exception_being_thrown;
                        state2.original_exception_being_handled = original_exception_being_handled;
                    }
                    else {
                        code_pos = ExecuteBufferReader::Position{ skip_jump };
                    }
                    break; }
                case ExecuteBufferReader::Op::TryCatchTypeCompleted: {
                    auto &state = STATE(TryState);
                    POP_STATE();
                    throw *exception_being_thrown;
                    }
                case ExecuteBufferReader::Op::TryBlockCompleted: {
                    auto &state = STATE(CatchState);
                    assert(exception_being_thrown);
                    assert(exception_being_handled);
                    exception_being_thrown = std::nullopt;
                    exception_being_handled = std::nullopt;
                    code_pos = ExecuteBufferReader::decode_jump(start_code, code_pos, data_kinds);
                    break; }
                case ExecuteBufferReader::Op::WhileInit: {
                    PUSH_STATE(WhileState{});
                    auto &state = STATE(WhileState);
                    state.end_position = ExecuteBufferReader::decode_jump(start_code, code_pos, data_kinds);
                    state.loop_index = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                    state.loop_control_depth = ExecuteBufferReader::decode<std::uint8_t>(start_code, code_pos, data_kinds);
                    state.continue_position = code_pos;
                    break; }
                case ExecuteBufferReader::Op::WhileCondition: {
                    auto &state = STATE(WhileState);
                    state.index++;
                    if (state.loop_index != std::numeric_limits<std::uint32_t>::max()) {
                        LOCAL_VAR(state.loop_index) = state.index;
                    }
                    break; }
                case ExecuteBufferReader::Op::WhileNext: {
                    auto &state = STATE(WhileState);

                    auto value = PEEK_VALUE(1).as_bool(vm);
                    POP_VALUES(1);
                    if (!value) {
                        code_pos = ExecuteBufferReader::Position{ state.end_position };
                    }
                    break; }
                case ExecuteBufferReader::Op::WhileCompleted: {
                    POP_STATE();
                    break; }
                case ExecuteBufferReader::Op::WithInit: {
                    PUSH_STATE(WithState{});
                    auto &state = STATE(WithState);
                    auto &obj = PEEK_VALUE(1);
                    state.context = obj;
                    obj = vm->member(obj, "__enter__");
                    obj = execute_call_from_values(temporary_ptr, states_ptr, 1);
                    state.entered = true;
                    auto index = ExecuteBufferReader::decode<std::uint32_t>(start_code, code_pos, data_kinds);
                    if (index != std::numeric_limits<std::uint32_t>::max()) {
                        LOCAL_VAR(index) = obj;
                    }
                    POP_VALUES(1);
                    break; }
                case ExecuteBufferReader::Op::WithCompleted: {
                    auto &state = STATE(WithState);
                    if (state.entered) {
                        auto mbm = vm->member(state.context, "__exit__");
                        PUSH_VALUE(mbm);
                        execute_call_from_values(temporary_ptr, states_ptr, 1);
                        POP_VALUES(1);
                    }
                    POP_STATE();
                    break; }
                case ExecuteBufferReader::Op::Yield: {
                    auto val = PEEK_VALUE(1);
                    POP_VALUES(1);
                    return { val, temporary_ptr, states_ptr, code_pos };
                    }
                case ExecuteBufferReader::Op::Jump: {
                    auto dest = ExecuteBufferReader::decode_jump(start_code, code_pos, data_kinds);
                    code_pos = dest;
                    break; }
                }
next_iteration:
                stacktrace[stacktrace_index].code_position = code_pos;
#ifdef MEASURE
                auto end = std::chrono::high_resolution_clock::now();
                times[(size_t)opcode] += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                counts[(size_t)opcode]++;
#endif
                //std::cout << "setting code position (" << (void*)&frame.code_position << ") to " << frame.code_position << std::endl;
            }
        }
        catch(OwcaException oe) {
            process_thrown_exception(&code_pos, states_ptr, oe);
            goto restart;
        }

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
        if (right == 0) 
            throw_division_by_zero();
        return left / right;
    }
    Number Executor::expr_oper_2(Executor::TagMod, Number left, Number right) {
        if (right == 0)
            throw_division_by_zero();
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
        throw_unsupported_operation_2(tag_name<A>, OwcaValue{ b }.type(), OwcaValue{ c }.type());
    }
    template <typename Tag> void Executor::run_impl_opcodes_execute_expr_oper2(TemporariesPtr &temporary_ptr) {
        auto right = PEEK_VALUE(1);
        auto left = PEEK_VALUE(2);
        auto &ret = PEEK_VALUE(2);
        ret = left.visit([&](auto left_val) -> OwcaValue {
            return right.visit([&](auto right_val) -> OwcaValue {
                return expr_oper_2(Tag{}, left_val, right_val);
            });
        });
        POP_VALUES(1);
    }

    OwcaValue Executor::run_script_function(RuntimeFunction *function, TemporariesPtr temporary_ptr, StatesTypePtr states_ptr, unsigned int arg_count, RuntimeFunction::ScriptFunction& sf, bool clear_locals) {
        auto locals_ptr = temporary_ptr.locals(arg_count);
        const auto max_values = function->max_values;
        temporary_ptr = temporary_ptr + max_values - arg_count;

        assert(locals_ptr.local_values_ptr + max_values + function->max_temporaries <= values_vector.data() + values_vector.size());
        assert(states_ptr.states_type_ptr + function->max_states <= states_vector.data() + states_vector.size());
        if (clear_locals) [[likely]] {
            for(auto i = arg_count; i < max_values; ++i) {
                LOCAL_VAR(i) = OwcaEmpty{};
            }
        }

        assert(sf.copy_from_parents.size() == sf.values_from_parents.size());

        for (auto i = 0u; i < sf.copy_from_parents.size(); ++i) {
            LOCAL_VAR(sf.copy_from_parents[i].index_in_child) = sf.values_from_parents[i];
        }

        PUSH_STATE(EmptyState{});
        auto est = StackTraceState{ *this, function, sf.entry_point };
        auto [ retval, new_values_ptr, new_states_ptr, new_code_pos ] = run_opcodes(locals_ptr, temporary_ptr, states_ptr, StartOfCode{}, sf.entry_point);
        return retval;
    }
    OwcaValue Executor::run_native_function(RuntimeFunction *function, TemporariesPtr temporary_ptr, StatesTypePtr states_ptr, unsigned int arg_count, RuntimeFunction::NativeFunction& sf) {
        auto locals_ptr = temporary_ptr.locals(arg_count);
        const auto max_values = function->max_values;
        temporary_ptr = temporary_ptr + max_values - arg_count;

        assert(locals_ptr.local_values_ptr + max_values + function->max_temporaries <= values_vector.data() + values_vector.size());
        auto est = StackTraceState{ *this, function, {} };
        update_current_top_ptrs(temporary_ptr + function->max_temporaries, states_ptr);
        return sf.function(vm, std::span{ locals_ptr.local_values_ptr, arg_count });
    }
    Generator Executor::run_native_generator(Iterator *iter_object, RuntimeFunction *function, RuntimeFunction::NativeGenerator& ng, std::span<OwcaValue> arguments) {
        assert(arguments.size() > 0);
        Generator generator = ng.generator(vm, arguments);
        while(true) {
            std::optional<OwcaValue> val;
            {
                auto est = StackTraceState{ *this, function, {} };
                val = generator.next();
            }
            iter_object->first_time = false;
            if (val.has_value()) {
                co_yield *val;
            }
            else {
                iter_object->generator.reset();
                break;
            }
        }
    }
    OwcaValue Executor::start_native_generator(RuntimeFunction *function, TemporariesPtr temporary_ptr, StatesTypePtr states_ptr, unsigned int arg_count, RuntimeFunction::NativeGenerator& ng) {
        auto locals_ptr = temporary_ptr.locals(arg_count);
        const auto max_values = function->max_values;
        assert(locals_ptr.local_values_ptr + max_values + function->max_temporaries <= values_vector.data() + values_vector.size());
        auto iter = vm->allocate<Iterator>(0, function, std::span<OwcaValue>{}, std::span<StatesType>{});
        Generator generator = run_native_generator(iter, function, ng, std::span{ locals_ptr.local_values_ptr, arg_count });
        iter->generator = std::move(generator);
        return OwcaIterator{ iter };
    }
    Generator Executor::run_script_generator(Iterator *iter_object, RuntimeFunction *function, std::vector<OwcaValue> values_vec, std::vector<StatesType> states_vec, ExecuteBufferReader::Position code_pos)
    {
        const auto locals_ptr = LocalsPtr{ values_vec.data() };
        auto states_ptr = StatesTypePtr{ states_vec.data() + 1 };
        states_vec[0] = EmptyState{};
        const auto temporary_ptr = temporary_ptr_current_top;
        while(true) {
            OwcaValue val;
            {
                auto est = StackTraceState{ *this, function, code_pos };
                auto [ retval, new_temporary_ptr, new_states_ptr, new_code_pos ] = run_opcodes(locals_ptr, temporary_ptr, states_ptr, StartOfCode{}, code_pos);
                assert(new_temporary_ptr.temporaries_ptr == temporary_ptr.temporaries_ptr);
                val = retval;
                states_ptr = new_states_ptr;
                code_pos = new_code_pos;
            }

            iter_object->first_time = false;
            if (val.kind() == OwcaValueKind::Completed) {
                iter_object->generator.reset();
                break;
            }
            else {
                co_yield val;
            }
        }
    }

    std::optional<OwcaValue> Executor::continue_iterator(OwcaIterator oi) {
        return oi.internal_value()->generator->next();
    }

    OwcaValue Executor::start_script_generator(RuntimeFunction *function, TemporariesPtr temporary_ptr, StatesTypePtr states_ptr, unsigned int arg_count, RuntimeFunction::ScriptFunction& sf) {
        auto locals_ptr = temporary_ptr.locals(arg_count);
        assert(locals_ptr.local_values_ptr + function->max_values + function->max_temporaries <= values_vector.data() + values_vector.size());

        std::vector<OwcaValue> values_vec(function->max_values + function->max_temporaries);
        std::vector<StatesType> states_vec(function->max_states);
        for(auto i = 0u; i < arg_count; ++i) {
            values_vec[i] = LOCAL_VAR(i);
        }

        auto values_span = std::span{ values_vec.data(), values_vec.size() };
        auto states_span = std::span{ states_vec.data(), states_vec.size() };
        auto iter = vm->allocate<Iterator>(0, function, values_span, states_span);
        auto generator = run_script_generator(iter, function, std::move(values_vec), std::move(states_vec), sf.entry_point);
        iter->generator = std::move(generator);
        return OwcaIterator{ iter };
    }

//     void Executor::run() {
//         while(!completed()) {
//             auto &frame = currently_executing_frame();
// #ifdef OWCA_SCRIPT_EXEC_LOG
//             std::cout << "Executing frame at depth " << currently_executing_frame_index() << " (" << (void*)&frame << ") function " << frame.runtime_function->full_name;
//             if (auto q = std::get_if<RuntimeFunction::ScriptFunction>(&frame.runtime_function->data)) {
//                 std::cout << " with code position " << frame.code_position.pos << " (" << (void*)&frame.code_position << ")";
//             }
//             std::cout << std::endl;
// #endif
//             visit_variant(
//                 frame.runtime_function->data,
//                 [&](RuntimeFunction::ScriptFunction& sf) -> void {
//                     if (frame.is_iterator) {
//                         if (!frame.iterator_object) {
//                             auto obj = vm->allocate<Iterator>(0, false);
//                             frame.iterator_object = obj;
//                             auto iter_frame = frame.clone_for_iterator();
//                             obj->frame = std::move(iter_frame);
//                             *frame.return_value = OwcaIterator{ obj };
//                             pop_frame();
//                             return;
//                         }
//                     }
//                     auto frame_index = currently_executing_frame_index();
//                     run_impl_opcodes(frame, sf);
// #ifdef OWCA_SCRIPT_EXEC_LOG
//                     std::cout << "Suspended frame at depth " << frame_index << std::endl;
// #endif
//                 },
//                 [&](RuntimeFunction::NativeFunction& nf) -> void {
//                     try {
//                         *frame.return_value = nf.function(vm, std::span{ frame.values_, frame.max_values });
//                         pop_frame();
//                     }
//                     catch(OwcaException e) {
//                         frame.exception_in_progress = e;
//                         process_thrown_exception(nullptr);
//                     }
//                     catch(const std::exception &e) {
//                         prepare_throw_cpp_exception(std::format("C++ exception during execution of native generator: {}", e.what()));
//                     }
//                     catch(...) {
//                         prepare_throw_cpp_exception("Unknown C++ exception during execution of native generator");
//                     }
//                 },
//                 [&](RuntimeFunction::NativeGenerator& ngf) -> void {
//                     if (!frame.iterator_object) {
//                         frame.iterator_object = vm->allocate<Iterator>(0, true);
//                         frame.iterator_object->internal_value()->generator = ngf.generator(vm, std::span{ frame.values_, frame.max_values });
//                         *frame.return_value = *frame.iterator_object;
//                         auto iter_frame = frame.clone_for_iterator();
//                         frame.iterator_object->internal_value()->frame = std::move(iter_frame);
//                         pop_frame();
//                         return;
//                     }
//                     if (frame.iterator_object->internal_value()->completed) {
//                         *frame.return_value = OwcaCompleted{};
//                         pop_frame();
//                         return;
//                     }
//                     try {
//                         frame.iterator_object->internal_value()->first_time = false;
//                         auto val = frame.iterator_object->internal_value()->generator->next();
//                         if (!val) {
//                             frame.iterator_object->internal_value()->completed = true;
//                             *frame.return_value = frame.iterator_object->internal_value()->last_value = OwcaCompleted{};
//                             pop_frame();
//                             return;
//                         }
//                         *frame.return_value = frame.iterator_object->internal_value()->last_value = *val;
//                         if (frame.return_value->kind() == OwcaValueKind::Completed) {
//                             frame.iterator_object->internal_value()->completed = true;
//                         }
//                         pop_frame();
//                         return;
//                     }
//                     catch(OwcaException e) {
//                         frame.exception_in_progress = e;
//                         process_thrown_exception(nullptr);
//                     }
//                     catch(const std::exception &e) {
//                         prepare_throw_cpp_exception(std::format("C++ exception during execution of native generator: {}", e.what()));
//                     }
//                     catch(...) {
//                         prepare_throw_cpp_exception("Unknown C++ exception during execution of native generator");
//                     }
//                 }
//             );
//         }
//     }
    // void Executor::prepare_execute_code_block(OwcaValue &return_value, const OwcaCode &oc) {
    //     auto frame = ExecutionFrame::create(vm, 0, 1, 0);
    //     frame->initialize_code_block(return_value, vm, oc);
    //     vm->push_frame(frame);
    // }
    // void Executor::prepare_execute_main_function(OwcaValue &return_value, RuntimeFunctions* runtime_functions, std::optional<OwcaMap> arguments) {
    //     auto it = runtime_functions->functions[0];
    //     assert(it != nullptr);
    //     auto frame = ExecutionFrame::create(it);
    //     frame->initialize_main_block_function(return_value, vm, runtime_functions, arguments);
    //     vm->push_frame(frame);
    // }

    // void Executor::prepare_resume_generator(OwcaValue &return_value, OwcaIterator oi)
    // {
	// 	if (oi.internal_value()->completed) {
    //         return_value = OwcaCompleted{};
	// 		return;
	// 	}
    //     vm->push_frame(oi.internal_value()->frame.get());
    //     currently_executing_frame().return_value = &return_value;
    //     oi.internal_value()->first_time = false;
    //     exit = true;
    // }

	OwcaValue Executor::allocate_user_class_from_values(TemporariesPtr temporary_ptr, StatesTypePtr states_ptr, unsigned int arg_count) {
		OwcaValue obj;
        
        assert(arg_count > 0);
        
        auto locals_ptr = temporary_ptr.locals(arg_count);
        auto cls = LOCAL_VAR(0).as_class(vm).internal_value();

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
			if (arg_count > 1) {
				throw_cant_call(std::format("type {} has no __init__ function defined - expected constructor's call with no parameters, instead got {} parameters", cls->full_name, arg_count - 1));
			}
            return obj;
		}
        if (auto state = std::get_if<RuntimeFunctions*>(&it->second)) [[likely]] {
            LOCAL_VAR(0) = obj;
            auto retval = execute_function_call_from_values(*state, temporary_ptr, states_ptr, true, arg_count);
            if (!cls->reload_self) [[likely]]
                retval = obj;
            return retval;
        }
        throw_cant_call(std::format("type {} has __init__ variable, not a function", std::get<Class*>(it->second)->full_name));
	}
    // OwcaValue Executor::allocate_user_class(Class *cls, std::span<OwcaValue> arguments) {
    //     OwcaValue return_value;
    //     prepare_allocate_user_class(return_value, cls, arguments);
    //     run();
    //     return return_value;
    // }
    OwcaValue Executor::execute_call_from_values(TemporariesPtr temporary_ptr, StatesTypePtr states_ptr, unsigned int argument_count) {
        auto func = PEEK_VALUE(argument_count);
        if (func.kind() == OwcaValueKind::Functions) [[likely]] {
            auto f = func.as_functions(vm);
            auto runtime_functions = f.internal_value();
            bool has_self = f.internal_self_object() != nullptr;
            PEEK_VALUE(argument_count) = has_self ? *f.self() : OwcaEmpty{};
            
            return execute_function_call_from_values(runtime_functions, temporary_ptr, states_ptr, has_self, argument_count - (has_self ? 0 : 1));
        }
        else {
            if (func.kind() == OwcaValueKind::Class) [[likely]] {
                return allocate_user_class_from_values(temporary_ptr, states_ptr, argument_count);
            }
            throw_cant_call(std::format("can't call {} with {} parameters", func.type(), argument_count - 1));
        }
    }
    OwcaValue Executor::execute_function_call_from_values(RuntimeFunctions* runtime_functions, TemporariesPtr temporary_ptr, StatesTypePtr states_ptr, bool has_self, unsigned int arg_count) {
        auto runtime_function = runtime_functions->functions[arg_count];
        if (!runtime_function && has_self) [[unlikely]] {
            runtime_function = runtime_functions->functions[arg_count - 1];
            --arg_count;
        }
        if (!runtime_function) [[unlikely]] {
            auto tmp = std::string{ "function " };
            tmp += runtime_functions->name;
            throw_not_callable_wrong_number_of_params(std::move(tmp), arg_count + (has_self ? 1 : 0));
        }
        if (auto state = std::get_if<RuntimeFunction::ScriptFunction>(&runtime_function->data)) [[likely]] {
            return run_script_function(runtime_function, temporary_ptr, states_ptr, arg_count, *state);
        }
        return run_native_function(runtime_function, temporary_ptr, states_ptr, arg_count, std::get<RuntimeFunction::NativeFunction>(runtime_function->data));
    }


	OwcaValue Executor::execute_code_block(const OwcaCode &oc, std::optional<OwcaMap> values_to_set, OwcaMap *dict_output)
	{
        auto tpk = TopPtrsKeeper{ *this };

        auto runtime_function = vm->allocate<RuntimeFunction>(0, oc, std::string_view("main-code-block"), std::string_view("main-code-block"), RuntimeFunction::ScriptFunction{
            .entry_point = CodePosition{ oc.code().data()}
        });
        auto temporary_ptr = temporary_ptr_current_top;
        auto locals_ptr = temporary_ptr.locals(0);
        auto states_ptr = states_ptr_current_top;
        PUSH_STATE(EmptyState{});
        auto fnc = run_script_function(runtime_function, temporary_ptr, states_ptr, 0, std::get<RuntimeFunction::ScriptFunction>(runtime_function->data));
        auto f = fnc.as_functions(vm);
        auto f2 = f.internal_value()->functions[0];
        assert(f2);
        assert(!f.self());

        {
            auto &sf = std::get<RuntimeFunction::ScriptFunction>(f2->data);
            std::unordered_map<std::string_view, unsigned int> value_index_map;
            for (auto i = 0u; i < sf.identifier_names.size(); ++i) {
                value_index_map[sf.identifier_names[i]] = i;
            }
            
            for(auto &it : vm->builtin_objects) {
                auto it2 = value_index_map.find(it.first);
                assert(it2 != value_index_map.end());
                LOCAL_VAR(it2->second) = it.second;
            }
            if (values_to_set) {
                for (auto it : *values_to_set) {
                    auto key = it.first.as_string(vm).text();
                    auto it2 = value_index_map.find(key);
                    if (it2 != value_index_map.end()) {
                        LOCAL_VAR(it2->second) = it.second;
                    }
                }
            }
        }

        auto retval = run_script_function(f2, temporary_ptr, states_ptr, 0, std::get<RuntimeFunction::ScriptFunction>(f2->data), false);
        if (dict_output) {
            f2->visit(
                [&](RuntimeFunction::ScriptFunction& sf) -> void {
                    for (auto i = 0u; i < sf.identifier_names.size(); ++i) {
                        auto key = vm->create_string_from_view(sf.identifier_names[i]);
                        (*dict_output)[key] = LOCAL_VAR(i);
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
        return retval;
    }

    OwcaValue Executor::allocate_user_class(Class *cls, std::span<OwcaValue> arguments) {
        auto tpk = TopPtrsKeeper{ *this };

        auto temporary_ptr = temporary_ptr_current_top;
        auto states_ptr = states_ptr_current_top;
        auto locals_ptr = temporary_ptr.locals(0);
        PUSH_VALUE(OwcaClass{ cls });
        for(auto &v : arguments) {
            PUSH_VALUE(v);
        }

        return allocate_user_class_from_values(temporary_ptr, states_ptr, (unsigned int)arguments.size() + 1);

    }
    OwcaValue Executor::execute_call(OwcaValue func, std::span<OwcaValue> arguments) {
        auto tpk = TopPtrsKeeper{ *this };

        auto temporary_ptr = temporary_ptr_current_top;
        auto states_ptr = states_ptr_current_top;
        auto locals_ptr = temporary_ptr.locals(0);
        LOCAL_VAR(0) = func;
        for(size_t i = 0; i < arguments.size(); ++i) {
            LOCAL_VAR(i + 1) = arguments[i];
        }
        return execute_call_from_values(temporary_ptr + (unsigned int)arguments.size() + 1, states_ptr, (unsigned int)arguments.size() + 1);
    }
    // bool Executor::prepare_execute_function(OwcaValue &return_value, RuntimeFunctions* runtime_functions, std::optional<OwcaValue> self_value, std::span<OwcaValue> arguments)
    // {
	// 	auto it = runtime_functions->functions[arguments.size() + (self_value ? 1 : 0)];
	// 	if (it == nullptr) {
	// 		it = runtime_functions->functions[arguments.size()];
	// 		if (it == nullptr || it->is_method) {
	// 			auto tmp = std::string{ "function " };
	// 			tmp += runtime_functions->name;
	// 			prepare_throw_not_callable_wrong_number_of_params(std::move(tmp), arguments.size());
    //             return false;
	// 		}
	// 	}

    //     auto frame = ExecutionFrame::create(it);
    //     frame->initialize_execute_function(return_value, vm, it, self_value, arguments);
    //     vm->push_frame(frame);
    //     exit = true;
    //     return true;
    // }

    // void Executor::prepare_execute_call(OwcaValue &return_value, OwcaValue func, std::span<OwcaValue> arguments)
    // {
	// 	return func.visit(
	// 		[&](OwcaIterator oi) -> void {
	// 			if (!arguments.empty()) {
    //                 prepare_throw_not_callable_wrong_number_of_params("generator", (unsigned int)arguments.size());
    //             }
    //             else {
    //                 prepare_resume_generator(return_value, oi);
    //             }
	// 		},
	// 		[&](OwcaFunctions of) -> void {
	// 			auto runtime_functions = func.as_functions(vm).internal_value();
    //             prepare_execute_function(return_value, runtime_functions, of.self(), arguments);
	// 		},
	// 		[&](OwcaClass oc) -> void {
	// 			auto cls = func.as_class(vm).internal_value();
	// 			prepare_allocate_user_class(return_value, cls, arguments);
	// 		},
	// 		[&](const auto&) -> void {
	// 			prepare_throw_not_callable(func.type());
	// 		}
	// 	);
    // }


    // void Executor::prepare_throw_cpp_exception(std::string_view msg)
    // {
    //     OwcaValue temp_arg = vm->create_string_from_view(msg);
    //     prepare_allocate_user_class(temporary_exception_return_value, vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }, true);
    // }
	void Executor::throw_too_many_elements(size_t expected)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("too many values to unpack (expected {})", expected));
        throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}
	void Executor::throw_not_enough_elements(size_t expected, size_t got)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("not enough values to unpack (expected {}, got {})", expected, got));
        throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}
	void Executor::throw_dictionary_changed(bool is_dict)
	{
        OwcaValue temp_arg = is_dict ? vm->create_string_from_view("dictionary changed during iteration") : vm->create_string_from_view("set changed during iteration");
        throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}
	void Executor::throw_not_implemented(std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
        throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}
	void Executor::throw_range_step_is_zero()
	{
        OwcaValue temp_arg = vm->create_string_from_view("range step is zero");
        throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}
	void Executor::throw_division_by_zero()
	{
        OwcaValue temp_arg = vm->create_string_from_view("division by zero");
        throw allocate_user_class(vm->c_math_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}
	void Executor::throw_mod_division_by_zero()
	{
        OwcaValue temp_arg = vm->create_string_from_view("modulo by zero");
        throw allocate_user_class(vm->c_math_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_cant_convert_to_float_message(std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
        throw allocate_user_class(vm->c_math_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_cant_convert_to_float(std::string_view type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("can't convert value of type `{}` to floating point", type));
        throw allocate_user_class(vm->c_math_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_cant_convert_to_integer(Number val)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("can't convert {} to integer", val));
        throw allocate_user_class(vm->c_math_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_cant_convert_to_integer(std::string_view type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("can't convert {} to integer", type));
        throw allocate_user_class(vm->c_math_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_not_a_number(std::string_view type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} is not a number", type));
        throw allocate_user_class(vm->c_math_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_overflow(std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
        throw allocate_user_class(vm->c_math_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_range_step_must_be_one_in_left_side_of_write_assign()
	{
        OwcaValue temp_arg = vm->create_string_from_view("step of a range must be 1 in left side of write assignment");
		throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_cant_compare(CompareKind kind, std::string_view left, std::string_view right)
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
        throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_string_too_large(size_t size)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("string is too large ({} bytes)", size));
        throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}
	void Executor::throw_index_out_of_range(std::string msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
        throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_value_not_indexable(std::string_view type, std::string_view key_type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} is not indexable with key {}", type, key_type));
		throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_missing_member(std::string_view type, std::string_view ident)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} doesn't have a member {}", type, ident));
		throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_cant_call(std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
		throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_not_callable(std::string_view type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} is not callable", type));
		throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}
	
	void Executor::throw_not_callable_wrong_number_of_params(std::string_view type, unsigned int params)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} is not callable - wrong number of parameters ({})", type, params));
		throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_wrong_type(std::string_view type, std::string_view expected)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("wrong type {} - expected {}", type, expected));
		throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_wrong_type(std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
		throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_unsupported_operation_2(std::string_view oper, std::string_view left, std::string_view right)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("can't execute {} {} {}", left, oper, right));
		throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_invalid_operand_for_mul_string(std::string_view val)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("can't multiply string by {}", val));
		throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_missing_key(std::string_view key)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("missing key {}", key));
		throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_not_hashable(std::string_view type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} is not hashable", type));
		throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_value_cant_have_fields(std::string_view type)
	{
        OwcaValue temp_arg = vm->create_string_from_view(std::format("{} can't have fields", type));
		throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_missing_native(std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
		throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_not_iterable(std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
		throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_readonly(std::string_view msg)
	{
        OwcaValue temp_arg = vm->create_string_from_view(msg);
		throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_cant_return_value_from_generator()
	{
        OwcaValue temp_arg = vm->create_string_from_view("can't return value from generator");
		throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}

	void Executor::throw_container_is_empty()
	{
        OwcaValue temp_arg = vm->create_string_from_view("container is empty");
		throw allocate_user_class(vm->c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception(vm);
	}


    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const Executor::WhileState &e) {
    }
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const Executor::ClassState &e) {
        gc_mark_value(vm, generation_gc, e.cls);
    }
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const Executor::ForState &e) {
        gc_mark_value(vm, generation_gc, e.iterator);
    }
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const Executor::TryState &e) {
        if (e.original_exception_being_handled)
            gc_mark_value(vm, generation_gc, *e.original_exception_being_handled);
    }
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const Executor::CatchState &e) {
        if (e.exception_being_handled)
            gc_mark_value(vm, generation_gc, *e.exception_being_handled);
        if (e.original_exception_being_handled)
            gc_mark_value(vm, generation_gc, *e.original_exception_being_handled);
    }
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const Executor::WithState &e) {
        gc_mark_value(vm, generation_gc, e.context);
    }
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const Executor::StatesType &e) {
        visit_variant(e,
            [&](const auto &a) { gc_mark_value(vm, generation_gc, a); }
        );
    }
    void gc_mark_value(OwcaVM vm, GenerationGC ggc, const Executor &e) {
        for(auto v = e.values_vector.data(); v < e.temporary_ptr_current_top.temporaries_ptr; ++v) {
            gc_mark_value(vm, ggc, *v);
        }
        for(auto s = e.states_vector.data(); s < e.states_ptr_current_top.states_type_ptr; ++s) {
            gc_mark_value(vm, ggc, *s);
        }
        for(auto &s : e.stacktrace) {
            gc_mark_value(vm, ggc, s.runtime_function);
        }
        if (e.exception_being_thrown) {
            gc_mark_value(vm, ggc, *e.exception_being_thrown);
        }
        if (e.exception_being_handled) {
            gc_mark_value(vm, ggc, *e.exception_being_handled);
        }
    }
}