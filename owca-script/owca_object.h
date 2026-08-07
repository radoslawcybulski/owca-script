#ifndef RC_OWCA_SCRIPT_OWCA_OBJECT_H
#define RC_OWCA_SCRIPT_OWCA_OBJECT_H

#include "owca-script/identifier_index.h"
#include "stdafx.h"
#include "tokens.h"
#include "garbage.h"
#include "identifier_index.h"
#include "native_class_interface.h"
#include "allocation_base.h"

namespace OwcaScript {
	class OwcaValue;
	class OwcaPtrObject;
	class OwcaVM;

	namespace Internal {
		struct Object;
		class VM;
	}

	class PtrObjectInterface : private Internal::AllocationBase {
		friend class Internal::VM;
		friend class OwcaPtrObject;
		friend class OwcaFunctions;

		mutable GenerationGC generation_gc_ = GenerationGC{};
		std::uintptr_t type_ptr_ = 0;

		virtual void acquired() = 0;
		virtual void released() = 0;
		virtual void gc_mark_members_impl(GenerationGC generation_gc) const {}

		void gc_mark(GenerationGC generation_gc) const override {
			if (generation_gc_ == generation_gc) return;
			generation_gc_ = generation_gc;
			gc_mark_members_impl(generation_gc);
		}

		void acquire();
		void release();

		OwcaValue bound_function_self_object() override;
	public:
		PtrObjectInterface(std::uintptr_t type_ptr) : type_ptr_(type_ptr) {}

		virtual ~PtrObjectInterface() = default;

		std::uintptr_t type_ptr() const { return type_ptr_; }
		using Internal::AllocationBase::to_string;
		using Internal::AllocationBase::type;

		virtual bool get_member(IdentifierIndex, OwcaValue &);
		virtual bool set_member(IdentifierIndex, OwcaValue);
		virtual std::optional<OwcaClass> get_type() const { return std::nullopt; }

		friend void gc_mark_members(PtrObjectInterface &, GenerationGC generation_gc);
	};

	class OwcaPtrObject {
		PtrObjectInterface *object;
	public:
		explicit OwcaPtrObject(PtrObjectInterface *object) : object(object) {
			if (object) object->acquire();
		}

		auto internal_value() const { return object; }

		std::string to_string() const { return object->to_string(); }
		std::string_view type() const { return object->type(); }

		OwcaValue member(std::string_view key) const;
		std::optional<OwcaValue> try_member(std::string_view key) const;
		void member(std::string_view key, OwcaValue);
		OwcaValue member(IdentifierIndex key) const;
		std::optional<OwcaValue> try_member(IdentifierIndex key) const;
		void member(IdentifierIndex key, OwcaValue);

		bool is(OwcaPtrObject other) const { return object == other.internal_value(); }

		bool operator == (OwcaPtrObject other) const { return object == other.internal_value(); }
		bool operator != (OwcaPtrObject other) const { return !(*this == other); }
	};

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
