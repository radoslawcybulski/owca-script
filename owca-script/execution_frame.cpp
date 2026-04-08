#include "stdafx.h"
#include "execution_frame.h"
#include "owca_value.h"
#include "vm.h"
#include "runtime_function.h"

namespace OwcaScript::Internal {
    ExecutionFrame::ExecutionFrame() {
        values.reserve(8);
        temporaries.reserve(8);
    }
    ExecutionFrame::~ExecutionFrame() = default;

    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ExecutionFrame::WhileState &e) {

    }
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ExecutionFrame::ForState &e) {

    }
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ExecutionFrame::TryCatchState &e) {

    }
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ExecutionFrame::States &e) {
        visit_variant(e, [&](const auto &e) {
            gc_mark_value(vm, generation_gc, e);
        });
    }
    void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ExecutionFrame &e)
    {
        gc_mark_value(vm, generation_gc, e.runtime_function);
        gc_mark_value(vm, generation_gc, e.runtime_functions);
        gc_mark_value(vm, generation_gc, e.values);
        gc_mark_value(vm, generation_gc, e.temporaries);
        gc_mark_value(vm, generation_gc, e.states);
    }
}