#ifndef RC_OWCA_SCRIPT_OWCA_MAP_H
#define RC_OWCA_SCRIPT_OWCA_MAP_H

#include "stdafx.h"

namespace OwcaScript {
	class OwcaValue;
	
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
		const OwcaValue &operator [] (OwcaValue key) const;
		
		OwcaValue *value(const OwcaValue &key) const;

		std::vector<OwcaValue> keys() const;
		std::vector<OwcaValue> values() const;
		std::vector<std::pair<OwcaValue, OwcaValue>> items() const;

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

			Iterator(Internal::DictionaryShared *dictionary, size_t pos) : dictionary(dictionary), pos(pos) {}

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
		};

		Iterator begin() const;
		Iterator end() const;
	};
}

#endif
