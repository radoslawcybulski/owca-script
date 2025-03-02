#include "stdafx.h"
#include "iterator.h"
#include "vm.h"
#include "runtime_function.h"

namespace OwcaScript::Internal {
    OwcaValue Iterator::execute_next() {
        if (!generator) return OwcaCompleted{};
        return vm->resume_generator(OwcaIterator{ this });
    }
    std::string Iterator::to_string() const {
        return "generator " + std::string{ frame.runtime_function->full_name };
    }
    void Iterator::gc_mark(VM &vm, GenerationGC generation_gc) {
        frame.gc_mark(vm, generation_gc);
    }
}