#include "stdafx.h"
#include "object.h"
#include "vm.h"
#include "owca_vm.h"
#include "runtime_function.h"

namespace OwcaScript::Internal {
	Class::Class(Line line, std::string_view name, std::string_view full_name, std::shared_ptr<CodeBuffer> code, size_t base_class_count) : fileline(line), name(name), full_name(full_name), code(std::move(code)) {
		base_classes.reserve(base_class_count);
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

	void Class::initialize_add_base_class(OwcaVM vm, const OwcaValue &b)
	{
		auto c = b.as_class(vm).internal_value();
		base_classes.push_back(c);
	}
	void Class::initialize_add_function(OwcaVM vm, const OwcaValue &f)
	{
		auto fnc = f.as_functions(vm);
		assert(fnc.internal_value()->functions.size() == 1);
		for (auto it2 : fnc.internal_value()->functions) {
			runtime_functions.push_back(it2.second);
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
			for (auto q : base_classes)
				fill_lookup_order(q);
		};
		fill_lookup_order(this);
		for (auto i = lookup_order.size(); i > 0; --i) {
			for (auto f : lookup_order[i - 1]->runtime_functions) {
				auto name = f->name;

				auto it = values.insert({ std::string{ name }, {} });
				if (it.second || it.first->second.kind() != OwcaValueKind::Functions) {
					auto rf = VM::get(vm).allocate<RuntimeFunctions>(0, name, f->full_name);
					it.first->second = OwcaFunctions{ rf };
				}
				auto dst_fnc = it.first->second.as_functions(vm);
				dst_fnc.internal_value()->functions[f->param_count] = f;
			}
		}
	}

	std::span<char> Object::native_storage(ClassToken cls)
	{
		auto c = (Class*)cls.value();
		auto it = type_->native_storage_pointers.find(c);
		if (it == type_->native_storage_pointers.end()) return {};
		auto p = type_->native_storage_ptr(this);
		return { p + it->second.first, it->second.second };
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

	void Object::gc_mark(VM& vm, GenerationGC generation_gc)
	{
		vm.gc_mark(type_, generation_gc);
		for (auto& it : values)
			vm.gc_mark(it.second, generation_gc);
	}
}