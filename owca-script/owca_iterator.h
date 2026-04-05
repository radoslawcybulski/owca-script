#ifndef RC_OWCA_SCRIPT_OWCA_ITERATOR_H
#define RC_OWCA_SCRIPT_OWCA_ITERATOR_H

#include "stdafx.h"

namespace OwcaScript {
    class OwcaValue;

    namespace Internal {
        class VM;
        struct DictionaryShared;
        struct Iterator;
    }

	class OwcaIterator {
        Internal::Iterator *object;

	public:
        OwcaIterator(Internal::Iterator *object) : object(object) {}

		auto internal_value() const { return object; }

        bool completed() const;
        OwcaValue next() const;

        friend void gc_mark_value(OwcaVM vm, GenerationGC gc, const OwcaIterator &);

		class Iterator {
            OwcaIterator *iter;
		public:
            Iterator(OwcaIterator *iter);
			
            using value_type = OwcaValue;
			using pointer = value_type*;
			using reference = value_type;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::forward_iterator_tag;

			reference operator*() const;
			pointer operator->() const;

			Iterator& operator++();
			Iterator operator++(int) {
				Iterator temp = *this;
				++(*this);
				return temp;
			}

			friend bool operator==(Iterator a, Iterator b) {
                if (!a.iter) {
                    if (!b.iter) return true;
                    return b == a;
                }
                if (!b.iter) {
                    return a.iter->completed();
                }
                return false;
            }
			friend bool operator!=(Iterator a, Iterator b) { return !(a == b); }

		private:
			Internal::DictionaryShared *dictionary;
			size_t pos;
		};

        Iterator begin() { return Iterator{ this }; }
        Iterator end() { return Iterator{ nullptr }; }
	};
}

#endif
