#include "stdafx.h"
#include "runtime_function.h"
#include "vm.h"

namespace OwcaScript::Internal {
	std::string_view RuntimeFunction::type() const {
		return "function set";
	}
	std::string RuntimeFunction::to_string() const {
		return std::format("function set {}", name);
	}
	void RuntimeFunction::gc_mark(OwcaVM vm, GenerationGC generation_gc) {
		visit([&](ScriptFunction& s) {
			VM::get(vm).gc_mark(s.values_from_parents, generation_gc);
			},
			[](const auto&) {});
	}

	bool RuntimeFunction::is_generator() const {
		return visit([&](const ScriptFunction& s) {
			return s.is_generator;
			},
			[](const NativeFunction&) {
				return false;
			},
			[](const NativeGenerator&) {
				return true;
			});
	}

	std::string_view RuntimeFunctions::type() const {
		return "function set";
	}
	std::string RuntimeFunctions::to_string() const {
		return std::format("function set {}", name);
	}
	void RuntimeFunctions::gc_mark(OwcaVM vm, GenerationGC generation_gc) {
		for (auto& it : functions) {
			VM::get(vm).gc_mark(it.second, generation_gc);
		}
	}

	void BoundFunctionSelfObject::gc_mark(OwcaVM vm, GenerationGC generation_gc)
	{

	}
}