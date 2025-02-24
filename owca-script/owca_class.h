#ifndef RC_OWCA_SCRIPT_OWCA_CLASS_H
#define RC_OWCA_SCRIPT_OWCA_CLASS_H

#include "stdafx.h"
#include <new>

namespace OwcaScript {
	class OwcaValue;
	class GenerationGC;
	class OwcaVM;
	
	namespace Internal {
		struct Class;
	}

	class OwcaClass {
		Internal::Class* object;

	public:
		struct NativeClassInterface {
			virtual ~NativeClassInterface() = default;

			virtual void initialize_storage(void* ptr, size_t s) = 0;
			virtual void destroy_storage(void* ptr, size_t s) = 0;
			virtual void gc_mark_members(void* ptr, size_t s, OwcaVM vm, GenerationGC generation_gc) = 0;
			virtual size_t native_storage_size() = 0;
		};
		template <typename T> struct NativeClassInterfaceSimpleImplementation : public NativeClassInterface {
			void initialize_storage(void* ptr, size_t s) override { new (ptr) T(); }
			void destroy_storage(void* ptr, size_t s) override { ((T*)ptr)->~T(); }
			void gc_mark_members(void* ptr, size_t s, OwcaVM vm, GenerationGC generation_gc) override { ((T*)ptr)->gc_mark(vm, generation_gc); }
			size_t native_storage_size() override { return sizeof(T); }
		};

		explicit OwcaClass(Internal::Class* object) : object(object) { assert(object); }

		auto internal_value() const { return object; }

		std::string to_string() const;
		std::string_view type() const { return "class"; }
		bool has_base_class(OwcaClass base) const;
		OwcaValue operator [] (const std::string &key) const;
	};
}

#endif
