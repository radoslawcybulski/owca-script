#include "stdafx.h"
#include "iterator.h"
#include "vm.h"
#include "runtime_function.h"
#include "executor.h"

namespace OwcaScript::Internal {
    Iterator::Iterator(RuntimeFunction *function, std::span<OwcaValue> values, std::span<Executor::StatesType> states) : function(function), values(values), states(states) {}
    Iterator::~Iterator() = default;

    std::string Iterator::to_string() const {
        return "generator " + std::string{ function->full_name };
    }
    void Iterator::gc_mark(GenerationGC generation_gc) const {
        gc_mark_value(generation_gc, function);
        for(auto &v : values) {
            gc_mark_value(generation_gc, v);
        }
        for(auto &s : states) {
            gc_mark_value(generation_gc, s);
        }
    }
    OwcaValue Iterator::bound_function_self_object() {
        return OwcaIterator{ this };
    }
}