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
                std::cout << __FILE__ << ":" << __LINE__ << ": setting code position (" << (void*)&code_position << ") to " << code_position << std::endl;
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
                std::cout << __FILE__ << ":" << __LINE__ << ": setting code position (" << (void*)&this->code_position << ") to " << this->code_position << std::endl;
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
        std::cout << __FILE__ << ":" << __LINE__ << ": setting code position (" << (void*)&code_position << ") to " << code_position << std::endl;
#endif

        runtime_function = vm->allocate<RuntimeFunction>(0, oc, std::string_view("main-code-block"), std::string_view("main-code-block"), RuntimeFunction::ScriptFunction{});
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
    std::unique_ptr<ExecutionFrame> ExecutionFrame::create(RuntimeFunction *runtime_function) {
        auto f = create(runtime_function->max_values, runtime_function->max_temporaries, runtime_function->max_states);
        f->runtime_function = runtime_function;
        return f;
    }
    std::unique_ptr<ExecutionFrame> ExecutionFrame::create(std::uint16_t values, std::uint16_t temporaries, std::uint16_t states) {
        auto additional_size = (sizeof(OwcaValue) * values) + (sizeof(OwcaValue) * temporaries) + (sizeof(StatesType) * states) + ((states + 7) & ~7u);
        auto frame = std::unique_ptr<ExecutionFrame>(new (new char[sizeof(ExecutionFrame) + additional_size]) ExecutionFrame());
        frame->max_values = values;
        frame->max_temporaries = temporaries;
        frame->max_states = states;
        auto [ b_state_kinds, b_states, b_values, b_temporaries ] = frame->calculate_blocks();
        assert(b_state_kinds.data() >= reinterpret_cast<std::uint8_t*>(frame.get()) + sizeof(ExecutionFrame));
        assert(b_state_kinds.data() + b_state_kinds.size() <= reinterpret_cast<std::uint8_t*>(frame.get()) + sizeof(ExecutionFrame) + additional_size);
        assert((std::uint8_t*)b_states.data() >= reinterpret_cast<std::uint8_t*>(frame.get()) + sizeof(ExecutionFrame));
        assert((std::uint8_t*)(b_states.data() + b_states.size()) <= reinterpret_cast<std::uint8_t*>(frame.get()) + sizeof(ExecutionFrame) + additional_size);
        assert((std::uint8_t*)b_values.data() >= reinterpret_cast<std::uint8_t*>(frame.get()) + sizeof(ExecutionFrame));
        assert((std::uint8_t*)(b_values.data() + b_values.size()) <= reinterpret_cast<std::uint8_t*>(frame.get()) + sizeof(ExecutionFrame) + additional_size);
        assert((std::uint8_t*)b_temporaries.data() >= reinterpret_cast<std::uint8_t*>(frame.get()) + sizeof(ExecutionFrame));
        assert((std::uint8_t*)(b_temporaries.data() + b_temporaries.size()) == reinterpret_cast<std::uint8_t*>(frame.get()) + sizeof(ExecutionFrame) + additional_size);
        
        frame->state_kinds_ = b_state_kinds.data();
        frame->state_ = b_states.data();
        frame->values_ = b_values.data();
        frame->temporary_ = b_temporaries.data();
        for(auto &v : b_values) v = OwcaEmpty{};
#ifdef DEBUG
        frame->temporaries_begin_ = b_temporaries.data();
        frame->temporaries_end_ = b_temporaries.data() + b_temporaries.size();
#endif
        return frame;
    }
    
}