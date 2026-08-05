#include "owca-script/identifier_index.h"
#include "stdafx.h"
#include "object.h"
#include "vm.h"
#include "owca_vm.h"
#include "runtime_function.h"

namespace OwcaScript::Internal {
	Class::Class(Line line, std::string_view name, std::string_view full_name, OwcaCode code) : fileline(line), name(name), full_name(full_name), code(std::move(code)) {
	}
	Object::Object(Class* type) : type_(type) {
		auto ptr = type_->native_storage_ptr(this);
		for (auto it : type_->native_storage_pointers) {
			auto p = ptr + std::get<2>(it);
			auto size = std::get<3>(it);
			auto cls = std::get<1>(it);;
			assert(cls->native);
			cls->native->initialize_storage(p, size);
		}
	}
	Object::~Object() {
		auto ptr = type_->native_storage_ptr(this);
		for (auto it : type_->native_storage_pointers) {
			auto p = ptr + std::get<2>(it);
			auto size = std::get<3>(it);
			auto cls = std::get<1>(it);
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

	void Class::gc_mark(GenerationGC generation_gc) const
	{
		values.for_each(
			[&](auto key, auto value) {
				visit_variant(value, [&](const Class* p) {
					gc_mark_value(generation_gc, p);
				}, [&](const RuntimeFunctions* f) {
					gc_mark_value(generation_gc, f);
				});
			}
		);
		for (auto c : base_classes) {
			gc_mark_value(generation_gc, c);
		}
		for (auto c : runtime_functions) {
			gc_mark_value(generation_gc, c);
		}
	}

	char* Class::native_storage_ptr(Object *o) const
	{
		auto p = reinterpret_cast<std::uintptr_t>(o) + sizeof(*o);
		p = (p + 15) & ~15;
		return reinterpret_cast<char*>(p);
	}
	const char* Class::native_storage_ptr(const Object *o) const
	{
		auto p = reinterpret_cast<std::uintptr_t>(o) + sizeof(*o);
		p = (p + 15) & ~15;
		return reinterpret_cast<const char*>(p);
	}

	void Class::initialize_add_base_class(OwcaClass b)
	{
		base_classes.push_back(b.internal_value());
	}
	void Class::initialize_add_variable(IdentifierIndex name) {
		runtime_variables.push_back(name);
	}
	void Class::initialize_set_all_variables() {
		all_variables = true;
	}
	void Class::initialize_add_function(OwcaFunctions fnc)
	{
		for(auto i = 0u; i < fnc.internal_value()->functions.size(); ++i) {
			if (fnc.internal_value()->functions[i]) {
				runtime_functions.push_back(fnc.internal_value()->functions[i]);
			}
		}
	}
	void Class::initialize_set_native_class_info(UserClassTokenPtr token, size_t sz) {
		assert(native_storage_pointers.empty());
		native_storage_pointers.push_back({ token, this, 0, sz });
		native_token = token;
	}
	void Class::finalize_initializing()
	{
		size_t offset = 0;
		if (!native_storage_pointers.empty()) {
			assert(native_storage_pointers.size() == 1);
			offset = std::get<3>(native_storage_pointers[0]);
			offset = (offset + 15) & ~15;
		}
		for (auto q : base_classes) {
			for (auto it : q->native_storage_pointers) {
				bool found = false;
				for(auto &it2 : native_storage_pointers) {
					if (std::get<0>(it) == std::get<0>(it2)) {
						found = true;
						break;
					}
				}
				if (found) continue;
				native_storage_pointers.push_back({
					std::get<0>(it),
					std::get<1>(it),
					offset,
					std::get<3>(it)
				});
				offset += std::get<3>(it);
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
				auto [ inserted_ptr, is_inserted ] = values.emplace(f->name_index, {});
				if (is_inserted || std::get_if<RuntimeFunctions*>(inserted_ptr) == nullptr) {
					auto rf = Internal::current_vm().allocate<RuntimeFunctions>(0, f->name_index, f->name, f->full_name);
					*inserted_ptr = rf;
				}
				auto dst_fnc = std::get<RuntimeFunctions*>(*inserted_ptr);
				dst_fnc->functions[f->param_count] = f;
			}
			for (auto name: lookup_order[i - 1]->runtime_variables) {
				auto [ inserted_ptr, is_inserted ] = values.emplace(name, {});
				*inserted_ptr = lookup_order[i - 1];
			}
		}
		for(auto q : lookup_order) all_base_classes.insert(q);
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

	void Object::gc_mark(GenerationGC generation_gc) const
	{
		gc_mark_value(generation_gc, type_);
		values.for_each([&](auto key, auto value) {
			gc_mark_value(generation_gc, value);
		});
		auto ptr = type_->native_storage_ptr(this);
		for(auto it : type_->native_storage_pointers) {
			auto p = ptr + std::get<2>(it);
			auto size = std::get<3>(it);
			auto cls = std::get<1>(it);
			assert(cls->native);
			cls->native->gc_mark_members(p, size, generation_gc);
		}
	}

    OwcaValue Class::bound_function_self_object() { return OwcaClass{ this }; }
    OwcaValue Object::bound_function_self_object() { return OwcaObject{ this }; }
}
