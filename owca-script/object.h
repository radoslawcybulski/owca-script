#ifndef RC_OWCA_SCRIPT_OBJECT_H
#define RC_OWCA_SCRIPT_OBJECT_H

#include "stdafx.h"
#include "allocation_base.h"
#include "line.h"
#include "owca_class.h"

namespace OwcaScript {
	class OwcaVM;

	namespace Internal {
		class CodeBuffer;
		struct RuntimeFunction;
		struct Object;

		struct Class : public AllocationBase {
			std::unordered_map<std::string, OwcaValue> values;
			const std::string_view name, full_name;
			std::shared_ptr<CodeBuffer> code;
			Line fileline;
			std::vector<Class*> base_classes;
			std::vector<Class*> lookup_order;
			std::vector<RuntimeFunction*> runtime_functions;
			std::unordered_map<Class*, std::pair<size_t, size_t>> native_storage_pointers;
			std::unique_ptr<OwcaClass::NativeClassInterface> native;
			size_t native_storage = 0;
			size_t native_storage_total = 0;
			std::function<OwcaValue()> allocator_override;

			std::string_view type() const override { return "Class"; }
			std::string to_string() const override;
			void gc_mark(VM& vm, GenerationGC generation_gc) override;

			Object* allocate(OwcaVM);
			void initialize_add_base_class(OwcaVM vm, const OwcaValue &b);
			void initialize_add_function(OwcaVM vm, const OwcaValue &f);
			void finalize_initializing(OwcaVM vm);
			char* native_storage_ptr(Object *) const;

			Class(Line line, std::string_view type, std::string_view full_name, std::shared_ptr<CodeBuffer> code, size_t base_class_count);
		};

		struct Object : public AllocationBase {
			std::unordered_map<std::string, OwcaValue> values;
			Class* type_;

			Object(Class* type);

			std::string_view type() const override;
			std::string to_string() const override;
			void gc_mark(VM& vm, GenerationGC generation_gc) override;
			std::span<char> native_storage(OwcaVM::ClassToken cls);
		};
	}
}

#endif
