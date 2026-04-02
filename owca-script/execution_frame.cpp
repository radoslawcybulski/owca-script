#include "stdafx.h"
#include "execution_frame.h"
#include "owca_value.h"
#include "vm.h"
#include "runtime_function.h"

namespace OwcaScript::Internal {
    ExecutionFrame::ExecutionFrame(Line line) : line(line) {}
    ExecutionFrame::~ExecutionFrame() = default;

    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ExecutionFrame &e)
    {
        gc_mark_value(vm, generation_gc, e.runtime_function);
        gc_mark_value(vm, generation_gc, e.runtime_functions);
        gc_mark_value(vm, generation_gc, e.values);
    }
}