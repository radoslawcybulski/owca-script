#include "stdafx.h"
#include "owca_variable.h"
#include "vm.h"

namespace OwcaScript {
    OwcaVariable::OwcaVariable(const OwcaVM &vm) {
        this->vm = vm.vm;
        this->vm->register_variable(*this);
    }
    OwcaVariable::OwcaVariable(const OwcaVariable &v) : vm(v.vm), value(v.value) {
        this->vm->register_variable(*this);
    }
    OwcaVariable::~OwcaVariable() {
        this->vm->unregister_variable(*this);
    }

    OwcaVariable &OwcaVariable::operator = (const OwcaVariable &v) {
        if (this != &v) {
            if (vm != v.vm) {
                this->vm->unregister_variable(*this);
                vm = v.vm;
                this->vm->register_variable(*this);
            }
            value = v.value;
        }
        return *this;
    }

    OwcaVariable &OwcaVariable::operator = (OwcaValue v) {
        value = v;
        return *this;
    }
}