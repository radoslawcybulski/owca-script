#ifndef RC_OWCA_SCRIPT_AST_BASE_H
#define RC_OWCA_SCRIPT_AST_BASE_H

#include "stdafx.h"
#include "impl_base.h"
#include "ast_visitor.h"
#include "line.h"
#include "exec_buffer.h"

namespace OwcaScript {
	namespace Internal {
		class AstBase {
		public:
			const Line line;

			struct EmitInfo {
				ExecuteBufferWriter code_writer;
				std::uint32_t max_storage_counter = 0;
				std::uint32_t current_storage_counter = 0;
				std::uint32_t max_stack_size = 0;
				std::uint32_t current_stack_size = 0;

				struct Info {
					std::uint32_t max_storage_counter = 0;
					std::uint32_t current_storage_counter = 0;
					std::uint32_t max_stack_size = 0;
					std::uint32_t current_stack_size = 0;
				};

				Info take_and_reset() {
					Info info{ max_storage_counter, current_storage_counter, max_stack_size, current_stack_size };
					max_storage_counter = 0;
					current_storage_counter = 0;
					max_stack_size = 0;
					current_stack_size = 0;
					return info;
				}
				void restore_info(Info info) {
					max_storage_counter = info.max_storage_counter;
					current_storage_counter = info.current_storage_counter;
					max_stack_size = info.max_stack_size;
					current_stack_size = info.current_stack_size;
				}
				void push_stack() {
					current_stack_size++;
					if (current_stack_size > max_stack_size) {
						max_stack_size = current_stack_size;
					}
				}
				void pop_stack() {
					assert(current_stack_size > 0);
					current_stack_size--;
				}
				void push_storage() {
					current_storage_counter++;
					if (current_storage_counter > max_storage_counter) {
						max_storage_counter = current_storage_counter;
					}
				}
				void pop_storage() {
					assert(current_storage_counter > 0);
					current_storage_counter--;
				}
			};

			AstBase(Line line) : line(line) {}

			virtual ~AstBase() = default;

			virtual void emit(EmitInfo& ei) = 0;
			virtual void visit(AstVisitor&) = 0;
			virtual void visit_children(AstVisitor&) = 0;
		};

		class AstStat : public AstBase {
		public:
			using AstBase::AstBase;

			virtual void emit(EmitInfo& ei) = 0;
		};

		class AstExpr : public AstBase {
		public:
			using AstBase::AstBase;

			virtual void emit(EmitInfo& ei) = 0;
		};
	}
}

#endif
