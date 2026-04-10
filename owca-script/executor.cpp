#include "stdafx.h"
#include "executor.h"
#include "vm.h"
#include "object.h"
#include "runtime_function.h"
#include "iterator.h"

namespace OwcaScript::Internal {
    Executor::Executor(VM *vm) : vm(vm), stack_top_level_index(vm->current_stack_trace_index) {
    }

    void Executor::run(OwcaMap *dict_output) {
        run_impl();
        if (dict_output) {
            auto &old_frame = vm->just_executed_executing_frame();
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
        auto &frame = vm->push_new_frame();
        frame.initialize_code_block(return_value, oc);
    }
    void Executor::prepare_execute_main_function(OwcaValue &return_value, RuntimeFunctions* runtime_functions, std::optional<OwcaMap> arguments) {
        auto &frame = vm->push_new_frame();
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
        auto &frame = vm->push_new_frame();
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

        auto &frame = vm->push_new_frame();
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