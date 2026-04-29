#include "stdafx.h"
#include "runtime_function.h"
#include "vm.h"

namespace OwcaScript::Internal {
	RuntimeFunction::RuntimeFunction(OwcaCode code, std::string_view name, std::string_view full_name, std::variant<ScriptFunction, NativeFunction, NativeGenerator> data, bool is_method, bool is_generator) :
		code(std::move(code)), name(name), full_name(full_name), data(std::move(data)), is_method(is_method), is_generator(is_generator) {}


	std::string_view RuntimeFunction::type() const {
		return "function set";
	}
	std::string RuntimeFunction::to_string() const {
		return std::format("function set {}", name);
	}
	void RuntimeFunction::gc_mark(OwcaVM vm, GenerationGC generation_gc) const {
		visit([&](const ScriptFunction& s) {
			gc_mark_value(vm, generation_gc, s.values_from_parents);
			},
			[](const auto&) {});
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
}