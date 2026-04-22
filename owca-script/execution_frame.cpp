#include "exec_buffer.h"
#include "stdafx.h"
#include "execution_frame.h"
#include "owca_value.h"
#include "vm.h"
#include "runtime_function.h"
#include "object.h"
#include "executor.h"
#include "iterator.h"

namespace OwcaScript::Internal {
    ExecutionFrame::ExecutionFrame() = default;
    ExecutionFrame::~ExecutionFrame() = default;

    // void ExecutionFrame::set_arguments(OwcaMap arguments) {
    //     assert(runtime_function);
	// 	runtime_function->visit(
	// 		[&](RuntimeFunction::ScriptFunction& sf) -> void {
	// 			std::unordered_map<std::string_view, unsigned int> value_index_map;
	// 			for (auto i = 0u; i < sf.identifier_names.size(); ++i) {
	// 				value_index_map[sf.identifier_names[i]] = i;
	// 			}
				
	// 			for(auto &it : vm->builtin_objects) {
	// 				auto it2 = value_index_map.find(it.first);
	// 				assert(it2 != value_index_map.end());
	// 				vm->set_identifier(it2->second, it.second);
	// 			}
    //             for (auto it : *arguments) {
    //                 auto key = it.first.as_string(vm).text();
    //                 auto it2 = value_index_map.find(key);
    //                 if (it2 != value_index_map.end()) {
    //                     vm->set_identifier(it2->second, it.second);
    //                 }
    //             }
	// 		},
	// 		[&](RuntimeFunction::NativeFunction&) -> void {
	// 			assert(false);
	// 		},
	// 		[&](RuntimeFunction::NativeGenerator&) -> void {
	// 			assert(false);
	// 		}
	// 	);

    //     std::unordered_map<std::string_view, unsigned int> value_index_map;
    //     for (auto i = 0u; i < sf.identifier_names.size(); ++i) {
    //         value_index_map[sf.identifier_names[i]] = i;
    //     }
        
    //     for(auto &it : vm->builtin_objects) {
    //         auto it2 = value_index_map.find(it.first);
    //         assert(it2 != value_index_map.end());
    //         vm->set_identifier(it2->second, it.second);
    //     }
    //     if (arguments) {
    //         for (auto it : *arguments) {
    //             auto key = it.first.as_string(vm).text();
    //             auto it2 = value_index_map.find(key);
    //             if (it2 != value_index_map.end()) {
    //                 vm->set_identifier(it2->second, it.second);
    //             }
    //         }
    //     }

    // }
    OwcaValue ExecutionFrame::get_identifier(unsigned int index) {
        assert(index < max_values);
        return values_[index];
    }
    void ExecutionFrame::set_identifier(unsigned int index, OwcaValue value) {
        assert(index < max_values);
        values_[index] = value;
    }
    void ExecutionFrame::set_identifier_function(VM *vm, unsigned int index, OwcaValue value) {
        assert(index < max_values);
        assert(value.kind() == OwcaValueKind::Functions);
        auto fnc = value.as_functions(vm);
        assert(fnc.internal_value()->functions.size() == 1);
        auto &dst = values_[index];
        if (dst.kind() == OwcaValueKind::Functions) {
            auto dst_fnc = dst.as_functions(vm);
            for(auto it : fnc.internal_value()->functions) {
                dst_fnc.internal_value()->functions[it.first] = it.second;
            }
            return;
        }
        dst = value;
    }
    
