#ifndef RC_OWCA_SCRIPT_OBJECT_H
#define RC_OWCA_SCRIPT_OBJECT_H

#include "stdafx.h"
#include "allocation_base.h"
#include "owca_code.h"
#include "line.h"

namespace OwcaScript {
	class OwcaVM;
	class OwcaClass;
	class OwcaFunctions;
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
			std::vector<std::tuple<UserClassTokenPtr, Class*, size_t, size_t>> native_storage_pointers;
			std::shared_ptr<NativeClassInterface> native;
			std::optional<UserClassTokenPtr> native_token;
			size_t native_storage = 0;
			size_t native_storage_total = 0;
			std::function<OwcaValue()> allocator_override;
			bool reload_self = false;
			bool all_variables = false;

			std::string_view type() const override { return "Class"; }
			std::string to_string() const override;
			void gc_mark(GenerationGC generation_gc) const override;

			void initialize_set_native_class_info(UserClassTokenPtr token, size_t sz);
			void initialize_add_base_class(OwcaClass b);
			void initialize_add_function(OwcaFunctions f);
			void initialize_add_variable(std::string_view name);
			void initialize_set_all_variables();
			void finalize_initializing();
			char* native_storage_ptr(Object *) const;
			const char* native_storage_ptr(const Object *) const;

			Class(Line line, std::string_view type, std::string_view full_name, OwcaCode code);
		};

		struct Object : public AllocationBase {
			static constexpr const Kind object_kind = Kind::User;

			std::unordered_map<std::string, OwcaValue, StringHash, StringCmp> values;
			Class* type_;

			Object(Class* type);
			~Object();

			std::string_view type() const override;
			std::string to_string() const override;
			void gc_mark(GenerationGC generation_gc) const override;
			std::span<char> native_storage_raw(UserClassTokenPtr token) {
				for(auto &p : type_->native_storage_pointers) {
					if (std::get<0>(p) == token) {
						return { type_->native_storage_ptr(this) + std::get<2>(p), std::get<3>(p) };
					}
				}
				return { (char*)nullptr, 0 };
			}
		};

		void gc_mark_value(GenerationGC ggc, const AllocationBase* ptr);
	}
}

#endif
