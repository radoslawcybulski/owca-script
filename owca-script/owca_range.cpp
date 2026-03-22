#include "stdafx.h"
#include "owca_range.h"
#include "owca_value.h"
#include "range.h"
#include "vm.h"

namespace OwcaScript {
    std::string OwcaRange::to_string() const {
        return object->to_string();
    }
    Number OwcaRange::lower() const {
        return object->from;
    }
    Number OwcaRange::upper() const {
        return object->to;
    }
    Number OwcaRange::step() const {
        return object->step;
    }

    Number OwcaRange::size() const {
        if (object->step == 0) {
            Internal::VM::get(object->vm).range_step_is_zero();
        }
        if (object->step > 0) {
            return std::max(Number{ 0 }, std::floor((object->to + object->step * 0.5 - object->from) / object->step));
        }
        else {
            return std::max(Number{ 0 }, std::floor((object->from + object->step * 0.5 - object->to) / -object->step));
        }
    }
}