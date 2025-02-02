#ifndef RC_OWCA_SCRIPT_AST_VISITOR_H
#define RC_OWCA_SCRIPT_AST_VISITOR_H

#include "stdafx.h"

namespace OwcaScript {
	namespace Internal {
		class AstBase;
		class AstExpr;
		class AstStat;
		class AstBlock;
		class AstExprAsStat;
		class AstExprCompare;
		class AstExprConstant;
		class AstExprIdentifier;
		class AstExprMember;
		class AstExprOper1;
		class AstExprOper2;
		class AstExprOperX;
		class AstFunction;
		class AstClass;
		class AstReturn;
		class AstIf;
		class AstWhile;
		class AstLoopControl;

		struct AstVisitor {
			virtual ~AstVisitor() = default;
			
			virtual void apply(AstBase &o) { }
			virtual void apply(AstExpr &o);
			virtual void apply(AstStat &o);
			virtual void apply(AstBlock &o);
			virtual void apply(AstExprAsStat &o);
			virtual void apply(AstExprCompare &o);
			virtual void apply(AstExprConstant &o);
			virtual void apply(AstExprIdentifier &o);
			virtual void apply(AstExprMember &o);
			virtual void apply(AstExprOper1 &o);
			virtual void apply(AstExprOper2 &o);
			virtual void apply(AstExprOperX &o);
			virtual void apply(AstFunction& o);
			virtual void apply(AstClass& o);
			virtual void apply(AstReturn& o);
			virtual void apply(AstIf& o);
			virtual void apply(AstWhile& o);
			virtual void apply(AstLoopControl& o);
		};
	}
}

#endif
