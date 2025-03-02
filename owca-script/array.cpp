#include "stdafx.h"
#include "array.h"
#include "owca_value.h"
#include "vm.h"

namespace OwcaScript::Internal {
    std::string Array::to_string() const
    {
        if (values.empty()) return "[]";

        std::string temp;
        temp += "[";
        for(auto &q : values) {
            if (temp.size() > 1) temp += ",";
            temp += " ";
            temp += q.to_string();
        }
        temp += " ]";
        return temp;
    }

    void Array::gc_mark(OwcaVM vm, GenerationGC generation_gc)
    {
        VM::get(vm).gc_mark(values, generation_gc);
    }

    std::deque<OwcaValue> Array::sub_deque(size_t from, size_t to) const
    {
        assert(from <= to);
        assert(to <= values.size());
        std::deque<OwcaValue> temp;
        for(auto i = from; i < to; ++i) {
            temp.push_back(values[i]);
        }
        return temp;
    }

    // std::vector<OwcaValue> Array::sub_array(size_t from, size_t to) const
    // {
    //     assert(from <= to);
    //     assert(to <= values.size());
    //     std::vector<OwcaValue> temp;
    //     temp.reserve(to - from);
    //     for(auto i = from; i < to; ++i) {
    //         temp.push_back(values[i]);
    //     }
    //     return temp;
    // }
}