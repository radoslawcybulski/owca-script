#ifndef RC_OWCA_SCRIPT_OWCA_CLASS_H
#define RC_OWCA_SCRIPT_OWCA_CLASS_H

#include "stdafx.h"

namespace OwcaScript {
	class OwcaValue;
	class GenerationGC;

	namespace Internal {
		struct Class;
		class VM;
	}

	class OwcaClass {
		friend class OwcaValue;
		friend class Internal::VM;

		Internal::Class* object;

	public:
		struct NativeClassInterface {
			virtual ~NativeClassInterface() = default;

			virtual void initialize_storage(void* ptr, size_t s) = 0;
			virtual void destroy_storage(void* ptr, size_t s) = 0;
			virtual void gc_mark_members(void* ptr, size_t s, GenerationGC generation_gc) = 0;
			virtual size_t native_storage_size() = 0;
		};

		explicit OwcaClass(Internal::Class* object) : object(object) {}

		std::string to_string() const;
		std::string_view type() const { return "class"; }
		OwcaValue operator [] (const std::string &key) const;
	};
}

#endif
