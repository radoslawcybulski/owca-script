#ifndef RC_OWCA_SCRIPT_OBJECT_H
#define RC_OWCA_SCRIPT_OBJECT_H

#include "stdafx.h"
#include "allocation_base.h"
#include "identifier_index.h"
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

			std::unordered_map<IdentifierIndex, std::variant<Class*, RuntimeFunctions*>> values;
			const std::string_view name, full_name;
			OwcaCode code;
			Line fileline;
			std::unordered_set<Class*> all_base_classes;
			std::vector<Class*> base_classes;
			std::vector<Class*> lookup_order;
			std::vector<RuntimeFunction*> runtime_functions;
			std::vector<IdentifierIndex> runtime_variables;
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
			void initialize_add_variable(IdentifierIndex name);
			void initialize_set_all_variables();
			void finalize_initializing();
			char* native_storage_ptr(Object *) const;
			const char* native_storage_ptr(const Object *) const;

			Class(Line line, std::string_view type, std::string_view full_name, OwcaCode code);

            OwcaValue bound_function_self_object() override;
		};

		struct Object : public AllocationBase {
			static constexpr const Kind object_kind = Kind::User;

			std::unordered_map<IdentifierIndex, OwcaValue> values;
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

            OwcaValue bound_function_self_object() override;
		};

		void gc_mark_value(GenerationGC ggc, const AllocationBase* ptr);
	}
}

#endif
