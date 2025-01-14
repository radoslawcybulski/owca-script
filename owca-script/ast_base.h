#ifndef RC_OWCA_SCRIPT_AST_BASE_H
#define RC_OWCA_SCRIPT_AST_BASE_H

#include "stdafx.h"
#include "code_buffer.h"
#include "impl_base.h"
#include "ast_visitor.h"
#include "line.h"

namespace OwcaScript {
	namespace Internal {
		class AstBase {
		public:
			const Line line;

			struct EmitInfo {
				CodeBuffer code_buffer;

				template <typename T> T* preallocate() {
					return code_buffer.preallocate<T>();
				}
			};

			AstBase(Line line) : line(line) {}

			virtual ~AstBase() = default;

			virtual ImplBase* emit(EmitInfo& ei) = 0;
			virtual void calculate_size(CodeBufferSizeCalculator &) const = 0;
			virtual void visit(AstVisitor&) = 0;
			virtual void visit_children(AstVisitor&) = 0;
		};

		class AstStat : public AstBase {
		public:
			using AstBase::AstBase;

			virtual ImplStat* emit(EmitInfo& ei) = 0;
		};

		class AstExpr : public AstBase {
		public:
			using AstBase::AstBase;

			virtual ImplExpr* emit(EmitInfo& ei) = 0;
		};
	}
}

#endif
