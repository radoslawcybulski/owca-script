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
	void RuntimeFunction::gc_mark(GenerationGC generation_gc) const {}

	unsigned int RuntimeFunction::line(CodePosition pos) const {
		return code.get_line_by_position(pos - 1).line;
	}

	std::string_view RuntimeFunctions::type() const {
		return "function set";
	}
	std::string RuntimeFunctions::to_string() const {
		return std::format("function set {}", name);
	}
	void RuntimeFunctions::gc_mark(GenerationGC generation_gc) const {
		for (auto& it : functions) {
			if (it)
				gc_mark_value(generation_gc, it);
		}
	}

	void BoundFunctionSelfObject::gc_mark(GenerationGC generation_gc) const
	{
	}

	void RuntimeFunctionScriptFunction::gc_mark(GenerationGC generation_gc) const {
		gc_mark_value(generation_gc, values_from_parents);
	}

	OwcaValue RuntimeFunctionScriptFunction::call(Executor &e, TemporariesPtr temporary_ptr) {
		return e.run_script_code(this, globals_ptr, temporary_ptr, param_count, true);
	}

	void RuntimeFunctionScriptGenerator::gc_mark(GenerationGC generation_gc) const {
		gc_mark_value(generation_gc, values_from_parents);
	}

	OwcaValue RuntimeFunctionScriptGenerator::call(Executor &e, TemporariesPtr temporary_ptr) {
        auto locals_ptr = temporary_ptr.locals(param_count);
        assert(locals_ptr.local_values_ptr + max_values + max_temporaries <= e.values_vector_span().data() + e.values_vector_span().size());

        std::vector<OwcaValue> values_vec(max_values + max_temporaries);
        for(auto i = 0u; i < param_count; ++i) {
            values_vec[i] = locals_ptr[i];
        }

        assert(copy_from_parents.size() == values_from_parents.size());

        for (auto i = 0u; i < copy_from_parents.size(); ++i) {
            values_vec[copy_from_parents[i].index_in_child] = values_from_parents[i];
        }

        auto values_span = std::span{ values_vec.data(), values_vec.size() };
        auto iter = Internal::current_vm().allocate<Iterator>(0, this, values_span);
        iter->generator = e.run_script_generator(iter, this, globals_ptr, std::move(values_vec), entry_point);
        return OwcaIterator{ iter };
	}

	OwcaValue RuntimeFunctionNativeFunction::call(Executor &e, TemporariesPtr temporary_ptr) {
        auto locals_ptr = temporary_ptr.locals(param_count + 1);
        temporary_ptr = temporary_ptr + max_values - param_count;

        assert(locals_ptr.local_values_ptr + max_values + max_temporaries <= e.values_vector_span().data() + e.values_vector_span().size());
        auto est = Executor::StackTraceState{ e, this, CodePosition{} };
        e.update_current_top_ptrs(temporary_ptr + max_temporaries);
        return function(std::span{ locals_ptr.local_values_ptr, param_count + 1u});
	}

    Generator RuntimeFunctionNativeGenerator::run_native_generator(Executor &e, Iterator *iter_object, Generator generator_object) {
        while(true) {
            std::optional<OwcaValue> val;
            {
                auto est = Executor::StackTraceState{ e, this, CodePosition{} };
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

	OwcaValue RuntimeFunctionNativeGenerator::call(Executor &e, TemporariesPtr temporary_ptr) {
        auto locals_ptr = temporary_ptr.locals(param_count + 1);
        assert(locals_ptr.local_values_ptr + max_values + max_temporaries <= e.values_vector_span().data() + e.values_vector_span().size());
		Generator generator_object = this->generator(std::span{ locals_ptr.local_values_ptr, param_count + 1u });
        auto iter = Internal::current_vm().allocate<Iterator>(0, this, std::span<OwcaValue>{});
        iter->generator = run_native_generator(e, iter, std::move(generator_object));
        return OwcaIterator{ iter };
	}

    OwcaValue RuntimeFunctions::bound_function_self_object() { return OwcaFunctions{ this }; }
}