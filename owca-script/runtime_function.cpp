#include "stdafx.h"
#include "runtime_function.h"
#include "vm.h"

namespace OwcaScript::Internal {
	RuntimeFunction::ScriptFunction::ScriptFunction(OwcaCode code, std::uint32_t entry_point, bool is_generator) : code(std::move(code)), entry_point(entry_point), is_generator(is_generator) {}
	RuntimeFunction::ScriptFunction::~ScriptFunction() = default;

	RuntimeFunction::RuntimeFunction(OwcaCode code, std::string_view name, std::string_view full_name, unsigned int param_count, bool is_method, std::variant<ScriptFunction, NativeFunction, NativeGenerator> data) :
		code(std::move(code)), name(name), full_name(full_name), param_count(param_count), is_method(is_method), data(std::move(data)) {}


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
	void RuntimeFunctions::gc_mark(OwcaVM vm, GenerationGC generation_gc) const {
		for (auto& it : functions) {
			gc_mark_value(vm, generation_gc, it.second);
		}
	}

	void BoundFunctionSelfObject::gc_mark(OwcaVM vm, GenerationGC generation_gc) const
	{
	}
}