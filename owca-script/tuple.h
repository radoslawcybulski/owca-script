#ifndef RC_OWCA_SCRIPT_TUPLE_H
#define RC_OWCA_SCRIPT_TUPLE_H

#include "stdafx.h"
#include "allocation_base.h"

namespace OwcaScript {
	class OwcaVM;

	namespace Internal {
		struct Tuple : public AllocationBase {
			static constexpr const Kind object_kind = Kind::Tuple;

            using AllocationBase::AllocationBase;

            std::vector<OwcaValue> values;
			
            Tuple(std::vector<OwcaValue> values) : values(std::move(values)) {}

			std::string_view type() const override {
				return "Tuple";
			}
            std::vector<OwcaValue> sub_array(size_t from, size_t to) const;
			std::string to_string() const override;
			void gc_mark(OwcaVM vm, GenerationGC generation_gc) override;
            size_t hash() const;
		};
	}
}

#endif
