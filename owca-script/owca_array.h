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
