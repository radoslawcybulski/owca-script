#include "stdafx.h"
#include "runtime_function.h"
#include "vm.h"

namespace OwcaScript::Internal {
	std::string_view RuntimeFunctions::type() const {
		return "function set";
	}
	std::string RuntimeFunctions::to_string() const {
		return std::format("function set {}", name);
	}
	void RuntimeFunctions::gc_mark(VM &vm, GenerationGC generation_gc) {
		for (auto& it : functions) {
			auto& rt = it.second;
			rt.visit([&](RuntimeFunction::ScriptFunction& s) {
				vm.gc_mark(s.values_from_parents, generation_gc);
				},
				[](const auto&) {});
		}
	}
}