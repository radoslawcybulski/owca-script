#ifndef RC_OWCA_SCRIPT_TUPLE_H
#define RC_OWCA_SCRIPT_TUPLE_H

#include "stdafx.h"
#include "allocation_base.h"
#include "owca_value.h"

namespace OwcaScript {
	class OwcaVM;

	namespace Internal {
		struct Tuple : public AllocationBase {
            std::vector<OwcaValue> values;
			mutable size_t hash_value = 0;
			mutable bool hash_value_calculated = false;
			
            Tuple(VM *vm, std::vector<OwcaValue> values = {}) : AllocationBase(vm, Kind::Tuple), values(std::move(values)) {}

			std::string_view type() const override {
				return "Tuple";
			}
            std::vector<OwcaValue> sub_array(size_t from, size_t to) const;
			std::string to_string() const override;
			void gc_mark(const OwcaVM &vm, GenerationGC generation_gc) const override;
            size_t hash() const;
		};
	}
}

#endif
