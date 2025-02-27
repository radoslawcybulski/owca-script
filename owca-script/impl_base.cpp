#include "stdafx.h"
#include "impl_base.h"
#include "vm.h"

namespace OwcaScript::Internal {
    void ImplStat::execute_statement(OwcaVM vm) const
    {
        VM::get(vm).update_execution_line(line);
        auto pp = VM::AllocatedObjectsPointer{ VM::get(vm) };
        return execute_statement_impl(vm);
    }

    OwcaValue ImplExpr::execute_expression(OwcaVM vm) const
    {
        VM::get(vm).update_execution_line(line);
        return execute_expression_impl(vm);
    }
}
