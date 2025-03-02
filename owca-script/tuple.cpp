#include "stdafx.h"
#include "tuple.h"
#include "owca_value.h"
#include "vm.h"

namespace OwcaScript::Internal {
    std::string Tuple::to_string() const
    {
        if (values.empty()) return "()";

        std::string temp;
        temp += "(";
        for(auto &q : values) {
            temp += ",";
            temp += " ";
            temp += q.to_string();
        }
        temp += " )";
        return temp;
    }

    void Tuple::gc_mark(OwcaVM vm, GenerationGC generation_gc)
    {
        VM::get(vm).gc_mark(values, generation_gc);
    }

    size_t Tuple::hash() const
    {
        size_t h = 13;

        for(auto q : values) {
            auto v = vm->calculate_hash(q);
            h = h * 1299709 + v;
        }

        return h;
    }

    std::vector<OwcaValue> Tuple::sub_array(size_t from, size_t to) const
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