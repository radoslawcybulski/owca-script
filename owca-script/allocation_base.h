#ifndef RC_OWCA_SCRIPT_ALLOCATION_H
#define RC_OWCA_SCRIPT_ALLOCATION_H

#include "stdafx.h"

namespace OwcaScript {
	class OwcaValue;

	namespace Internal {
		class VM;

		struct GenerationGC {
			unsigned int value;

			explicit GenerationGC(unsigned int value) : value(value) {}

			bool operator == (GenerationGC other) const { return value == other.value; }
			bool operator != (GenerationGC other) const { return !(*this == other); }
		};
		struct AllocationBase {
#ifdef _DEBUG
			AllocationBase();
			virtual ~AllocationBase();
#else
			AllocationBase() = default;
			virtual ~AllocationBase() = default;
#endif
			AllocationBase(const AllocationBase&) = delete;
			AllocationBase(AllocationBase&&) = delete;

			AllocationBase &operator = (const AllocationBase&) = delete;
			AllocationBase &operator = (AllocationBase&&) = delete;

			AllocationBase* prev, * next;
			GenerationGC last_gc_mark = GenerationGC{ 0 };

			virtual std::string_view type() const = 0;
			virtual std::string to_string() const = 0;
			virtual void gc_mark(VM &vm, GenerationGC generation_gc) = 0;

			static unsigned int get_currently_remaining_allocations();
		};
		struct AllocationEmpty : public AllocationBase {
			std::string_view type() const override { return ""; }
			std::string to_string() const override { return ""; }
			void gc_mark(VM &vm, GenerationGC generation_gc) {}
		};
	}
}

#endif