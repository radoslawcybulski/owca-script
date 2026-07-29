#ifndef RC_OWCA_SCRIPT_OWCA_OBJECT_H
#define RC_OWCA_SCRIPT_OWCA_OBJECT_H

#include "owca-script/identifier_index.h"
#include "stdafx.h"
#include "tokens.h"
#include "object.h"
#include "garbage.h"
#include "identifier_index.h"
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

		OwcaValue member(std::string_view key) const;
		std::optional<OwcaValue> try_member(std::string_view key) const;
		void member(std::string_view key, OwcaValue);
		OwcaValue member(IdentifierIndex key) const;
		std::optional<OwcaValue> try_member(IdentifierIndex key) const;
		void member(IdentifierIndex key, OwcaValue);

		bool is(OwcaObject other) const { return object == other.object; }


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
			assert(sp.size() >= sizeof(T) || sp.data() == nullptr);
			return (T*)sp.data();
		}

		friend void gc_mark_value(GenerationGC gc, const OwcaObject &);
	};
}

#endif
