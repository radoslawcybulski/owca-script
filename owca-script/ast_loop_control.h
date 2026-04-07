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
			std::uint8_t depth_;
            Mode mode_;
		public:        
			AstLoopControl(Line line, Mode mode, std::uint8_t depth) : AstStat(line), depth_(depth), mode_(mode) {}

			auto mode() const { return mode_; }
			auto depth() const { return depth_; }

			void emit(EmitInfo& ei) override;

			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
