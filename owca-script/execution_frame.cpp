#include "stdafx.h"
#include "execution_frame.h"
#include "owca_value.h"
#include "vm.h"
#include "runtime_function.h"

namespace OwcaScript::Internal {
    ExecutionFrame::ExecutionFrame(Line line) : line(line) {}
    ExecutionFrame::~ExecutionFrame() = default;

    void ExecutionFrame::gc_mark(OwcaVM vm, GenerationGC generation_gc)
    {
        VM::get(vm).gc_mark(runtime_function, generation_gc);
        VM::get(vm).gc_mark(runtime_functions, generation_gc);
        VM::get(vm).gc_mark(values, generation_gc);
    }
}