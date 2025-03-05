#include "stdafx.h"
#include "code_buffer.h"
#include "impl_base.h"

namespace OwcaScript::Internal {
    std::span<std::function<ImplExpr*(Deserializer&, Line)>> get_expression_constructors();
    std::span<std::function<ImplStat*(Deserializer&, Line)>> get_statement_constructors();

	CodeBuffer::CodeBuffer(std::string filename, std::span<unsigned char> storage, std::unique_ptr<OwcaVM::NativeCodeProvider> native_code_provider) : filename_(std::move(filename)), native_code_provider_(std::move(native_code_provider)) {
		auto ser = Deserializer{ filename_, storage, get_statement_constructors(), get_expression_constructors() };
		ImplExpr *expr;
		ser.deserialize(expr);
		this->storage = ser.take_loaded_data();
		//assert(root() == expr);
	}
	char* CodeBuffer::get_ptr(size_t size, size_t align) {
		offset = (offset + align - 1) & ~(align - 1);
		//std::cout << "Allocate " << offset << " " << size << "\n";
		assert(offset + size <= storage.size());
		char* ptr = storage.data() + offset;
		offset += size;
		return ptr;
	}
	void CodeBufferSizeCalculator::CodeBufTemp::get_ptr(size_t size, size_t align) {
		offset = (offset + align - 1) & ~(align - 1);
		//std::cout << "Calculate " << offset << " " << size << "\n";
		offset += size;
	}
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

	std::vector<unsigned char> CodeBuffer::serialize()
	{
		auto r = root();
		auto ser = Serializer{ storage.size() };
		ser.serialize(r);
		return ser.take_result();
	}

	bool CodeBuffer::compare(const CodeBuffer &other) const {
		return root()->compare(other.root());
	}
}

#include <Windows.h>

void OwcaScript::check_memory()
{
	_ASSERTE( _CrtCheckMemory( ) );
}
