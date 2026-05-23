#ifndef RC_OWCA_SCRIPT_OWCA_OBJECT_H
#define RC_OWCA_SCRIPT_OWCA_OBJECT_H

#include "stdafx.h"
#include "tokens.h"
#include "object.h"
#include "garbage.h"
#include "native_class_interface.h"

namespace OwcaScript {
	class OwcaValue;
	class OwcaVM;

	namespace Internal {
		struct Object;
	}
	class OwcaObject {
		Internal::Object* object;

		std::span<char> user_data_impl(Internal::UserClassTokenPtr) const;
	public:
		explicit OwcaObject(Internal::Object* object) : object(object) {}

		auto internal_value() const { return object; }
		
		std::string to_string() const;
		std::string_view type() const;

		OwcaValue member(const std::string& key) const;
		std::optional<OwcaValue> try_member(const std::string& key) const;
		bool is(OwcaObject other) const { return object == other.object; }

		void member(const std::string& key, OwcaValue);

		bool operator == (OwcaObject other) const { return object == other.object; }
		bool operator != (OwcaObject other) const { return !(*this == other); }
		
		template <typename T> T &user_data_certainly() const {
			auto tok = NativeClassInterfaceImplementation<T>::token();
			auto sp = user_data_impl(tok);
			assert(sp.size() >= sizeof(T));
			assert(sp.data() != nullptr);
			return *(T*)sp.data();
		}
		template <typename T> T *user_data_maybe() const {
			auto tok = NativeClassInterfaceImplementation<T>::token();
			auto sp = user_data_impl(tok);
			assert(sp.size() >= sizeof(T));
			return (T*)sp.data();
		}

		friend void gc_mark_value(const OwcaVM &vm, GenerationGC gc, const OwcaObject &);
	};
}

#endif
