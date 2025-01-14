#include "stdafx.h"
#include "code_buffer.h"

namespace OwcaScript::Internal {
	void* CodeBuffer::allocate_simple_with_copy(const void* source, size_t size_alloc, size_t size_copy, size_t align)
	{
		auto p = get_ptr(size_alloc, align);
		std::memcpy(p, source, size_copy);
		return p;
	}

	std::string_view CodeBuffer::allocate(std::string_view txt)
	{
		auto p = allocate_simple_with_copy(txt.data(), txt.size() + 1, txt.size(), 1);
		((char*)p)[txt.size()] = 0;
		return std::string_view{ (char*)p, txt.size() };
	}

	void CodeBuffer::validate_size(size_t size)
	{
		assert(storage.size() == size);
		assert(offset == size);
	}
}

#include <Windows.h>

void OwcaScript::check_memory()
{
	_ASSERTE( _CrtCheckMemory( ) );
}
