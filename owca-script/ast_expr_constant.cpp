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
		OwcaValue execute_impl(OwcaVM vm) const override {
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
		OwcaValue execute_impl(OwcaVM vm) const override {
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
		OwcaValue execute_impl(OwcaVM vm) const override {
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
		OwcaValue execute_impl(OwcaVM vm) const override {
			return OwcaFloat{ value };
		}
	};
	class ImplExprConstantString : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;
		std::string_view value;

		void init(std::string_view value) {
			this->value = std::move(value);
		}
		OwcaValue execute_impl(OwcaVM vm) const override {
			return vm.create_string_from_view(value);
		}
	};

	void AstExprConstant::calculate_size(CodeBufferSizeCalculator &ei) const {
		return visit(
			[&](OwcaEmpty) {
				ei.code_buffer.preallocate<ImplExprConstantEmpty>(line);
			},
			[&](OwcaBool o) {
				ei.code_buffer.preallocate<ImplExprConstantBool>(line);
			},
			[&](OwcaInt o) {
				ei.code_buffer.preallocate<ImplExprConstantInt>(line);
			},
			[&](OwcaFloat o) {
				ei.code_buffer.preallocate<ImplExprConstantFloat>(line);
			},
			[&](const std::string &o) {
				ei.code_buffer.preallocate<ImplExprConstantString>(line);
				ei.code_buffer.allocate(o);
			}
		);	}
	ImplExpr* AstExprConstant::emit(EmitInfo& ei) {
		return visit(
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
			[&](const std::string &o) -> ImplExpr* {
				auto ret = ei.code_buffer.preallocate<ImplExprConstantString>(line);
				auto txt = ei.code_buffer.allocate(o);
				ret->init(txt);
				return ret;
			}
		);
	}
	void AstExprConstant::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprConstant::visit_children(AstVisitor& vis) {
	}
}