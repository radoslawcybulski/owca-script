#include "stdafx.h"
#include "impl_base.h"
#include "vm.h"

namespace OwcaScript::Internal {
    void ImplStat::execute(OwcaVM vm) const
    {
        VM::get(vm).update_execution_line(line);
        auto pp = VM::AllocatedObjectsPointer{ VM::get(vm) };
        return execute_impl(vm);
    }

    OwcaValue ImplExpr::execute(OwcaVM vm) const
    {
        VM::get(vm).update_execution_line(line);
        return execute_impl(vm);
    }
}
