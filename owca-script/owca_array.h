#ifndef RC_OWCA_SCRIPT_OWCA_ARRAY_H
#define RC_OWCA_SCRIPT_OWCA_ARRAY_H

#include "stdafx.h"

namespace OwcaScript {
    class OwcaValue;

    namespace Internal {
        struct Array;
        class VM;
        class ImplExprIndexRead;
        class ImplExprIndexWrite;
        class ImplExprCompare;
    }
	class OwcaArray {
        friend class Internal::VM;
        friend class OwcaValue;
        friend class Internal::ImplExprIndexRead;
        friend class Internal::ImplExprIndexWrite;
        friend class Internal::ImplExprCompare;

		Internal::Array *object;

	public:
        OwcaArray(Internal::Array *object) : object(object) {}
        
        size_t size() const;
        void resize(size_t);
        void reserve(size_t);
        const OwcaValue &operator [] (size_t) const;
        OwcaValue &operator [] (size_t);

        std::string to_string() const;

        std::vector<OwcaValue>::iterator begin();
        std::vector<OwcaValue>::iterator end();
        std::vector<OwcaValue>::const_iterator cbegin();
        std::vector<OwcaValue>::const_iterator cend();
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
