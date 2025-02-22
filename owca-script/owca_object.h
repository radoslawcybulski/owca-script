#ifndef RC_OWCA_SCRIPT_OWCA_OBJECT_H
#define RC_OWCA_SCRIPT_OWCA_OBJECT_H

#include "stdafx.h"

namespace OwcaScript {
	class OwcaValue;
	class OwcaVM;
	class ClassToken;
	class FunctionToken;

	namespace Internal {
		struct Object;
	}
	class OwcaObject {
		Internal::Object* object;

		std::span<char> user_data_impl(ClassToken) const;
	public:
		explicit OwcaObject(Internal::Object* object) : object(object) {}

		auto internal_value() const { return object; }
		
		std::string to_string() const;
		std::string_view type() const;

		OwcaValue member(const std::string& key) const;
		std::optional<OwcaValue> try_member(const std::string& key) const;

		void member(const std::string& key, OwcaValue);

		template <typename T> T &user_data(ClassToken token) const {
			auto sp = user_data_impl(token);
			assert(sp.size() >= sizeof(T));
			return *(T*)sp.data();
		}
	};
}

#endif
