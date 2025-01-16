#include "stdafx.h"
#include "vm.h"
#include "owca_value.h"
#include "runtime_function.h"
#include "owca_code.h"
#include "code_buffer.h"
#include "owca_vm.h"
#include "owca_value.h"
#include "flow_control.h"
#include "dictionary.h"
#include "object.h"

namespace OwcaScript::Internal {
	VM::VM() {
		root_allocated_memory.prev = root_allocated_memory.next = &root_allocated_memory;
	}
	VM::~VM() {
		run_gc();
	}

	VM& VM::get(const OwcaVM& v)
	{
		return *v.vm;
	}

	void VM::throw_division_by_zero() const
	{
		assert(false);
	}

	void VM::throw_mod_division_by_zero() const
	{
		assert(false);
	}

	void VM::throw_cant_convert_to_float(std::string_view type) const
	{
		assert(false);
		//throw OwcaException{ std::format("can't convert value of type `{}` to floating point", type()) };
	}

	void VM::throw_cant_convert_to_integer(OwcaFloatInternal val) const
	{
		assert(false);
		//throw OwcaException{ std::format("can't convert value `{}` to integer - the conversion would loose data", f) };
	}

	void VM::throw_cant_convert_to_integer(std::string_view type) const
	{
		assert(false);
		// throw OwcaException{ std::format("can't convert value of type `{}` to integer", type()) };
	}

	void VM::throw_not_a_number(std::string_view type) const
	{
		assert(false);
	}

	void VM::throw_cant_compare(AstExprCompare::Kind kind, std::string_view left, std::string_view right) const
	{
		assert(false);
	}

	void VM::throw_index_out_of_range(std::string msg) const
	{
		assert(false);
	}

	void VM::throw_value_not_indexable(std::string_view type, std::string_view key_type) const
	{
		assert(false);
	}

	void VM::throw_missing_member(std::string_view type, std::string_view ident) const
	{
		assert(false);
	}

	void VM::throw_cant_call(std::string_view msg) const
	{
		assert(false);
	}

	void VM::throw_not_callable(std::string_view type) const
	{
		assert(false);
	}
	
	void VM::throw_not_callable_wrong_number_of_params(std::string_view type, unsigned int) const
	{
		assert(false);
	}

	void VM::throw_wrong_type(std::string_view type, std::string_view expected) const
	{
		assert(false);
	}

	void VM::throw_unsupported_operation_2(std::string_view oper, std::string_view left, std::string_view right) const
	{
		assert(false);
	}

	void VM::throw_invalid_operand_for_mul_string(std::string_view val) const
	{
		assert(false);
	}

	void VM::throw_missing_key(std::string_view key) const
	{
		assert(false);
	}

	void VM::throw_not_hashable(std::string_view type) const
	{
		assert(false);
	}

	void VM::value_cant_have_fields(std::string_view type) const
	{
		assert(false);
	}

	OwcaValue VM::member(const OwcaValue& val, const std::string& key)
	{
		if (val.kind() == OwcaValueKind::Object) {
			auto vm = OwcaVM{ shared_from_this() };
			return std::get<OwcaObject>(val.value_).member(vm, key);
		}
		assert(false);
		return {};
	}

	void VM::member(OwcaValue& val, const std::string& key, OwcaValue value)
	{
		if (val.kind() != OwcaValueKind::Object) {
			value_cant_have_fields(val.type());
		}
		std::get<OwcaObject>(val.value_).member(key, std::move(value));
	}

	void VM::update_execution_line(Line line)
	{
		stacktrace.back().line = line;
	}

	VM::PopStack VM::prepare_exec(RuntimeFunctions* runtime_functions, unsigned int index, bool has_self_value)
	{
		auto it = runtime_functions->functions.find(index + (has_self_value ? 1 : 0));
		if (it == runtime_functions->functions.end()) {
			it = runtime_functions->functions.find(index);
			if (it == runtime_functions->functions.end() || it->second->is_method) {
				auto tmp = std::string{ "function " };
				tmp += runtime_functions->name;
				throw_not_callable_wrong_number_of_params(std::move(tmp), index);
			}
		}

		stacktrace.push_back({ it->second->fileline });
		auto& s = stacktrace.back();
		s.runtime_functions = runtime_functions;
		s.runtime_function = it->second;
		s.runtime_function->visit(
			[&](const RuntimeFunction::NativeFunction& nf) {
				s.values.resize(nf.parameter_names.size());
			},
			[&](const RuntimeFunction::ScriptFunction& sf) {
				s.values.resize(sf.identifier_names.size());
				assert(sf.copy_from_parents.size() == sf.values_from_parents.size());

				for (auto i = 0u; i < sf.copy_from_parents.size(); ++i) {
					s.values[sf.copy_from_parents[i].index_in_child] = sf.values_from_parents[i];
				}
			});

		return PopStack{ this };
	}