    void ExecutionFrame::initialize_execute_function(OwcaValue &return_value, VM *vm, RuntimeFunction* runtime_function, std::optional<OwcaValue> self_value, std::span<OwcaValue> arguments) {
		this->runtime_function = runtime_function;
        this->return_value = &return_value;
		runtime_function->visit(
			[&](const RuntimeFunction::NativeFunction& nf) {
                assert(max_values <= nf.parameter_names.size());
			},
			[&](const RuntimeFunction::NativeGenerator& nf) {
                assert(max_values <= nf.parameter_names.size());
                is_iterator = true;
			},
			[&](const RuntimeFunction::ScriptFunction& sf) {
                assert(max_values <= sf.identifier_names.size());
				assert(sf.copy_from_parents.size() == sf.values_from_parents.size());

				for (auto i = 0u; i < sf.copy_from_parents.size(); ++i) {
					values_[sf.copy_from_parents[i].index_in_child] = sf.values_from_parents[i];
				}
                code_position = sf.entry_point;
#ifdef OWCA_SCRIPT_EXEC_LOG                
                std::cout << __FILE__ << ":" << __LINE__ << ": setting code position (" << (void*)&code_position << ") to " << code_position.pos << std::endl;
#endif
                is_iterator = sf.is_generator;
			});

        bool self = runtime_function->is_method;
        if (self) {
            if (self_value) {
                values_[0] = *self_value;
            }
            else {
                Executor executor{ vm };
                executor.throw_cant_call(std::format("can't call {} - missing self value", runtime_function->to_string()));
            }
        }
        for (auto i = (self ? 1u : 0u); i < runtime_function->param_count; ++i) {
            values_[i] = arguments[i - (self ? 1u : 0u)];
        }
    }
    void ExecutionFrame::initialize_main_block_function(OwcaValue &return_value, VM *vm, RuntimeFunctions* runtime_functions, std::optional<OwcaMap> arguments) {
        assert(runtime_functions);
        this->return_value = &return_value;
		assert(runtime_functions->name.find("main-block") != std::string::npos);
		assert(runtime_functions->functions.size() == 1);
        assert(runtime_functions->functions.begin()->first == 0);
		auto &function = runtime_functions->functions.begin()->second;
		assert(function->is_method == false);

        this->runtime_function = function;
		runtime_function->visit(
			[&](RuntimeFunction::ScriptFunction& sf) -> void {
                this->code_position = sf.entry_point;
#ifdef OWCA_SCRIPT_EXEC_LOG                
                std::cout << __FILE__ << ":" << __LINE__ << ": setting code position (" << (void*)&this->code_position << ") to " << this->code_position.pos << std::endl;
#endif
                assert(max_values == sf.identifier_names.size());
				std::unordered_map<std::string_view, unsigned int> value_index_map;
				for (auto i = 0u; i < sf.identifier_names.size(); ++i) {
					value_index_map[sf.identifier_names[i]] = i;
				}
				
				for(auto &it : vm->builtin_objects) {
					auto it2 = value_index_map.find(it.first);
					assert(it2 != value_index_map.end());
					set_identifier(it2->second, it.second);
				}
                if (arguments) {
                    for (auto it : *arguments) {
                        auto key = it.first.as_string(vm).text();
                        auto it2 = value_index_map.find(key);
                        if (it2 != value_index_map.end()) {
                            set_identifier(it2->second, it.second);
                        }
                    }
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
    void ExecutionFrame::initialize_code_block(OwcaValue &return_value, VM *vm, const OwcaCode &oc)
    {
#ifdef OWCA_SCRIPT_EXEC_LOG        
        std::cout << __FILE__ << ":" << __LINE__ << ": setting code position (" << (void*)&code_position << ") to " << code_position.pos << std::endl;
#endif

        runtime_function = vm->allocate<RuntimeFunction>(0, oc, std::string_view("main-code-block"), std::string_view("main-code-block"), RuntimeFunction::ScriptFunction{
            .entry_point = CodePosition{ oc.code().data()}
        });
        initialize_execute_function(return_value, vm, runtime_function, std::nullopt, std::span<OwcaValue>{});
    }
		// assert(runtime_functions->name.find("main-block") != std::string::npos);
		// assert(runtime_functions->functions.size() == 1);
        // assert(runtime_functions->functions.begin()->first == 0);
		// auto &function = runtime_functions->functions.begin()->second;
		// assert(function->is_method == false);

        // auto &frame = vm->push_new_frame();
        // frame.runtime_functions = runtime_functions;
        // frame.runtime_function = function;
		// function->visit(
		// 	[&](RuntimeFunction::ScriptFunction& sf) -> void {
        //         frame.code = &sf.code;
        //         frame.code_position = sf.entry_point;
        //         assert(!sf.is_generator);

		// 		std::unordered_map<std::string_view, unsigned int> value_index_map;
		// 		for (auto i = 0u; i < sf.identifier_names.size(); ++i) {
		// 			value_index_map[sf.identifier_names[i]] = i;
		// 		}
				
		// 		for(auto &it : vm->builtin_objects) {
		// 			auto it2 = value_index_map.find(it.first);
		// 			assert(it2 != value_index_map.end());
		// 			vm->set_identifier(it2->second, it.second);
		// 		}
		// 		if (arguments) {
		// 			for (auto it : *arguments) {
		// 				auto key = it.first.as_string(vm).text();
		// 				auto it2 = value_index_map.find(key);
		// 				if (it2 != value_index_map.end()) {
		// 					vm->set_identifier(it2->second, it.second);
		// 				}
		// 			}
		// 		}
		// 	},
		// 	[&](RuntimeFunction::NativeFunction&) -> void {
		// 		assert(false);
		// 	},
		// 	[&](RuntimeFunction::NativeGenerator&) -> void {
		// 		assert(false);
		// 	}
		// );
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ExecutionFrame::WhileState &e) {
    }
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ExecutionFrame::ClassState &e) {
        gc_mark_value(vm, generation_gc, e.cls);
    }
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ExecutionFrame::ForState &e) {
        gc_mark_value(vm, generation_gc, e.iterator);
    }
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ExecutionFrame::TryCatchState &e) {
        if (e.parent_exception)
            gc_mark_value(vm, generation_gc, *e.parent_exception);
    }
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ExecutionFrame::WithState &e) {
        gc_mark_value(vm, generation_gc, e.context);
    }
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ExecutionFrame &e)
    {
        gc_mark_value(vm, generation_gc, e.runtime_function);
        auto [ b_state_kinds, b_states, b_values, b_temporaries ] = e.calculate_blocks();
        for(auto i = 0u; i < e.current_state; ++i) {
            switch(b_state_kinds[i]) {
            case 0:
                gc_mark_value(vm, generation_gc, b_states[i].class_state);
                break;
            case 1:
                gc_mark_value(vm, generation_gc, b_states[i].for_state);
                break;
            case 2:
                gc_mark_value(vm, generation_gc, b_states[i].while_state);
                break;
            case 3:
                gc_mark_value(vm, generation_gc, b_states[i].try_catch_state);
                break;
            case 4:
                gc_mark_value(vm, generation_gc, b_states[i].with_state);
                break;
            default:
                assert(false);
            }
        }
        for(auto &v : b_values) {
            gc_mark_value(vm, generation_gc, v);
        }
        for(auto v = b_temporaries.data(); v < e.temporary_; ++v) {
            gc_mark_value(vm, generation_gc, *v);
        }
    }
    std::tuple<std::span<std::uint8_t>, std::span<ExecutionFrame::StatesType>, std::span<OwcaValue>, std::span<OwcaValue>> ExecutionFrame::calculate_blocks() const {
        auto ptr = const_cast<char*>((const char*)this) + sizeof(ExecutionFrame);
        auto state_kinds = std::span<std::uint8_t>{ reinterpret_cast<std::uint8_t*>(ptr), max_states };
        ptr += (max_states + 7) & ~7u;
        auto states = std::span<StatesType>{ reinterpret_cast<StatesType*>(ptr), max_states };
        ptr += sizeof(StatesType) * max_states;
        auto values = std::span<OwcaValue>{ reinterpret_cast<OwcaValue*>(ptr), max_values };
        ptr += sizeof(OwcaValue) * max_values;
        auto temporaries = std::span<OwcaValue>{ reinterpret_cast<OwcaValue*>(ptr), max_temporaries };
        return { state_kinds, states, values, temporaries };
    }
    ExecutionFrame *ExecutionFrame::create(RuntimeFunction *runtime_function) {
        auto f = create(runtime_function->vm, runtime_function->max_values, runtime_function->max_temporaries, runtime_function->max_states);
        f->runtime_function = runtime_function;
        return f;
    }
    std::unique_ptr<ExecutionFrame> ExecutionFrame::clone_for_iterator() {
        assert(runtime_function->is_generator());

        auto additional_size = calculate_additional_size(runtime_function->max_values, runtime_function->max_temporaries, runtime_function->max_states);
        auto ptr = new char[sizeof(ExecutionFrame) + additional_size];
        auto frame = new (ptr) ExecutionFrame();
        std::unique_ptr<ExecutionFrame> result{ frame };
        result->initialize(runtime_function->max_values, runtime_function->max_temporaries, runtime_function->max_states);
        frame->iterator_object = iterator_object;

        auto [ b_state_kinds, b_states, b_values, b_temporaries ] = frame->calculate_blocks();
        auto [ ob_state_kinds, ob_states, ob_values, ob_temporaries ] = calculate_blocks();
        std::copy(ob_state_kinds.data(), ob_state_kinds.data() + ob_state_kinds.size(), b_state_kinds.data());
        std::copy(ob_states.data(), ob_states.data() + ob_states.size(), b_states.data());
        std::copy(ob_values.data(), ob_values.data() + ob_values.size(), b_values.data());
        std::copy(ob_temporaries.data(), ob_temporaries.data() + ob_temporaries.size(), b_temporaries.data());
        result->runtime_function = runtime_function;
        result->is_iterator = true;
        result->current_state = current_state;
        result->code_position = code_position;
        result->temporary_ += temporary_ - ob_temporaries.data();

        return result;
    }
    ExecutionFrame *ExecutionFrame::create(VM *vm, std::uint16_t values, std::uint16_t temporaries, std::uint16_t states) {
        auto additional_size = calculate_additional_size(values, temporaries, states);
        auto frame = vm->allocate_stack_frame(additional_size);
        frame->initialize(values, temporaries, states);
        return frame;
    }
    size_t ExecutionFrame::calculate_additional_size(std::uint16_t values, std::uint16_t temporaries, std::uint16_t states) {
        return (sizeof(OwcaValue) * values) + (sizeof(OwcaValue) * temporaries) + (sizeof(StatesType) * states) + ((states + 7) & ~7u);
    }
    void ExecutionFrame::initialize(std::uint16_t values, std::uint16_t temporaries, std::uint16_t states) {
        auto additional_size = calculate_additional_size(values, temporaries, states);
        max_values = values;
        max_temporaries = temporaries;
        max_states = states;
        auto [ b_state_kinds, b_states, b_values, b_temporaries ] = calculate_blocks();
        assert(b_state_kinds.data() >= reinterpret_cast<std::uint8_t*>(this) + sizeof(ExecutionFrame));
        assert(b_state_kinds.data() + b_state_kinds.size() <= reinterpret_cast<std::uint8_t*>(this) + sizeof(ExecutionFrame) + additional_size);
        assert((std::uint8_t*)b_states.data() >= reinterpret_cast<std::uint8_t*>(this) + sizeof(ExecutionFrame));
        assert((std::uint8_t*)(b_states.data() + b_states.size()) <= reinterpret_cast<std::uint8_t*>(this) + sizeof(ExecutionFrame) + additional_size);
        assert((std::uint8_t*)b_values.data() >= reinterpret_cast<std::uint8_t*>(this) + sizeof(ExecutionFrame));
        assert((std::uint8_t*)(b_values.data() + b_values.size()) <= reinterpret_cast<std::uint8_t*>(this) + sizeof(ExecutionFrame) + additional_size);
        assert((std::uint8_t*)b_temporaries.data() >= reinterpret_cast<std::uint8_t*>(this) + sizeof(ExecutionFrame));
        assert((std::uint8_t*)(b_temporaries.data() + b_temporaries.size()) == reinterpret_cast<std::uint8_t*>(this) + sizeof(ExecutionFrame) + additional_size);
        
        state_kinds_ = b_state_kinds.data();
        state_ = b_states.data();
        values_ = b_values.data();
        temporary_ = b_temporaries.data();
        for(auto &v : b_values) v = OwcaEmpty{};
#ifdef DEBUG
        temporaries_begin_ = b_temporaries.data();
        temporaries_end_ = b_temporaries.data() + b_temporaries.size();
#endif
    }
    
}