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

    void Tuple::gc_mark(OwcaVM vm, GenerationGC generation_gc) const
    {
        gc_mark_value(vm, generation_gc, values);
    }

    size_t Tuple::hash() const
    {
        if (!hash_value_calculated) {
            size_t h = 13;

            for(auto q : values) {
                auto v = vm->calculate_hash(q);
                h = h * 1299709 + v;
            }
            hash_value = h;
            hash_value_calculated = true;
        }
        return hash_value;
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