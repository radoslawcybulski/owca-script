#ifndef RC_OWCA_SCRIPT_ITERATOR_H
#define RC_OWCA_SCRIPT_ITERATOR_H

#include "runtime_function.h"
#include "stdafx.h"
#include "allocation_base.h"
#include "impl_base.h"
#include "owca_iterator.h"
#include "owca_value.h"
#include "owca_variable.h"
#include "generator.h"
#include "executor.h"

namespace OwcaScript {
	class OwcaVM;

	namespace Internal {
		struct Iterator : public AllocationBase {
            static constexpr const Kind object_kind = Kind::Iterator;

			std::optional<Generator> generator;
			RuntimeFunction *function;
			std::span<OwcaValue> values;
			std::span<Executor::StatesType> states;
			OwcaValue last_value;
			bool first_time = true;

            Iterator(RuntimeFunction *function, std::span<OwcaValue> values, std::span<Executor::StatesType> states);
			~Iterator();

            // OwcaValue execute_next();
			std::string_view type() const override{
				return "Iterator";
			}
			std::string to_string() const override;
			void gc_mark(OwcaVM vm, GenerationGC generation_gc) const override;
		};
	}
}

#endif
