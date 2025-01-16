#ifndef RC_OWCA_SCRIPT_OUTPUT_BUFFER_H
#define RC_OWCA_SCRIPT_OUTPUT_BUFFER_H

#include "stdafx.h"
#include "line.h"
#include "owca_class.h"

namespace OwcaScript {
	void check_memory();

	namespace Internal {
		class ImplBase;
		class ImplExpr;

		class CodeBuffer {
			std::string filename_;
			std::vector<char> storage;
			std::vector<std::unique_ptr<OwcaClass::NativeClassInterface>> native_interfaces;
			size_t offset = 0;

			char* get_ptr(size_t size, size_t align) {
				offset = (offset + align - 1) & ~(align - 1);
				assert(offset + size <= storage.size());
				char* ptr = storage.data() + offset;
				offset += size;
				return ptr;
			}

			void* allocate_simple_with_copy(const void* source, size_t size_alloc, size_t size_copy, size_t align);
		public:
			CodeBuffer(std::string filename, size_t size) : filename_(std::move(filename)) {
				storage.resize(size);
			}

			std::string_view filename() const { return filename_; }
			void validate_size(size_t size);
			template <typename T> T* preallocate(Line line) {
				auto p = get_ptr(sizeof(T), alignof(T));
				return new (p) T(line);
			}

			std::string_view allocate(std::string_view txt);
			template <typename T> std::span<T> preallocate_array(size_t sz) {
				auto p = get_ptr(sz * sizeof(T), alignof(T));
				return std::span{ (T*)p, sz };
			}
			const ImplExpr* root() const { return (const ImplExpr*)storage.data(); }
			void register_native_interface(std::unique_ptr<OwcaClass::NativeClassInterface> native);
		};

		class CodeBufferSizeCalculator {
		public:
			struct CodeBufTemp {
				CodeBufferSizeCalculator* self;

				size_t offset = 0;

				void get_ptr(size_t size, size_t align) {
					offset = (offset + align - 1) & ~(align - 1);
					offset += size;
				}

				template <typename T> void preallocate(Line line) {
					get_ptr(sizeof(T), alignof(T));
				}

				void allocate(std::string_view txt) {
					get_ptr(txt.size() + 1, 1);
				}
				template <typename T> void preallocate_array(size_t sz) {
					get_ptr(sz * sizeof(T), alignof(T));
				}
			};
			CodeBufTemp code_buffer{ this };

			auto get_total_size() const { return code_buffer.offset; }
		};
	}
}

#endif
