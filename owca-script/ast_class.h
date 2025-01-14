//#ifndef RC_OWCA_SCRIPT_AST_CLASS_H
//#define RC_OWCA_SCRIPT_AST_CLASS_H
//
//#include "stdafx.h"
//#include "ast_base.h"
//
//namespace OwcaScript {
//	namespace Internal {
//		class AstClass : public AstExpr {
//			std::vector<std::unique_ptr<AstStat>> children;
//
//		public:
//			AstBlock(Line line, std::vector<std::unique_ptr<AstStat>> children) : AstStat(line), children(std::move(children)) {}
//
//			ImplStat* emit(EmitInfo& ei) override;
//			void visit(AstVisitor&) override;
//			void visit_children(AstVisitor&) override;
//		};
//	}
//}
//
//#endif
