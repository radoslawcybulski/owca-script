#include "stdafx.h"
#include "execution_frame.h"
#include "owca_value.h"
#include "vm.h"
#include "runtime_function.h"

namespace OwcaScript::Internal {
    ExecutionFrame::ExecutionFrame() {
        values.reserve(8);
        temporaries.reserve(8);
    }
    ExecutionFrame::~ExecutionFrame() = default;

    void ExecutionFrame::clear()
    {
        runtime_functions = nullptr;
        runtime_function = nullptr;
        values.clear();
        temporaries.clear();
        states.clear();
        prev_code_position =code_position = 0;
        return_value = nullptr;
        constructor_move_self_to_return_value = false;
        iterator_update_completed = false;
    }

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
    void ExecutionFrame::initialize_execute_function(OwcaValue &return_value, VM *vm, RuntimeFunctions* runtime_functions, RuntimeFunction* runtime_function, std::optional<OwcaValue> self_value, std::span<OwcaValue> arguments) {
        clear();
		this->runtime_functions = runtime_functions;
		this->runtime_function = runtime_function;
        this->return_value = &return_value;
		runtime_function->visit(
			[&](const RuntimeFunction::NativeFunction& nf) {
				values.resize(nf.parameter_names.size());
			},
			[&](const RuntimeFunction::NativeGenerator& nf) {
				values.resize(nf.parameter_names.size());
			},
			[&](const RuntimeFunction::ScriptFunction& sf) {
				values.resize(sf.identifier_names.size());
				assert(sf.copy_from_parents.size() == sf.values_from_parents.size());

				for (auto i = 0u; i < sf.copy_from_parents.size(); ++i) {
					values[sf.copy_from_parents[i].index_in_child] = sf.values_from_parents[i];
				}
                prev_code_position = code_position = sf.entry_point;
                iterator_update_completed = sf.is_generator;
			});

        bool self = runtime_function->is_method;
        if (self) {
            if (self_value) {
                values[0] = *self_value;
            }
            else {
                vm->throw_cant_call(std::format("can't call {} - missing self value", runtime_function->to_string()));
            }
        }
        for (auto i = (self ? 1u : 0u); i < runtime_function->param_count; ++i) {
            values[i] = arguments[i - (self ? 1u : 0u)];
        }
    }
    void ExecutionFrame::initialize_main_block_function(OwcaValue &return_value, VM *vm, RuntimeFunctions* runtime_functions, std::optional<OwcaMap> arguments) {
        clear();
        assert(runtime_function);
        this->runtime_functions = runtime_functions;
		assert(runtime_functions->name.find("main-block") != std::string::npos);
		assert(runtime_functions->functions.size() == 1);
        assert(runtime_functions->functions.begin()->first == 0);
		auto &function = runtime_functions->functions.begin()->second;
		assert(function->is_method == false);

        this->runtime_function = function;
		runtime_function->visit(
			[&](RuntimeFunction::ScriptFunction& sf) -> void {
                this->prev_code_position = this->code_position = sf.entry_point;
				std::unordered_map<std::string_view, unsigned int> value_index_map;
				for (auto i = 0u; i < sf.identifier_names.size(); ++i) {
					value_index_map[sf.identifier_names[i]] = i;
				}
				
				for(auto &it : vm->builtin_objects) {
					auto it2 = value_index_map.find(it.first);
					assert(it2 != value_index_map.end());
					vm->set_identifier(it2->second, it.second);
				}
                if (arguments) {
                    for (auto it : *arguments) {
                        auto key = it.first.as_string(vm).text();
                        auto it2 = value_index_map.find(key);
                        if (it2 != value_index_map.end()) {
                            vm->set_identifier(it2->second, it.second);
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
        clear();
        prev_code_position = code_position = 0;
        this->return_value = &return_value;
        runtime_functions = vm->allocate<RuntimeFunctions>(0, std::string_view("main-code-block"), std::string_view("main-code-block"));
        runtime_function = vm->allocate<RuntimeFunction>(0, oc, std::string_view("main-code-block"), std::string_view("main-code-block"), RuntimeFunction::ScriptFunction{});
        initialize_execute_function(return_value, vm, runtime_functions, runtime_function, std::nullopt, std::span<OwcaValue>{});
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
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ExecutionFrame::ForState &e) {

    }
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ExecutionFrame::TryCatchState &e) {

    }
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ExecutionFrame::WithState &e) {

    }
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ExecutionFrame::States &e) {
        visit_variant(e, [&](const auto &e) {
            gc_mark_value(vm, generation_gc, e);
        });
    }
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ExecutionFrame &e)
    {
        gc_mark_value(vm, generation_gc, e.runtime_function);
        gc_mark_value(vm, generation_gc, e.runtime_functions);
        gc_mark_value(vm, generation_gc, e.values);
        gc_mark_value(vm, generation_gc, e.temporaries);
        gc_mark_value(vm, generation_gc, e.states);
    }
}