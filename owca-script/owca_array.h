#ifndef RC_OWCA_SCRIPT_OWCA_ARRAY_H
#define RC_OWCA_SCRIPT_OWCA_ARRAY_H

#include "stdafx.h"

namespace OwcaScript {
    class OwcaValue;

    namespace Internal {
        struct Array;
    }
	class OwcaArray {
		Internal::Array *object;

	public:
        OwcaArray(Internal::Array *object) : object(object) {}
        
		auto internal_value() const { return object; }

        size_t size() const;
        void resize(size_t);
        OwcaValue operator [] (size_t) const;
        OwcaValue &operator [] (size_t);
		void push_back(OwcaValue);
		void push_front(OwcaValue);
		OwcaValue pop_back();
		OwcaValue pop_front();
        std::string to_string() const;

		friend void gc_mark_value(OwcaVM vm, GenerationGC gc, const OwcaArray &);

		class Iterator {
		public:
			using value_type = OwcaValue;
			using pointer = value_type*;
			using reference = value_type;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::forward_iterator_tag;

			Iterator(Internal::Array *array, size_t pos) : array(array), pos(pos) {}

			reference operator*() const;
			pointer operator->();

			Iterator& operator++() { ++pos; return *this; }

			Iterator operator++(int) {
				Iterator temp = *this;
				++(*this);
				return temp;
			}

			friend bool operator==(Iterator a, Iterator b) { return a.array == b.array && a.pos == b.pos; }
			friend bool operator!=(Iterator a, Iterator b) { return !(a == b); }

		private:
			Internal::Array *array;
			size_t pos;
		};

		Iterator begin() { return Iterator(object, 0); }
		Iterator end() { return Iterator(object, size()); }
	};
}

namespace std {
	template <>
	struct formatter<OwcaScript::OwcaArray>
	{
		template <typename FormatContext>
		auto format(OwcaScript::OwcaArray v, FormatContext& ctx) const
		{
			return format_to(ctx.out(), "{}", v.to_string());  
		}
		template<class ParseContext>
		constexpr ParseContext::iterator parse(ParseContext& ctx)
		{
			return ctx.begin();
		}
	};
}

#endif
