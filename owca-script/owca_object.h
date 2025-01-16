#ifndef RC_OWCA_SCRIPT_OWCA_OBJECT_H
#define RC_OWCA_SCRIPT_OWCA_OBJECT_H

#include "stdafx.h"

namespace OwcaScript {
	class OwcaValue;
	class OwcaVM;

	namespace Internal {
		struct Object;
		class VM;
	}
	class OwcaObject {
		friend class OwcaValue;
		friend class Internal::VM;

		Internal::Object* object;

	public:
		explicit OwcaObject(Internal::Object* object) : object(object) {}

		std::string to_string() const;
		std::string_view type() const;

		OwcaValue member(OwcaVM &vm, const std::string& key) const;
		void member(const std::string& key, OwcaValue);
	};
}

#endif
