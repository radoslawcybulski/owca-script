#ifndef RC_OWCA_SCRIPT_OBJECT_H
#define RC_OWCA_SCRIPT_OBJECT_H

#include "stdafx.h"
#include "allocation_base.h"
#include "owca_code.h"
#include "line.h"

namespace OwcaScript {
	class OwcaVM;
	class NativeClassInterface;

	namespace Internal {
		struct RuntimeFunction;
		struct RuntimeFunctions;
		struct Object;

		struct Class : public AllocationBase {
			static constexpr const Kind object_kind = Kind::Class;

			std::unordered_map<std::string, std::variant<Class*, RuntimeFunctions*>, StringHash, StringCmp> values;
			const std::string_view name, full_name;
			OwcaCode code;
			Line fileline;
			std::unordered_set<Class*> all_base_classes;
			std::vector<Class*> base_classes;
			std::vector<Class*> lookup_order;
			std::vector<RuntimeFunction*> runtime_functions;
			std::vector<std::string_view> runtime_variables;
			std::unordered_map<std::string_view, Class*> member_names;
			std::unordered_map<Class*, std::pair<size_t, size_t>> native_storage_pointers;
			std::shared_ptr<NativeClassInterface> native;
			size_t native_storage = 0;
			size_t native_storage_total = 0;
			std::function<OwcaValue()> allocator_override;
			bool reload_self = false;
			bool all_variables = false;

			std::string_view type() const override { return "Class"; }
			std::string to_string() const override;
			void gc_mark(OwcaVM vm, GenerationGC generation_gc) const override;

			void initialize_add_base_class(OwcaVM vm, OwcaValue b);
			void initialize_add_function(OwcaVM vm, OwcaValue f);
			void initialize_add_variable(std::string_view name);
			void initialize_set_all_variables();
			void finalize_initializing(OwcaVM vm);
			char* native_storage_ptr(Object *) const;
			const char* native_storage_ptr(const Object *) const;

			Class(Line line, std::string_view type, std::string_view full_name, OwcaCode code, size_t base_class_count);
		};

		struct Object : public AllocationBase {
			static constexpr const Kind object_kind = Kind::User;

			std::unordered_map<std::string, OwcaValue, StringHash, StringCmp> values;
			Class* type_;

			Object(Class* type);
			~Object();

			std::string_view type() const override;
			std::string to_string() const override;
			void gc_mark(OwcaVM vm, GenerationGC generation_gc) const override;
			std::span<char> native_storage_raw(ClassToken cls);
			template <typename T> T* native_storage(ClassToken cls) {
				auto sp = native_storage_raw(cls);
				if (sp.empty()) return nullptr;
				assert(sp.size() >= sizeof(T));
				return reinterpret_cast<T*>(sp.data());
			}
		};

		void gc_mark_value(OwcaVM vm, GenerationGC ggc, const AllocationBase* ptr);
	}
}

#endif
