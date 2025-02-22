#ifndef RC_OWCA_SCRIPT_OWCA_SET_H
#define RC_OWCA_SCRIPT_OWCA_SET_H

#include "stdafx.h"

namespace OwcaScript {
	class OwcaValue;
	
	namespace Internal {
		struct DictionaryShared;
	}

	class OwcaSet {
		Internal::DictionaryShared *dictionary;

	public:
		OwcaSet(Internal::DictionaryShared* dictionary) : dictionary(dictionary) {}
		~OwcaSet() = default;

		auto internal_value() const { return dictionary; }
		
		std::string to_string() const;
		size_t size() const;

		bool has_value(const OwcaValue &key) const;

		class Iterator {
		public:
			using value_type = const OwcaValue&;
			using pointer = const OwcaValue *;
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
