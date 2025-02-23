#ifndef RC_OWCA_SCRIPT_OWCA_VARIABLE_H
#define RC_OWCA_SCRIPT_OWCA_VARIABLE_H

#include "stdafx.h"
#include "owca_value.h"

namespace OwcaScript {
	class OwcaVM;

    class OwcaVariable {
        friend class Internal::VM;

        OwcaValue value;
        Internal::VM *vm;
        size_t global_variable_index;
    public:
        OwcaVariable(const OwcaVM &vm);
        OwcaVariable(const OwcaVariable &);
        OwcaVariable(OwcaVariable&&) = delete;
        ~OwcaVariable();

        OwcaVariable &operator = (const OwcaVariable &);
        OwcaVariable &operator = (OwcaVariable &&) = delete;

        OwcaVariable &operator = (OwcaValue);

        operator OwcaValue () const { return value; }
        auto operator -> () const { return &value; }
        auto operator * () const { return value; }
    };
}

#endif
