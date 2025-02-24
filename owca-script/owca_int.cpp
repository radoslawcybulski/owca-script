#include "stdafx.h"
#include "owca_int.h"
#include "vm.h"

namespace OwcaScript {
    void OwcaInt::throw_error(OwcaVM vm, std::string_view msg)
    {
        Internal::VM::get(vm).throw_overflow(msg);
    }
}