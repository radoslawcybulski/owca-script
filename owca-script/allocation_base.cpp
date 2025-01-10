#include "stdafx.h"
#include "allocation_base.h"
#include <atomic>

namespace OwcaScript::Internal {
#ifdef _DEBUG
	static std::atomic_uint32_t total{ 0 };

	AllocationBase::AllocationBase() {
		++total;
	}
	AllocationBase::~AllocationBase() {
		--total;
	}

	unsigned int AllocationBase::get_currently_remaining_allocations() {
		return total.load();
	}
#else
	unsigned int AllocationBase::get_currently_remaining_allocations() {
		return 0;
	}
#endif
}