	OwcaValue VM::execute()
	{
		auto& s = stacktrace.back();
		auto vm = OwcaVM{ shared_from_this() };

		return s.runtime_function->visit(
			[&](const RuntimeFunction::NativeFunction& nf) -> OwcaValue {
				auto vm = OwcaVM{ shared_from_this() };

				return (*nf.function)(vm, std::span{ s.values.begin(), s.values.end() });
			},
			[&](const RuntimeFunction::ScriptFunction& sf) -> OwcaValue {
				try {
					sf.body->execute(vm);
					return {};
				}
				catch (FlowControlReturn o) {
					return std::move(o.value);
				}
			});
	}

	OwcaValue VM::execute_code_block(const OwcaCode &oc, const std::unordered_map<std::string, OwcaValue> *values)
	{
		auto vm = OwcaVM{ shared_from_this() };
		OwcaValue val;
		{
			stacktrace.push_back({ oc.code_->root()->line });
			auto pop_stack = PopStack{ this };
			RuntimeFunction::ScriptFunction sf;
			RuntimeFunction rt_temp{ std::move(sf), oc.code_, "", Line{ 0 }, 0, false };
			stacktrace.back().runtime_function = &rt_temp;
			val = oc.code_->root()->execute(vm);
		}
		assert(val.kind() == OwcaValueKind::Functions);
		auto functions = val.as_functions(vm);
		auto pop_stack = prepare_exec(functions.functions, 0, false);
		auto& s = stacktrace.back();
		s.runtime_function->visit(
			[&](RuntimeFunction::ScriptFunction& sf) -> void {
				std::unordered_map<std::string_view, unsigned int> value_index_map;
				for (auto i = 0u; i < sf.identifier_names.size(); ++i) {
					value_index_map[sf.identifier_names[i]] = i;
				}

				if (values) {
					for (auto& it : *values) {
						auto it2 = value_index_map.find(it.first);
						if (it2 != value_index_map.end()) {
							set_identifier(it2->second, it.second);
						}
					}
				}
			},
			[&](RuntimeFunction::NativeFunction&) -> void {
				assert(false);
			}
		);

		try {
			return execute();
		}
		catch (FlowControlException) {
			assert(false);
			return {};
		}
	}
	OwcaValue VM::execute_call(const OwcaValue &func, std::span<OwcaValue> arguments)
	{
		return func.visit(
			[&](const OwcaFunctions& of) -> OwcaValue {
				auto vm = OwcaVM{ shared_from_this() };
				auto runtime_functions = func.as_functions(vm).functions;
				auto pop_stack = prepare_exec(runtime_functions, (unsigned int)arguments.size(), of.self_object != nullptr);
				auto& s = stacktrace.back();
				bool self = s.runtime_function->is_method;
				if (self) {
					if (of.self_object) {
						s.values[0] = OwcaObject{ of.self_object };
					}
					else {
						throw_cant_call(std::format("can't call {} - missing self value", func.to_string()));
					}
				}
				for (auto i = (self ? 1u : 0u); i < s.runtime_function->param_count; ++i) {
					s.values[i] = arguments[i - (self ? 1u : 0u)];
				}

				return execute();
			},
			[&](const OwcaClass& oc) -> OwcaValue {
				auto vm = OwcaVM{ shared_from_this() };
				auto cls = func.as_class(vm).object;

				auto obj = allocate<Object>(cls->native_storage_total, cls);

				auto it = cls->values.find("__init__");
				if (it == cls->values.end()) {
					if (!arguments.empty()) {
						throw_cant_call(std::format("type {} has no __init__ function defined - expected constructor's call with no parameters, instead got {}", cls->type_, arguments.size()));
					}
				}
				else {
					auto of = it->second.as_functions(vm);
					auto it2 = of.functions->functions.find((unsigned int)(1 + arguments.size()));
					if (it2 == of.functions->functions.end()) {
						throw_cant_call(std::format("type {} has __init__ function, but not one with {} parameters", cls->type_, 1 + arguments.size()));
					}
					else {
						auto of2 = OwcaFunctions{ of.functions, obj };
						execute_call(of2, arguments);
					}
				}

				return OwcaObject{ obj };
			},
			[&](const auto&) -> OwcaValue {
				throw_not_callable(func.type());
				assert(false);
				return {};
			}
		);
	}
	OwcaValue VM::create_array(std::vector<OwcaValue> arguments)
	{
		assert(false);
		return {};
	}
	OwcaValue VM::create_map(std::vector<OwcaValue> arguments)
	{
		auto ds = allocate<DictionaryShared>(0);
		auto vm = OwcaVM{ shared_from_this() };
		for (auto i = 0u; i < arguments.size(); i += 2) {
			ds->dict.write(vm, arguments[i], std::move(arguments[i + 1]));
		}
		return OwcaValue{ OwcaMap{ ds } };
	}
	OwcaValue VM::create_set(std::vector<OwcaValue> arguments)
	{
		assert(false);
		return {};
	}
	OwcaValue VM::get_identifier(unsigned int index)
	{
		auto& s = stacktrace.back();
		assert(index < s.values.size());
		return s.values[index];
	}
	void VM::set_identifier(unsigned int index, OwcaValue value)
	{
		auto& s = stacktrace.back();
		assert(index < s.values.size());
		s.values[index] = std::move(value);
	}
	std::shared_ptr<CodeBuffer> VM::currently_running_code() const
	{
		auto& s = stacktrace.back();
		return s.runtime_function->code;
	}
	bool VM::compare_values(const OwcaValue& left, const OwcaValue& right)
	{
		auto vm = OwcaVM{ shared_from_this() };
		return AstExprCompare::compare_equal(vm, left, right);
	}
	
