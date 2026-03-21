#ifndef RC_OWCA_SCRIPT_OWCA_CLASS_H
#define RC_OWCA_SCRIPT_OWCA_CLASS_H

#include "stdafx.h"
#include <new>
#include "owca_vm.h"
#include "native_class_interface.h"

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
		explicit OwcaClass(Internal::Class* object) : object(object) { assert(object); }

		auto internal_value() const { return object; }

		std::string to_string() const;
		std::string_view type() const { return "class"; }
		bool has_base_class(OwcaClass base) const;
		OwcaValue operator [] (const std::string &key) const;
	};
}

#endif
