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