	Class* VM::ensure_is_class(const OwcaValue&v)
	{
		auto vm = OwcaVM{ shared_from_this() };
		return v.as_class(vm).object;
	}

	Object* VM::ensure_is_object(const OwcaValue&v)
	{
		auto vm = OwcaVM{ shared_from_this() };
		return v.as_object(vm).object;
	}

	size_t VM::calculate_hash(const OwcaValue& value) {
		auto vm = OwcaVM{ shared_from_this() };
		static auto calc_hash = [](const auto& q) {
			return std::hash<std::remove_cvref_t<decltype(q)>>()(q);
			};
		return value.visit(
			[](const OwcaEmpty& o) -> size_t { return 3; },
			[](const OwcaRange& o) -> size_t { 
				return calc_hash(o.lower().internal_value()) * 1009 + calc_hash(o.upper().internal_value()) + 4;
			},
			[](const OwcaInt& o) -> size_t { return calc_hash(o.internal_value()) * 1013 + 5; },
			[](const OwcaFloat& o) -> size_t { return calc_hash(o.internal_value()) * 1019 + 6; },
			[](const OwcaBool& o) -> size_t { return (o.internal_value() ? 1 : 0) * 1021 + 7; },
			[](const OwcaString& o) -> size_t { return calc_hash(o.internal_value()) * 1031 + 8; },
			[](const OwcaFunctions& o) -> size_t { return calc_hash(o.name()) * 1033 + 9; },
			[&](const OwcaMap& o) -> size_t {
				throw_not_hashable(value.type());
				assert(false);
				return 0;
			},
			[&](const OwcaClass& o) -> size_t {
				return std::hash<void*>()(o.object) * 1009;
			},
			[&](const OwcaObject& o) -> size_t {
				throw_not_hashable(value.type());
				assert(false);
				return 0;
			}
		);
	}

	void VM::run_gc() {
		assert(stacktrace.empty());

		auto ggc = GenerationGC{ ++generation_gc };
		// mark
		for (auto& s : stacktrace) {
			gc_mark(s.runtime_function, ggc);
			gc_mark(s.runtime_functions, ggc);
			gc_mark(s.values, ggc);
		}
		
		// sweep
		AllocationBase *valid = &root_allocated_memory;
		while (valid->next != &root_allocated_memory) {
			auto obj = valid->next;
			if (obj->last_gc_mark != ggc) {
				valid->next = obj->next;
				obj->next->prev = valid;
				delete obj;
			}
			else {
				valid = valid->next;
			}
		}
	}

	void VM::gc_mark(AllocationBase* ptr, GenerationGC ggc)
	{
		if (ptr->last_gc_mark != ggc) {
			ptr->last_gc_mark = ggc;
			ptr->gc_mark(*this, ggc);
		}
	}

	void VM::gc_mark(const OwcaValue& o, GenerationGC ggc)
	{
		o.visit(
			[&](const OwcaFunctions& s) {
				gc_mark(s.functions, ggc);
				if (s.self_object)
					gc_mark(s.self_object, ggc);
			},
			[&](const OwcaMap& s) {
				gc_mark(s.dictionary, ggc);
			},
			[&](const OwcaClass& s) {
				gc_mark(s.object, ggc);
			},
			[&](const OwcaObject& s) {
				gc_mark(s.object, ggc);
			},
			[&](const auto&) {}
			);
	}
	void VM::gc_mark(const std::vector<OwcaValue>& o, GenerationGC ggc)
	{
		for (auto& q : o) {
			gc_mark(q, ggc);
		}
	}

}