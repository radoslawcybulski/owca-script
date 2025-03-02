#include "stdafx.h"
#include "impl_base.h"
#include "vm.h"

namespace OwcaScript::Internal {
    ImplStat::Task::Yield::Yield(OwcaVM vm, State &st, OwcaValue val) : st(st)
    {
        VM::get(vm).set_yield_value(val);
    }

    bool ImplStat::Task::Yield::await_suspend(std::coroutine_handle<promise_type> h) noexcept
    {
        if constexpr (debug_print) std::cout << "Awaiter::await_suspend(" << (void*)this << ") storing to " << ((char*)st.top() - st.storage.data()) <<"\n";
        st.top()->h = h;
        return true;
    }

    ImplStat::Task *ImplStat::State::top() {
        assert(storage.size() >= sizeof(Task));
        return (Task*)((char*)storage.data() + storage.size() - sizeof(Task));
    }

    void ImplStat::execute_statement(OwcaVM vm) const
    {
        VM::get(vm).update_execution_line(line);
        auto pp = VM::AllocatedObjectsPointer{ VM::get(vm) };
        return execute_statement_impl(vm);
    }

    ImplStat::Task ImplStat::execute_generator_statement(OwcaVM vm, State &st) const
    {
        return execute_generator_statement_impl(vm, st);
    }

    OwcaValue ImplExpr::execute_expression(OwcaVM vm) const
    {
        VM::get(vm).update_execution_line(line);
        return execute_expression_impl(vm);
    }

    size_t ImplStat::calculate_generator_object_size_for_this() const
    {
        size_t sz;
        {
            auto state = State{ 4096 };
            auto t = execute_generator_statement((Internal::VM*)nullptr, state);
            sz = state.storage.size();
        }
        return sz;
    }
}
