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
	void RuntimeFunction::gc_mark(GenerationGC generation_gc) const {
		gc_mark_value(generation_gc, owning_namespace);
	}

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

	void RuntimeFunctionScript::gc_mark(GenerationGC generation_gc) const {
	    gc_mark_value(generation_gc, values_from_parents);
	}

	void RuntimeFunctionScriptFunction::gc_mark(GenerationGC generation_gc) const {
	    RuntimeFunctionScript::gc_mark(generation_gc);
	}

	OwcaValue RuntimeFunctionScriptFunction::call(Executor &e) {
        auto locals = e.get_current_unused_locals_ptr();
        assert(locals.local_values_ptr + max_values <= e.values_vector_span().data() + e.values_vector_span().size());
		auto tpk = Executor::TopPtrsKeeper{ e, max_values };
		return e.run_script_code(this, locals, param_count, true);
	}

	void RuntimeFunctionScriptGenerator::gc_mark(GenerationGC generation_gc) const {
	    RuntimeFunctionScript::gc_mark(generation_gc);
	}

	OwcaValue RuntimeFunctionScriptGenerator::call(Executor &e) {
        auto locals_ptr = e.get_current_unused_locals_ptr();
        std::vector<OwcaValue> values_vec(max_values);
        for(auto i = 0u; i < param_count; ++i) {
            values_vec[i] = locals_ptr[i];
        }

        assert(copy_from_parents.size() == values_from_parents.size());

        auto values_span = std::span{ values_vec.data(), values_vec.size() };
        auto iter = Internal::current_vm().allocate<Iterator>(0, this, values_span);
        iter->generator = e.run_script_generator(iter, this, std::move(values_vec), entry_point);
        return OwcaIterator{ iter };
	}

	OwcaValue RuntimeFunctionNativeFunction::call(Executor &e) {
        auto locals = e.get_current_unused_locals_ptr();
        assert(locals.local_values_ptr + max_values <= e.values_vector_span().data() + e.values_vector_span().size());
        auto est = Executor::StackTraceState{ e, this, CodePosition{} };
		auto tpk = Executor::TopPtrsKeeper{ e, max_values };
        return function(std::span{ locals.local_values_ptr, param_count + 1u});
	}

    Generator RuntimeFunctionNativeGenerator::run_native_generator(Executor &e, Iterator *iter_object, Generator generator_object) {
        while(true) {
            std::optional<OwcaValue> val;
            {
                auto est = Executor::StackTraceState{ e, this, CodePosition{} };
				auto tpk = Executor::TopPtrsKeeper{ e, max_values };
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

	OwcaValue RuntimeFunctionNativeGenerator::call(Executor &e) {
        auto locals = e.get_current_unused_locals_ptr();
        assert(locals.local_values_ptr + max_values <= e.values_vector_span().data() + e.values_vector_span().size());
		Generator generator_object = this->generator(std::span{ locals.local_values_ptr, param_count + 1u });
        auto iter = Internal::current_vm().allocate<Iterator>(0, this, std::span<OwcaValue>{});
        iter->generator = run_native_generator(e, iter, std::move(generator_object));
        return OwcaIterator{ iter };
	}

    OwcaValue RuntimeFunctions::bound_function_self_object() { return OwcaFunctions{ this }; }
}
