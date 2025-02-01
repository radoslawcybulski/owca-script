#ifndef RC_OWCA_SCRIPT_OWCA_OBJECT_H
#define RC_OWCA_SCRIPT_OWCA_OBJECT_H

#include "stdafx.h"
#include "owca_vm.h"

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

		std::span<char> user_data_impl(OwcaVM::ClassToken) const;
	public:
		explicit OwcaObject(Internal::Object* object) : object(object) {}

		std::string to_string() const;
		std::string_view type() const;

		OwcaValue member(const std::string& key) const;
		std::optional<OwcaValue> try_member(const std::string& key) const;

		void member(const std::string& key, OwcaValue);

		template <typename T> T &user_data(OwcaVM::ClassToken token) const {
			auto sp = user_data_impl(token);
			assert(sp.size() >= sizeof(T));
			return *(T*)sp.data();
		}
	};
}

#endif
