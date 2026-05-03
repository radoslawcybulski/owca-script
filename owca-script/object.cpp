#include "stdafx.h"
#include "object.h"
#include "vm.h"
#include "owca_vm.h"
#include "runtime_function.h"

namespace OwcaScript::Internal {
	Class::Class(Line line, std::string_view name, std::string_view full_name, OwcaCode code) : fileline(line), name(name), full_name(full_name), code(std::move(code)) {
	}
	Object::Object(Class* type) : type_(type) {
		for (auto it : type_->native_storage_pointers) {
			auto p = type_->native_storage_ptr(this) + it.second.first;
			auto size = it.second.second;
			auto cls = it.first;
			assert(cls->native);
			cls->native->initialize_storage(p, size);
		}
	}
	Object::~Object() {
		for (auto it : type_->native_storage_pointers) {
			auto p = type_->native_storage_ptr(this) + it.second.first;
			auto size = it.second.second;
			auto cls = it.first;
			assert(cls->native);
			cls->native->destroy_storage(p, size);
		}
	}

	std::string Class::to_string() const
	{ 
		std::string tmp = "class ";
		tmp += full_name;
		return tmp;
	}

	void Class::gc_mark(OwcaVM vm, GenerationGC generation_gc) const
	{
		for (auto& it : values) {
			visit_variant(it.second, [&](const Class* p) {
				gc_mark_value(vm, generation_gc, p);
			}, [&](const RuntimeFunctions* f) {
				gc_mark_value(vm, generation_gc, f);
			});
		}
		for (auto c : base_classes) {
			gc_mark_value(vm, generation_gc, c);
		}
		for (auto c : runtime_functions) {
			gc_mark_value(vm, generation_gc, c);
		}
	}

	char* Class::native_storage_ptr(Object *o) const
	{
		return (char*)o + sizeof(*o);
	}
	const char* Class::native_storage_ptr(const Object *o) const
	{
		return (const char*)o + sizeof(*o);
	}

	void Class::initialize_add_base_class(OwcaVM vm, OwcaClass b)
	{
		base_classes.push_back(b.internal_value());
	}
	void Class::initialize_add_variable(std::string_view name) {
		runtime_variables.push_back(name);
	}
	void Class::initialize_set_all_variables() {
		all_variables = true;
	}
	void Class::initialize_add_function(OwcaVM vm, OwcaFunctions fnc)
	{
		for(auto i = 0u; i < fnc.internal_value()->functions.size(); ++i) {
			if (fnc.internal_value()->functions[i]) {
				runtime_functions.push_back(fnc.internal_value()->functions[i]);
			}
		}
	}
	void Class::finalize_initializing(OwcaVM vm)
	{
		size_t offset = 0;
		for (auto q : base_classes) {
			for (auto it : q->native_storage_pointers) {
				native_storage_pointers.insert({ it.first, { offset, it.second.second } });
				offset += it.second.second;
				offset = (offset + 15) & ~15;
			}
		}
		native_storage_total = offset;

		std::function<void(Class*)> fill_lookup_order = [&](Class* c) {
			lookup_order.push_back(c);
			for (auto q : c->base_classes)
				fill_lookup_order(q);
		};
		fill_lookup_order(this);
		for (auto i = lookup_order.size(); i > 0; --i) {
			for (auto f : lookup_order[i - 1]->runtime_functions) {
				auto name = f->name;

				auto it = values.insert({ std::string{ name }, {} });
				if (it.second || std::get_if<RuntimeFunctions*>(&it.first->second) == nullptr) {
					auto rf = VM::get(vm).allocate<RuntimeFunctions>(0, name, f->full_name);
					it.first->second = rf;
				}
				auto &dst_fnc = std::get<RuntimeFunctions*>(it.first->second);
				dst_fnc->functions[f->param_count] = f;
			}
			for (auto name: lookup_order[i - 1]->runtime_variables) {
				auto it = values.insert({ std::string{ name }, {} });
				it.first->second = lookup_order[i - 1];
			}
		}
		for(auto q : lookup_order) all_base_classes.insert(q);
	}

	std::span<char> Object::native_storage_raw(ClassToken cls)
	{
		auto c = (Class*)cls.value();
		auto it = type_->native_storage_pointers.find(c);
		if (it == type_->native_storage_pointers.end()) return {};
		return { (char*)this + sizeof(*this) + it->second.first, it->second.second };
	}

	std::string_view Object::type() const
	{
		return type_->full_name;
	}

	std::string Object::to_string() const
	{
		std::string tmp = "object of type ";
		tmp += type();
		return tmp;
	}

	void Object::gc_mark(OwcaVM vm, GenerationGC generation_gc) const
	{
		gc_mark_value(vm, generation_gc, type_);
		for (auto& it : values)
			gc_mark_value(vm, generation_gc, it.second);
		for(auto& it : type_->native_storage_pointers) {
			auto p = type_->native_storage_ptr(this) + it.second.first;
			auto size = it.second.second;
			auto cls = it.first;
			assert(cls->native);
			cls->native->gc_mark_members(p, size, vm, generation_gc);
		}
	}
}