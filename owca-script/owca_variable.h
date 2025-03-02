#ifndef RC_OWCA_SCRIPT_OWCA_VARIABLE_H
#define RC_OWCA_SCRIPT_OWCA_VARIABLE_H

#include "stdafx.h"
#include "owca_value.h"

namespace OwcaScript {
	class OwcaVM;
    class OwcaVariable;

    class OwcaVariableSet {
        OwcaVariableSet *next = nullptr, *prev = nullptr;
        
    protected:
        explicit OwcaVariableSet(OwcaVariableSet *);
    public:
        OwcaVariableSet() : next(this), prev(this) {}
        OwcaVariableSet(const OwcaVariableSet &) = delete;
        OwcaVariableSet(OwcaVariableSet&&) = delete;
        ~OwcaVariableSet();

        OwcaVariableSet &operator = (const OwcaVariableSet &) = delete;
        OwcaVariableSet &operator = (OwcaVariableSet &&) = delete;

        void gc_mark(OwcaVM, GenerationGC) const;
    };

    class OwcaVariable : protected OwcaVariableSet, public OwcaValue {
    public:
        OwcaVariable(const OwcaVM &vm);
        OwcaVariable(OwcaVariableSet &set) : OwcaVariableSet(&set) {}
        OwcaVariable(const OwcaVariable &) = delete;
        OwcaVariable(OwcaVariable&&) = delete;
        ~OwcaVariable() = default;

        OwcaVariable &operator = (const OwcaVariable &);
        OwcaVariable &operator = (OwcaVariable &&) = delete;
        OwcaVariable &operator = (OwcaValue);
    };
}

#endif
