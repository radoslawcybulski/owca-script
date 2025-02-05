#ifndef RC_OWCA_SCRIPT_ARRAY_H
#define RC_OWCA_SCRIPT_ARRAY_H

#include "stdafx.h"
#include "allocation_base.h"

namespace OwcaScript {
	class OwcaVM;

	namespace Internal {
		struct Array : public AllocationBase {
            using AllocationBase::AllocationBase;

            std::vector<OwcaValue> values;
            const bool is_tuple = false;
			
            Array(std::vector<OwcaValue> values, bool is_tuple = false) : values(std::move(values)), is_tuple(is_tuple) {}

			std::string_view type() const override {
				return is_tuple ? "tuple" : "array";
			}
            std::vector<OwcaValue> sub_array(size_t from, size_t to) const;
			std::string to_string() const override;
			void gc_mark(VM& vm, GenerationGC generation_gc) override;
            size_t hash() const;
		};
	}
}

#endif
