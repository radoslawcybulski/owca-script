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
    void gc_mark_value(OwcaVM vm, GenerationGC gc, const Exception &e) {
        if (e.parent_exception) {
            gc_mark_value(vm, gc, *e.parent_exception);
        }
    }
}