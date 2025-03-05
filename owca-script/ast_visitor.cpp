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
#include "ast_for.h"
#include "ast_loop_control.h"
#include "ast_try.h"
#include "ast_throw.h"
#include "ast_with.h"
#include "ast_yield.h"

namespace OwcaScript::Internal {
    std::span<std::function<ImplExpr*(Deserializer&, Line)>> get_expression_constructors() {
        static std::array<std::function<ImplExpr*(Deserializer&, Line)>, (unsigned int)ImplExpr::Kind::_Count> a = []() {
            std::array<std::function<ImplExpr*(Deserializer&, Line)>, (unsigned int)ImplExpr::Kind::_Count> tmp;

			AstExprCompare::initialize_serialization_functions({ tmp.begin(), tmp.end() });
			AstExprConstant::initialize_serialization_functions({ tmp.begin(), tmp.end() });
			AstExprIdentifier::initialize_serialization_functions({ tmp.begin(), tmp.end() });
			AstExprMember::initialize_serialization_functions({ tmp.begin(), tmp.end() });
			AstExprOper1::initialize_serialization_functions({ tmp.begin(), tmp.end() });
			AstExprOper2::initialize_serialization_functions({ tmp.begin(), tmp.end() });
			AstExprOperX::initialize_serialization_functions({ tmp.begin(), tmp.end() });
			AstFunction::initialize_serialization_functions({ tmp.begin(), tmp.end() });
			AstClass::initialize_serialization_functions({ tmp.begin(), tmp.end() });

			for(auto &q : tmp) assert(q);

			return tmp;
        }();
		return a;
    }
    std::span<std::function<ImplStat*(Deserializer&, Line)>> get_statement_constructors() {
        static std::array<std::function<ImplStat*(Deserializer&, Line)>, (unsigned int)ImplStat::Kind::_Count> a = []() {
            std::array<std::function<ImplStat*(Deserializer&, Line)>, (unsigned int)ImplStat::Kind::_Count> tmp;

			AstBlock::initialize_serialization_functions({ tmp.begin(), tmp.end() });
			AstExprAsStat::initialize_serialization_functions({ tmp.begin(), tmp.end() });
			AstReturn::initialize_serialization_functions({ tmp.begin(), tmp.end() });
			AstIf::initialize_serialization_functions({ tmp.begin(), tmp.end() });
			AstWhile::initialize_serialization_functions({ tmp.begin(), tmp.end() });
			AstFor::initialize_serialization_functions({ tmp.begin(), tmp.end() });
			AstLoopControl::initialize_serialization_functions({ tmp.begin(), tmp.end() });
			AstTry::initialize_serialization_functions({ tmp.begin(), tmp.end() });
			AstThrow::initialize_serialization_functions({ tmp.begin(), tmp.end() });
			AstWith::initialize_serialization_functions({ tmp.begin(), tmp.end() });
			AstYield::initialize_serialization_functions({ tmp.begin(), tmp.end() });
			
			for(auto &q : tmp) assert(q);

            return tmp;
        }();
		return a;
    }

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

	void AstVisitor::apply(AstFor &o)
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

	void AstVisitor::apply(AstWith &o)
	{
		apply(static_cast<AstStat&>(o));
	}

	void AstVisitor::apply(AstYield &o)
	{
		apply(static_cast<AstStat&>(o));
	}
}
