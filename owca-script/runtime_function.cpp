#include "owca-script/generator.h"
#include "stdafx.h"
#include "runtime_function.h"
#include "vm.h"
#include "iterator.h"

namespace OwcaScript::Internal {
	std::string_view RuntimeFunction::type() const {
		return "function set";
	}
	std::string RuntimeFunction::to_string() const {
		return std::format("function set {}", name);
	}
	void RuntimeFunction::gc_mark(OwcaVM vm, GenerationGC generation_gc) const {}

	unsigned int RuntimeFunction::line(ExecuteBufferReader::Position pos) const {
		return code.get_line_by_position(pos - 1).line;
	}

	std::string_view RuntimeFunctions::type() const {
		return "function set";
	}
	std::string RuntimeFunctions::to_string() const {
		return std::format("function set {}", name);
	}
	void RuntimeFunctions::gc_mark(OwcaVM vm, GenerationGC generation_gc) const {
		for (auto& it : functions) {
			if (it)
				gc_mark_value(vm, generation_gc, it);
		}
	}

	void BoundFunctionSelfObject::gc_mark(OwcaVM vm, GenerationGC generation_gc) const
	{
	}

	void RuntimeFunctionScriptFunction::gc_mark(OwcaVM vm, GenerationGC generation_gc) const {
		gc_mark_value(vm, generation_gc, values_from_parents);
	}

	OwcaValue RuntimeFunctionScriptFunction::call(Executor &e, Executor::TemporariesPtr temporary_ptr, Executor::StatesTypePtr states_ptr) {
		return e.run_script_code(this, temporary_ptr, states_ptr, param_count, true);
	}

	void RuntimeFunctionScriptGenerator::gc_mark(OwcaVM vm, GenerationGC generation_gc) const {
		gc_mark_value(vm, generation_gc, values_from_parents);
	}

	OwcaValue RuntimeFunctionScriptGenerator::call(Executor &e, Executor::TemporariesPtr temporary_ptr, Executor::StatesTypePtr states_ptr) {
        auto locals_ptr = temporary_ptr.locals(param_count);
        assert(locals_ptr.local_values_ptr + max_values + max_temporaries <= e.values_vector_span().data() + e.values_vector_span().size());

        std::vector<OwcaValue> values_vec(max_values + max_temporaries);
        std::vector<Executor::StatesType> states_vec(max_states);
        for(auto i = 0u; i < param_count; ++i) {
            values_vec[i] = locals_ptr[i];
        }

        assert(copy_from_parents.size() == values_from_parents.size());

        for (auto i = 0u; i < copy_from_parents.size(); ++i) {
            values_vec[copy_from_parents[i].index_in_child] = values_from_parents[i];
        }

        auto values_span = std::span{ values_vec.data(), values_vec.size() };
        auto states_span = std::span{ states_vec.data(), states_vec.size() };
        auto iter = vm->allocate<Iterator>(0, this, values_span, states_span);
        iter->generator = e.run_script_generator(iter, this, std::move(values_vec), std::move(states_vec), entry_point);
        return OwcaIterator{ iter };
	}

	OwcaValue RuntimeFunctionNativeFunction::call(Executor &e, Executor::TemporariesPtr temporary_ptr, Executor::StatesTypePtr states_ptr) {
        auto locals_ptr = temporary_ptr.locals(param_count);
        temporary_ptr = temporary_ptr + max_values - param_count;

        assert(locals_ptr.local_values_ptr + max_values + max_temporaries <= e.values_vector_span().data() + e.values_vector_span().size());
        auto est = Executor::StackTraceState{ e, this, {} };
        e.update_current_top_ptrs(temporary_ptr + max_temporaries, states_ptr);
        return function(vm, std::span{ locals_ptr.local_values_ptr, param_count });
	}

    Generator RuntimeFunctionNativeGenerator::run_native_generator(Executor &e, Iterator *iter_object, Generator generator_object) {
        while(true) {
            std::optional<OwcaValue> val;
            {
                auto est = Executor::StackTraceState{ e, this, {} };
                val = generator_object.next();
            }
            iter_object->first_time = false;
            if (val.has_value()) {
                co_yield *val;
            }
            else {
                break;
            }
        }
    }

	OwcaValue RuntimeFunctionNativeGenerator::call(Executor &e, Executor::TemporariesPtr temporary_ptr, Executor::StatesTypePtr states_ptr) {
        auto locals_ptr = temporary_ptr.locals(param_count);
        assert(locals_ptr.local_values_ptr + max_values + max_temporaries <= e.values_vector_span().data() + e.values_vector_span().size());
		Generator generator_object = this->generator(vm, std::span{ locals_ptr.local_values_ptr, param_count });
        auto iter = vm->allocate<Iterator>(0, this, std::span<OwcaValue>{}, std::span<Executor::StatesType>{});
        iter->generator = run_native_generator(e, iter, std::move(generator_object));
        return OwcaIterator{ iter };
	}
}