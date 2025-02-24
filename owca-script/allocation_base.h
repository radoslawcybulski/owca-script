#ifndef RC_OWCA_SCRIPT_ALLOCATION_H
#define RC_OWCA_SCRIPT_ALLOCATION_H

#include "stdafx.h"
#include "owca_vm.h"

namespace OwcaScript {
	class OwcaValue;

	namespace Internal {
		class VM;
		struct BoundFunctionSelfObject;

		struct AllocationBase {
			enum class Kind {
				User,
				String,
				RuntimeFunction,
				RuntimeFunctions,
				Map,
				Class,
				Tuple,
				Array,
				Set,
				BoundSelfObject,
				Exception,
			};
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

			AllocationBase* prev = nullptr, * next = nullptr;
			VM *vm = nullptr;
			GenerationGC last_gc_mark = GenerationGC{ 0 };
			Kind kind;

			virtual std::string_view type() const = 0;
			virtual std::string to_string() const = 0;
			virtual void gc_mark(VM &vm, GenerationGC generation_gc) = 0;
			virtual BoundFunctionSelfObject* is_bound_function_self_object() { return nullptr; }

			static unsigned int get_currently_remaining_allocations();
		};

		struct AllocationEmpty : public AllocationBase {
			static constexpr const Kind object_kind = Kind::User;

			friend class VM;

			std::string_view type() const override { return ""; }
			std::string to_string() const override { return ""; }

		private:
			void gc_mark(VM &vm, GenerationGC generation_gc) {}
		};
	}
}

#endif