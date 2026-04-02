#ifndef RC_OWCA_SCRIPT_OWCA_VARIABLE_H
#define RC_OWCA_SCRIPT_OWCA_VARIABLE_H

#include "stdafx.h"
#include "owca_value.h"

namespace OwcaScript {
	class OwcaVM;
    class OwcaVariable;

    class OwcaVariable : public OwcaValue {
    public:
        OwcaVariable() = default;
        OwcaVariable(const OwcaVariable &) = delete;
        OwcaVariable(OwcaVariable&&) = delete;
        ~OwcaVariable() = default;

        OwcaVariable &operator = (const OwcaVariable &);
        OwcaVariable &operator = (OwcaVariable &&) = delete;
        OwcaVariable &operator = (OwcaValue);

        friend void gc_mark_value(OwcaVM vm, GenerationGC gc, const OwcaVariable &);
    };
}

#endif
