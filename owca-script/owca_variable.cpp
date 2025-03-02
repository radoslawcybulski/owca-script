#include "stdafx.h"
#include "owca_variable.h"
#include "vm.h"

namespace OwcaScript {
    OwcaVariableSet::OwcaVariableSet(OwcaVariableSet *set) {
        next = set->next;
        prev = set;
        next->prev = prev->next = this;
    }
    OwcaVariableSet::~OwcaVariableSet() {
        prev->next = next;
        next->prev = prev;
    }
    void OwcaVariableSet::gc_mark(OwcaVM vm, GenerationGC ggc) const {
        auto p = next;
        while(p != this) {
            vm.gc_mark(reinterpret_cast<const OwcaVariable&>(*p), ggc);
            p = p ->next;
        }
    }
    OwcaVariable::OwcaVariable(const OwcaVM &vm) : OwcaVariable(Internal::VM::get(vm).global_variables) {}

    OwcaVariable &OwcaVariable::operator = (OwcaValue v) {
        static_cast<OwcaValue&>(*this) = v;
        return *this;
    }

    OwcaVariable &OwcaVariable::operator = (const OwcaVariable &v) {
        static_cast<OwcaValue&>(*this) = static_cast<const OwcaValue&>(v);
        return *this;
    }
}