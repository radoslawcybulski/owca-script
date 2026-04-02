#include "stdafx.h"
#include "owca_variable.h"
#include "vm.h"

namespace OwcaScript {
    OwcaVariable &OwcaVariable::operator = (const OwcaVariable &v) {
        static_cast<OwcaValue&>(*this) = static_cast<const OwcaValue&>(v);
        return *this;
    }

    void gc_mark_value(OwcaVM vm, GenerationGC gc, const OwcaVariable &v) {
        gc_mark_value(vm, gc, static_cast<const OwcaValue&>(v));
    }
}