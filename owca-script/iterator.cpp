#include "stdafx.h"
#include "iterator.h"
#include "vm.h"
#include "runtime_function.h"

namespace OwcaScript::Internal {
    // OwcaValue Iterator::execute_next() {
    //     first_time = false;
    //     if (!generator) return OwcaCompleted{};
    //     value = vm->resume_generator(OwcaIterator{ this });
    //     return value;
    // }
    std::string Iterator::to_string() const {
        return "generator " + std::string{ frame->runtime_function->full_name };
    }
    void Iterator::gc_mark(OwcaVM vm, GenerationGC generation_gc) const {
        gc_mark_value(vm, generation_gc, *frame);
    }
}