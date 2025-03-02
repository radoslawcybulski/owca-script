#ifndef RC_OWCA_SCRIPT_ARRAY_H
#define RC_OWCA_SCRIPT_ARRAY_H

#include "stdafx.h"
#include "allocation_base.h"

namespace OwcaScript {
	class OwcaVM;

	namespace Internal {
		struct Array : public AllocationBase {
			static constexpr const Kind object_kind = Kind::Array;

            using AllocationBase::AllocationBase;

            std::deque<OwcaValue> values;
            const bool is_tuple = false;
			
            Array(std::deque<OwcaValue> values) : values(std::move(values)) {}

			std::string_view type() const override {
				return "Array";
			}
            //std::vector<OwcaValue> sub_array(size_t from, size_t to) const;
			std::deque<OwcaValue> sub_deque(size_t from, size_t to) const;
			std::string to_string() const override;
			void gc_mark(OwcaVM vm, GenerationGC generation_gc) override;
		};
	}
}

#endif
