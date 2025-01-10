#include "stdafx.h"
#include "ast_expr_constant.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	class ImplExprConstantEmpty : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		void init() {
		}
		OwcaValue execute(OwcaVM &vm) const override {
			return OwcaEmpty{};
		}
	};
	class ImplExprConstantBool : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;
		bool value;

		void init(bool value) {
			this->value = value;
		}
		OwcaValue execute(OwcaVM &vm) const override {
			return OwcaBool{ value };
		}
	};
	class ImplExprConstantInt : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;
		OwcaIntInternal value;

		void init(OwcaIntInternal value) {
			this->value = value;
		}
		OwcaValue execute(OwcaVM &vm) const override {
			return OwcaInt{ value };
		}
	};
	class ImplExprConstantFloat : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;
		OwcaFloatInternal value;

		void init(OwcaFloatInternal value) {
			this->value = value;
		}
		OwcaValue execute(OwcaVM &vm) const override {
			return OwcaFloat{ value };
		}
	};
	class ImplExprConstantString : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;
		OwcaStringInternal value;

		void init(OwcaStringInternal value) {
			this->value = std::move(value);
		}
		OwcaValue execute(OwcaVM &vm) const override {
			return OwcaString{ value };
		}
	};

	ImplExpr* AstExprConstant::emit(EmitInfo& ei) {
		return value.visit(
			[&](OwcaEmpty) -> ImplExpr* {
				auto ret = ei.code_buffer.preallocate<ImplExprConstantEmpty>(line);
				ret->init();
				return ret;
			},
			[&](OwcaBool o) -> ImplExpr* {
				auto ret = ei.code_buffer.preallocate<ImplExprConstantBool>(line);
				ret->init(o.internal_value());
				return ret;

			},
			[&](OwcaInt o) -> ImplExpr* {
				auto ret = ei.code_buffer.preallocate<ImplExprConstantInt>(line);
				ret->init(o.internal_value());
				return ret;

			},
			[&](OwcaFloat o) -> ImplExpr* {
				auto ret = ei.code_buffer.preallocate<ImplExprConstantFloat>(line);
				ret->init(o.internal_value());
				return ret;

			},
			[&](OwcaString o) -> ImplExpr* {
				auto ret = ei.code_buffer.preallocate<ImplExprConstantString>(line);
				ret->init(o.internal_value());
				return ret;

			},
			[&](const auto&) -> ImplExpr* {
				assert(false);
				return nullptr;
			}
		);
	}
	void AstExprConstant::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprConstant::visit_children(AstVisitor& vis) {
	}
}