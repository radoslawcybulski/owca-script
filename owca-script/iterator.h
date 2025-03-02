#ifndef RC_OWCA_SCRIPT_ITERATOR_H
#define RC_OWCA_SCRIPT_ITERATOR_H

#include "stdafx.h"
#include "allocation_base.h"
#include "execution_frame.h"
#include "impl_base.h"
#include "owca_iterator.h"
#include "owca_value.h"
#include "owca_variable.h"

namespace OwcaScript {
	class OwcaVM;

	namespace Internal {
        class CodeBuffer;

		struct Iterator : public AllocationBase {
            static constexpr const Kind object_kind = Kind::Iterator;

            ExecutionFrame frame;
            ImplStat::State state;
            std::optional<Generator> generator;
			OwcaVariableSet variable_set;
			std::vector<AllocationBase*> allocated_objects;

            Iterator(size_t sz, Line line) : frame(line), state(sz) {}

            OwcaValue execute_next();
			std::string_view type() const override{
				return "Iterator";
			}
			std::string to_string() const override;
			void gc_mark(OwcaVM vm, GenerationGC generation_gc) override;
		};
	}
}

#endif
