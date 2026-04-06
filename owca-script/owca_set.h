#ifndef RC_OWCA_SCRIPT_OWCA_SET_H
#define RC_OWCA_SCRIPT_OWCA_SET_H

#include "stdafx.h"

namespace OwcaScript {
	class OwcaValue;
	
	namespace Internal {
		struct SetShared;
	}

	class OwcaSet {
		Internal::SetShared *dictionary;

	public:
		OwcaSet(Internal::SetShared* dictionary) : dictionary(dictionary) {}
		~OwcaSet() = default;

		auto internal_value() const { return dictionary; }
		
		std::string to_string() const;
		size_t size() const;
		void add(OwcaValue key);
		void remove(OwcaValue key);
		OwcaSet copy() const;

		bool has_value(OwcaValue key) const;

		void union_with(OwcaSet other);
		void intersection_with(OwcaSet other);
		void difference_with(OwcaSet other);

		OwcaSet operator | (OwcaSet other) const { auto v = copy(); v.union_with(other);  return v; }
		OwcaSet operator & (OwcaSet other) const { auto v = copy(); v.intersection_with(other);  return v; }
		OwcaSet operator - (OwcaSet other) const { auto v = copy(); v.difference_with(other);  return v; }

		OwcaSet &operator |= (OwcaSet other) { union_with(other); return *this; }
		OwcaSet &operator &= (OwcaSet other) { intersection_with(other); return *this; }
		OwcaSet &operator -= (OwcaSet other) { difference_with(other); return *this; }
		
		class Iterator {
		public:
			using value_type = const OwcaValue&;
			using pointer = const OwcaValue *;
			using reference = value_type;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::forward_iterator_tag;

			Iterator(Internal::SetShared *dictionary, size_t pos);

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
			Internal::SetShared *dictionary;
			size_t pos;
			size_t version;
		};

		Iterator begin() const;
		Iterator end() const;

		friend void gc_mark_value(OwcaVM vm, GenerationGC gc, const OwcaSet &);
	};
}

#endif
