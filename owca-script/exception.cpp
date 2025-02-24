#include "stdafx.h"
#include "exception.h"
#include "owca_value.h"
#include "vm.h"
#include "object.h"

namespace OwcaScript::Internal {
    std::string Exception::to_string() const {
        std::string temp;
        for(auto &f: frames) {
            temp += f.code->filename();
            temp += ":";
            temp += f.line;
            temp += " ";
            temp += f.function;
            temp += "\n";
        }
        temp += message;
        if (parent_exception) {
            temp += "\nwhile processing exception:\n" + parent_exception->internal_value()->to_string();
        }
        return temp;
    }
    void Exception::gc_mark(OwcaVM vm, GenerationGC generation_gc) {
        if (parent_exception) {
            VM::get(vm).gc_mark(parent_exception->internal_owner(), generation_gc);
        }
    }
}