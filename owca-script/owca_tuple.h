#ifndef RC_OWCA_SCRIPT_OWCA_TUPLE_H
#define RC_OWCA_SCRIPT_OWCA_TUPLE_H

#include "stdafx.h"

namespace OwcaScript {
    class OwcaValue;

    namespace Internal {
        struct Array;
    }
	class OwcaTuple {
		Internal::Array *object;

	public:
        OwcaTuple(Internal::Array *object) : object(object) {}
		
		auto internal_value() const { return object; }

        size_t size() const;
        OwcaValue operator [] (size_t) const;

        std::string to_string() const;

        std::vector<OwcaValue>::const_iterator begin();
        std::vector<OwcaValue>::const_iterator end();
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
