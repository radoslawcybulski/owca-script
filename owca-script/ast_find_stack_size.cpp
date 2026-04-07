#include "stdafx.h"
#include "ast_find_stack_size.h"
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

namespace OwcaScript {
    namespace Internal {
        void FindStackSizeVisitor::apply(AstBlock &o) {
            auto _ = AssertTheSameSize(*this);
            o.visit_children(*this);
        }
        void FindStackSizeVisitor::apply(AstExprAsStat &o) {
            auto _ = AssertTheSameSize(*this);
            o.visit_children(*this);
            pop();
        }
        void FindStackSizeVisitor::apply(AstExprCompare &o) {
            o.first().visit(*this);
            for(auto &n : o.nexts()) {
                std::get<2>(n)->visit(*this);
                pop();
            }
        }
        void FindStackSizeVisitor::apply(AstExprConstant &o) {
            push();
        }
        void FindStackSizeVisitor::apply(AstExprIdentifier &o) {
            if (o.write()) {
                o.value_to_write().visit(*this);
            }
            else {
                push();
            }
        }
        void FindStackSizeVisitor::apply(AstExprMember &o) {
            o.value().visit(*this);
            if (o.write()) {
                o.value_to_write().visit(*this);
                pop();
            }
        }
        void FindStackSizeVisitor::apply(AstExprOper1 &o) {
            o.left().visit(*this);
        }
        void FindStackSizeVisitor::apply(AstExprOper2 &o) {
            o.left().visit(*this);
            o.right().visit(*this);
            if (o.has_third()) {
                o.third().visit(*this);
            }
            pop(o.has_third() ? 2 : 1);
        }
        void FindStackSizeVisitor::apply(AstExprOperX &o) {
            for(auto &a : o.args()) {
                a->visit(*this);
            }
            pop(o.args().size() - 1);
        }
        void FindStackSizeVisitor::apply(AstFunction& o) {
            FindStackSizeVisitor vis;
            o.body().visit(vis);
            assert(vis.current_stack_size == 0);
            o.update_max_stack_size(vis.max_stack_size);
            push();
        }
        void FindStackSizeVisitor::apply(AstClass& o) {
            for(auto &b : o.base_classes()) {
                b->visit(*this);
                pop();
            }
            for(auto &m : o.members()) {
                m->visit(*this);
                pop();
            }
            push();
        }
        void FindStackSizeVisitor::apply(AstReturn& o) {
            auto _ = AssertTheSameSize(*this);
            if (o.has_value()) {
                o.value().visit(*this);
                pop();
            }
        }
        void FindStackSizeVisitor::apply(AstIf& o) {
            auto _ = AssertTheSameSize(*this);
            o.value().visit(*this);
            pop();
            o.if_true().visit(*this);
            o.if_false().visit(*this);
        }
        void FindStackSizeVisitor::apply(AstWhile& o) {
            auto _ = AssertTheSameSize(*this);
            o.value().visit(*this);
            pop();
            o.body().visit(*this);
        }
        void FindStackSizeVisitor::apply(AstFor& o) {
            auto _ = AssertTheSameSize(*this);
            o.iterator().visit(*this);
            o.body().visit(*this);
            pop(); // we pop iterator at end
        }
        void FindStackSizeVisitor::apply(AstLoopControl& o) {
        }
        void FindStackSizeVisitor::apply(AstTry& o) {
            auto _ = AssertTheSameSize(*this);
            o.body().visit(*this);
            for(auto &c : o.catches()) {
                for(auto &e : std::get<2>(c)) {
                    e->visit(*this);
                    pop();
                }
                std::get<3>(c)->visit(*this);
            }
        }
        void FindStackSizeVisitor::apply(AstThrow& o) {
            auto _ = AssertTheSameSize(*this);
            o.value().visit(*this);
            pop();
        }
        void FindStackSizeVisitor::apply(AstWith &o) {
            auto _ = AssertTheSameSize(*this);
            o.value().visit(*this);
            pop();
            o.body().visit(*this);
        }
        void FindStackSizeVisitor::apply(AstYield &o) {
            auto _ = AssertTheSameSize(*this);
            o.value().visit(*this);
            pop();
        }
    }
}