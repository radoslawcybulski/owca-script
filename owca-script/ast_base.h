#ifndef RC_OWCA_SCRIPT_AST_BASE_H
#define RC_OWCA_SCRIPT_AST_BASE_H

#include "stdafx.h" 
#include "impl_base.h"
#include "ast_visitor.h"
#include "line.h"
#include "exec_buffer.h"

namespace OwcaScript {
	namespace Internal {
		class AstCompiler;

		class AstBase {
		public:
			const Line line;

			struct EmitInfo {
				class MaxCounter {
					unsigned int max = 0, current = 0;
				public:
					void push() {
						++current;
						if (current > max) {
							max = current;
						}
					}
					void pop(size_t s = 1) {
						assert(current >= s);
						current -= s;
					}
					auto maximum() const { return max; }
					bool empty() const { return current == 0; }
				};
				ExecuteBufferWriter code_writer;
				MaxCounter stack, states;
				AstCompiler &compiler;
				bool generator = false;
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
