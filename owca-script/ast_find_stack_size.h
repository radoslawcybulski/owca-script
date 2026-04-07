#ifndef RC_OWCA_SCRIPT_AST_FIND_STACK_SIZE_H
#define RC_OWCA_SCRIPT_AST_FIND_STACK_SIZE_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
        struct FindStackSizeVisitor : public AstVisitor {
            unsigned int current_stack_size = 0;
            unsigned int max_stack_size = 0;

            struct AssertTheSameSize {
                FindStackSizeVisitor &visitor;
                unsigned int size;

                AssertTheSameSize(FindStackSizeVisitor &visitor) : visitor(visitor), size(visitor.current_stack_size) {}
                ~AssertTheSameSize() {
                    assert(visitor.current_stack_size == size);
                }
            };
            void push() {
                ++current_stack_size;
                if (current_stack_size > max_stack_size) {
                    max_stack_size = current_stack_size;
                }
            }
            void pop(unsigned int c = 1) {
                assert(current_stack_size >= c);
                current_stack_size -= c;
            }
            FindStackSizeVisitor() {}

			void apply(AstBlock &o) override;
			void apply(AstExprAsStat &o) override;
			void apply(AstExprCompare &o) override;
			void apply(AstExprConstant &o) override;
			void apply(AstExprIdentifier &o) override;
			void apply(AstExprMember &o) override;
			void apply(AstExprOper1 &o) override;
			void apply(AstExprOper2 &o) override;
			void apply(AstExprOperX &o) override;
			void apply(AstFunction& o) override;
			void apply(AstClass& o) override;
			void apply(AstReturn& o) override;
			void apply(AstIf& o) override;
			void apply(AstWhile& o) override;
			void apply(AstFor& o) override;
			void apply(AstLoopControl& o) override;
			void apply(AstTry& o) override;
			void apply(AstThrow& o) override;
			void apply(AstWith &o) override;
			void apply(AstYield &o) override;
        };
    }
}

#endif
