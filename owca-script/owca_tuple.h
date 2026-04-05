#ifndef RC_OWCA_SCRIPT_OWCA_TUPLE_H
#define RC_OWCA_SCRIPT_OWCA_TUPLE_H

#include "stdafx.h"

namespace OwcaScript {
    class OwcaValue;

    namespace Internal {
        struct Tuple;
    }
	class OwcaTuple {
		Internal::Tuple *object;

	public:
        OwcaTuple(Internal::Tuple *object) : object(object) {}
		
		auto internal_value() const { return object; }

        size_t size() const;
        OwcaValue operator [] (size_t) const;

        std::string to_string() const;

		class Iterator {
		public:
			using value_type = OwcaValue;
			using pointer = value_type*;
			using reference = value_type;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::forward_iterator_tag;

			Iterator(Internal::Tuple *tuple, size_t pos) : tuple(tuple), pos(pos) {}

			reference operator*() const;
			pointer operator->();

			Iterator& operator++() { ++pos; return *this; }

			Iterator operator++(int) {
				Iterator temp = *this;
				++(*this);
				return temp;
			}

			friend bool operator==(Iterator a, Iterator b) { return a.tuple == b.tuple && a.pos == b.pos; }
			friend bool operator!=(Iterator a, Iterator b) { return !(a == b); }

		private:
			Internal::Tuple *tuple;
			size_t pos;
		};

		Iterator begin() { return Iterator(object, 0); }
		Iterator end() { return Iterator(object, size()); }

		friend void gc_mark_value(OwcaVM vm, GenerationGC gc, const OwcaTuple &);
	};
}

namespace std {
	template <>
	struct formatter<OwcaScript::OwcaTuple>
	{
		template <typename FormatContext>
		auto format(OwcaScript::OwcaTuple v, FormatContext& ctx) const
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
