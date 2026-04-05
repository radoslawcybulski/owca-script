#include "stdafx.h"
#include "range.h"
#include "owca_value.h"
#include "generator.h"
#include "vm.h"

namespace OwcaScript::Internal {
    std::string Range::to_string() const
    {
        return std::format("{}:{}:{}", from, to, step);
    }

    size_t Range::hash() const
    {
        return std::hash<Number>{}(from) * 3 + std::hash<Number>{}(to) * 5 + std::hash<Number>{}(step) * 7;
    }

    Generator Range::iter(OwcaVM vm) const {
        auto index = from;
        if (step == 0) {
            VM::get(vm).range_step_is_zero();
        }
        if (step > 0) {
            while(index < to) {
                co_yield index;
                index += step;
            }
        }
        else {
            while(index > to) {
                co_yield index;
                index += step;
            }
        }
    }
}