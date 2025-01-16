#include "stdafx.h"
#include "object.h"
#include "vm.h"
#include "owca_vm.h"
#include "runtime_function.h"

namespace OwcaScript::Internal {
	Class::Class(Line line) : fileline(line) {}
	Object::Object(Class* type) : type_(type) {}

	std::string Class::to_string() const
	{ 
		std::string tmp = "class ";
		tmp += type_;
		return tmp;
	}

	void Class::gc_mark(VM& vm, GenerationGC generation_gc)
	{
		for (auto& it : values) {
			vm.gc_mark(it.second, generation_gc);
		}
		for (auto c : base_classes) {
			vm.gc_mark(c, generation_gc);
		}
		for (auto c : runtime_functions) {
			vm.gc_mark(c, generation_gc);
		}
	}

	char* Class::native_storage_ptr(Object *o) const
	{
		return (char*)o + sizeof(*o);
	}

	Object* Class::allocate(OwcaVM& vm)
	{
		auto o = VM::get(vm).allocate<Object>(native_storage_total, this);

		for (auto it : native_storage_pointers) {
			auto p = native_storage_ptr(o) + it.second.first;
			auto size = it.second.second;
			auto cls = it.first;
			assert(cls->native);
			cls->native->initialize_storage(p, size);
		}

		return o;
	}

	std::string_view Object::type() const
	{
		return type_->type_;
	}

	std::string Object::to_string() const
	{
		std::string tmp = "object of type ";
		tmp += type();
		return tmp;
	}

	void Object::gc_mark(VM& vm, GenerationGC generation_gc)
	{
		vm.gc_mark(type_, generation_gc);
		for (auto& it : values)
			vm.gc_mark(it.second, generation_gc);
	}

	std::optional<OwcaValue> Object::lookup(const std::string &key)
	{
		auto it = values.find(key);
		if (it != values.end()) return it->second;

		it = type_->values.find(key);
		if (it != type_->values.end()) return it->second;

		return std::nullopt;
	}
}