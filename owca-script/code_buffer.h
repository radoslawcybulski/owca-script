#ifndef RC_OWCA_SCRIPT_OUTPUT_BUFFER_H
#define RC_OWCA_SCRIPT_OUTPUT_BUFFER_H

#include "stdafx.h"
#include "line.h"

namespace OwcaScript {
	namespace Internal {
		class ImplBase;
		class ImplExpr;

		class CodeBuffer {
			std::vector<char> storage;
			size_t offset = 0;
			bool validate_offset = false;

			char* get_ptr(size_t size, size_t align) {
				offset = (offset + align - 1) & ~(align - 1);
				if (storage.size() < offset + size) {
					storage.resize(offset + size);
				}
				char* ptr = storage.data() + offset;
				offset += size;
				return ptr;
			}

			void* allocate_simple_with_copy(const void* source, size_t size_alloc, size_t size_copy, size_t align);
		public:
			CodeBuffer() = default;

			template <typename T> T* preallocate(Line line) {
				auto p = get_ptr(sizeof(T), alignof(T));
				if (validate_offset)
					assert(offset <= storage.size());
				return new (p) T(line);
			}

			void resize_for_final_output();
			std::string_view allocate(std::string_view txt);
			template <typename T> std::span<T> preallocate_array(size_t sz) {
				auto p = get_ptr(sz * sizeof(T), alignof(T));
				return std::span{ (T*)p, sz };
			}
			const ImplExpr* root() const { return (const ImplExpr*)storage.data(); }
		};
	}
}

#endif
