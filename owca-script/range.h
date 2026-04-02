#ifndef RC_OWCA_SCRIPT_RANGE_H
#define RC_OWCA_SCRIPT_RANGE_H

#include "stdafx.h"
#include "allocation_base.h"
#include "owca_value.h"

namespace OwcaScript {
	class OwcaVM;

	namespace Internal {
		struct Range : public AllocationBase {
			static constexpr const Kind object_kind = Kind::Range;

            using AllocationBase::AllocationBase;

            Number from = 0, to = 0, step = 1;
            
			std::string_view type() const override {
				return "Range";
			}
			std::string to_string() const override;
            Generator iter(OwcaVM vm) const;
			void gc_mark(OwcaVM vm, GenerationGC generation_gc) const override {}
            size_t hash() const;
		};
        struct RangeIterator {
            Number lower, upper, step;
            
            RangeIterator(Number lower, Number upper, Number step) : lower(lower), upper(upper), step(step) {}

            bool done() const {
                if (step > 0) return lower >= upper;
                else return lower <= upper;
            }

            Number get() const { return lower; }
            void next() { lower += step; }
        };
	}
}

#endif
