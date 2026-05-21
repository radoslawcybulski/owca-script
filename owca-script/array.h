#ifndef RC_OWCA_SCRIPT_ARRAY_H
#define RC_OWCA_SCRIPT_ARRAY_H

#include "stdafx.h"
#include "allocation_base.h"

namespace OwcaScript {
	class OwcaVM;

	namespace Internal {
		struct Array : public AllocationBase {
            std::deque<OwcaValue> values;
            const bool is_tuple = false;
			
            Array(VM *vm, std::deque<OwcaValue> values = {}) : AllocationBase(vm, Kind::Array), values(std::move(values)) {}

			std::string_view type() const override {
				return "Array";
			}
			std::deque<OwcaValue> sub_deque(size_t from, size_t to) const;
			std::string to_string() const override;
			void gc_mark(const OwcaVM &vm, GenerationGC generation_gc) const override;
		};
	}
}

#endif
