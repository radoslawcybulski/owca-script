#ifndef RC_OWCA_SCRIPT_OWCA_NAMESPACE_H
#define RC_OWCA_SCRIPT_OWCA_NAMESPACE_H

#include "stdafx.h"
#include "tokens.h"

namespace OwcaScript {
	class OwcaValue;
	class OwcaVM;
	class ClassToken;
	class FunctionToken;

	namespace Internal {
		struct Namespace;
	}
	class OwcaNamespace {
		Internal::Namespace* object;

        public:
		explicit OwcaNamespace(Internal::Namespace* object) : object(object) {}

		auto internal_value() const { return object; }
		
		std::string to_string() const;
		std::string_view type() const;

		OwcaValue member(std::string_view key) const;
		std::optional<OwcaValue> try_member(std::string_view key) const;
		void member(std::string_view key, OwcaValue);
        bool try_member(std::string_view key, OwcaValue);

		class Iterator {
		public:
			using value_type = std::pair<std::string_view, OwcaValue&>;
			class Pointer {
				value_type val;
			public:
				Pointer(value_type val) : val(val) {}

				value_type *operator -> () { return &val; }
			};
			using pointer = Pointer;
			using reference = value_type;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::forward_iterator_tag;

			Iterator(Internal::Namespace *nspace, std::unordered_map<std::string_view, size_t>::iterator it);

			reference operator*() const;
			pointer operator->();

			Iterator& operator++();

			Iterator operator++(int) {
				Iterator temp = *this;
				++(*this);
				return temp;
			}

			friend bool operator==(Iterator a, Iterator b) { return a.it == b.it; }
			friend bool operator!=(Iterator a, Iterator b) { return a.it != b.it; }

		private:
			Internal::Namespace *nspace;
			std::unordered_map<std::string_view, size_t>::iterator it;
		};

        Iterator begin() const;
		Iterator end() const;

        friend void gc_mark_value(OwcaVM vm, GenerationGC gc, OwcaNamespace);
	};
}

#endif
