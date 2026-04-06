#ifndef RC_OWCA_SCRIPT_OWCA_MAP_H
#define RC_OWCA_SCRIPT_OWCA_MAP_H

#include "stdafx.h"

namespace OwcaScript {
	namespace Internal {
		struct DictionaryShared;
	}

	class OwcaMap {
		Internal::DictionaryShared *dictionary;

	public:
		OwcaMap(Internal::DictionaryShared* dictionary) : dictionary(dictionary) {}
		~OwcaMap() = default;

		auto internal_value() const { return dictionary; }

		std::string to_string() const;
		size_t size() const;

		OwcaValue &operator [] (OwcaValue key);
		OwcaValue operator [] (OwcaValue key) const;
		
		OwcaValue *value(OwcaValue key) const;

		Generator keys() const;
		Generator values() const;
		Generator items() const;
		OwcaValue has_key(OwcaValue key);
		OwcaValue pop(OwcaValue key);
		OwcaValue pop_or_default(OwcaValue key, OwcaValue default_value);
		OwcaValue get_or_default(OwcaValue key, OwcaValue default_value);
		OwcaValue set_default(OwcaValue key, OwcaValue default_value);

		class Iterator {
		public:
			using value_type = std::pair<const OwcaValue&, OwcaValue&>;
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

			Iterator(Internal::DictionaryShared *dictionary, size_t pos);

			reference operator*() const;
			pointer operator->();

			Iterator& operator++();

			Iterator operator++(int) {
				Iterator temp = *this;
				++(*this);
				return temp;
			}

			friend bool operator==(Iterator a, Iterator b) { return a.pos == b.pos; }
			friend bool operator!=(Iterator a, Iterator b) { return a.pos != b.pos; }

		private:
			Internal::DictionaryShared *dictionary;
			size_t pos;
			size_t version;
		};

		Iterator begin() const;
		Iterator end() const;

		friend void gc_mark_value(OwcaVM vm, GenerationGC gc, const OwcaMap &);
	};
}

#endif
