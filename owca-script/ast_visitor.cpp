#include "stdafx.h"
#include "ast_visitor.h"
#include "ast_block.h"
#include "ast_expr_as_stat.h"
#include "ast_expr_compare.h"
#include "ast_expr_constant.h"
#include "ast_expr_identifier.h"
#include "ast_expr_member.h"
#include "ast_expr_oper_1.h"
#include "ast_expr_oper_2.h"
#include "ast_expr_oper_x.h"
#include "ast_function.h"
#include "ast_class.h"
#include "ast_return.h"
#include "ast_if.h"
#include "ast_while.h"
#include "ast_loop_control.h"
#include "ast_try.h"
#include "ast_throw.h"

namespace OwcaScript::Internal {
	void AstVisitor::apply(AstExpr &o)
	{
		apply(static_cast<AstBase&>(o));
	}

	void AstVisitor::apply(AstStat &o)
	{
		apply(static_cast<AstBase&>(o));
	}

	void AstVisitor::apply(AstBlock &o)
	{
		apply(static_cast<AstStat&>(o));
	}

	void AstVisitor::apply(AstExprAsStat &o)
	{
		apply(static_cast<AstStat&>(o));
	}

	void AstVisitor::apply(AstExprCompare &o)
	{
		apply(static_cast<AstExpr&>(o));
	}

	void AstVisitor::apply(AstExprConstant &o)
	{
		apply(static_cast<AstExpr&>(o));
	}

	void AstVisitor::apply(AstExprIdentifier &o)
	{
		apply(static_cast<AstExpr&>(o));
	}

	void AstVisitor::apply(AstExprMember &o)
	{
		apply(static_cast<AstExpr&>(o));
	}

	void AstVisitor::apply(AstExprOper1 &o)
	{
		apply(static_cast<AstExpr&>(o));
	}

	void AstVisitor::apply(AstExprOper2 &o)
	{
		apply(static_cast<AstExpr&>(o));
	}

	void AstVisitor::apply(AstExprOperX &o)
	{
		apply(static_cast<AstExpr&>(o));
	}

	void AstVisitor::apply(AstFunction &o)
	{
		apply(static_cast<AstExpr&>(o));
	}

	void AstVisitor::apply(AstClass &o)
	{
		apply(static_cast<AstExpr&>(o));
	}

	void AstVisitor::apply(AstReturn &o)
	{
		apply(static_cast<AstStat&>(o));
	}

	void AstVisitor::apply(AstIf &o)
	{
		apply(static_cast<AstStat&>(o));
	}

	void AstVisitor::apply(AstWhile &o)
	{
		apply(static_cast<AstStat&>(o));
	}

	void AstVisitor::apply(AstLoopControl &o)
	{
		apply(static_cast<AstStat&>(o));
	}

	void AstVisitor::apply(AstTry &o)
	{
		apply(static_cast<AstStat&>(o));
	}

	void AstVisitor::apply(AstThrow &o)
	{
		apply(static_cast<AstStat&>(o));
	}
}
