#ifndef RC_OWCA_SCRIPT_OWCA_EXCEPTION_H
#define RC_OWCA_SCRIPT_OWCA_EXCEPTION_H

#include "stdafx.h"

namespace OwcaScript {
	class OwcaClass;
	
	namespace Internal {
		struct Exception;
		struct Object;
	}
	class OwcaException {
		Internal::Object *owner;
		Internal::Exception *object;
	public:
		explicit OwcaException(Internal::Object *owner, Internal::Exception *object) : owner(owner), object(object) {}

		auto internal_owner() const { return owner; }
		auto internal_value() const { return object; }

		std::string_view message() const;
		std::string to_string() const;
		size_t count() const;
		OwcaClass type() const;
		struct Frame {
			std::string_view filename;
			std::string_view function;
			unsigned int line;
		};
		Frame frame(unsigned int) const;
	};
}

#endif
