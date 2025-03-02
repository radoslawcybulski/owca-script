#include "stdafx.h"
#include "array.h"
#include "owca_value.h"
#include "vm.h"

namespace OwcaScript::Internal {
    std::string Array::to_string() const
    {
        if (values.empty()) return is_tuple ? "()" : "[]";

        std::string temp;
        temp += is_tuple ? "(" : "[";
        for(auto &q : values) {
            if (temp.size() > 1) temp += ",";
            temp += " ";
            temp += q.to_string();
        }
        temp += is_tuple ? " )" : " ]";
        return temp;
    }

    void Array::gc_mark(OwcaVM vm, GenerationGC generation_gc)
    {
        VM::get(vm).gc_mark(values, generation_gc);
    }

    size_t Array::hash() const
    {
        assert(is_tuple);

        size_t h = 13;

        for(auto q : values) {
            auto v = vm->calculate_hash(q);
            h = h * 1299709 + v;
        }

        return h;
    }

    std::vector<OwcaValue> Array::sub_array(size_t from, size_t to) const
    {
        assert(from <= to);
        assert(to <= values.size());
        std::vector<OwcaValue> temp;
        temp.reserve(to - from);
        for(auto i = from; i < to; ++i) {
            temp.push_back(values[i]);
        }
        return temp;
    }
}