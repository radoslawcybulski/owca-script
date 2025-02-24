#include "stdafx.h"
#include "owca_exception.h"
#include "exception.h"
#include "code_buffer.h"
#include "object.h"

namespace OwcaScript {
    OwcaClass OwcaException::type() const
    {
        return OwcaClass{ owner->type_ };
    }
    std::string_view OwcaException::message() const {
        return object->message;
    }
    std::string OwcaException::to_string() const {
        return object->to_string();
    }
    size_t OwcaException::count() const {
        return object->frames.size();
    }
    OwcaException::Frame OwcaException::frame(unsigned int index) const {
        assert(index < object->frames.size());

        return {
            object->frames[index].code->filename(),
            object->frames[index].function,
            object->frames[index].line
        };
    }
}