#include "stdafx.h"
#include "string.h"
#include "vm.h"

namespace OwcaScript::Internal {
    size_t String::hash() const
    {
        if (!hash_calculated) {
            hash_calculated = true;
            hash_value = 0;
            hash_value = std::hash<std::string>()(data);
        }
        return hash_value;
    }
}