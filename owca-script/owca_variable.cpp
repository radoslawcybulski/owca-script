#include "stdafx.h"
#include "owca_variable.h"
#include "vm.h"

namespace OwcaScript {
    OwcaVariable &OwcaVariable::operator = (const OwcaVariable &v) {
        static_cast<OwcaValue&>(*this) = static_cast<const OwcaValue&>(v);
        return *this;
    }
}