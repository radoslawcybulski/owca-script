#ifndef RC_OWCA_SCRIPT_AST_LOOP_CONTROL_H
#define RC_OWCA_SCRIPT_AST_LOOP_CONTROL_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstLoopControl : public AstStat {
        public:
            enum class Mode {
                Break, Continue
            };
		private:
			unsigned int depth;
            Mode mode;
		public:        
			AstLoopControl(Line line, Mode mode, unsigned int depth) : AstStat(line), depth(depth), mode(mode) {}

			ImplStat* emit(EmitInfo& ei) override;
			void calculate_size(CodeBufferSizeCalculator &) const override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
