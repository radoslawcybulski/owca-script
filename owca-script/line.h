#ifndef RC_OWCA_SCRIPT_LINE_H
#define RC_OWCA_SCRIPT_LINE_H

#include "stdafx.h"

namespace OwcaScript {
	namespace Internal {
		class Serializer;
		class Deserializer;

        struct Line {
			unsigned int line = 0;
            Line() {}
			explicit Line(unsigned int line) : line(line) {}

            void serialize_object(Serializer &) const;
            void deserialize_object(Deserializer &);
            bool compare(const Line &o) const {
                return line == o.line;
            }
		};
	}
}

namespace std {
    template <>
    struct formatter<OwcaScript::Internal::Line>
    {
        template <typename FormatContext>
        auto format(OwcaScript::Internal::Line v, FormatContext& ctx) const
        {
            return format_to(ctx.out(), "{}", v.line);  
        }
        template<class ParseContext>
        constexpr ParseContext::iterator parse(ParseContext& ctx)
        {
            return ctx.begin();
        }
    };
}

#endif
