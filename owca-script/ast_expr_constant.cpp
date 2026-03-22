#include "stdafx.h"
#include "ast_expr_constant.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	class ImplExprConstantEmpty : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		#define FIELDS(Q) 

		IMPL_DEFINE_EXPR(Kind::ConstantEmpty)

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			return OwcaEmpty{};
		}
	};
	class ImplExprConstantBool : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		#undef FIELDS		
		#define FIELDS(Q) Q(value, bool)

		IMPL_DEFINE_EXPR(Kind::ConstantBool)

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			return OwcaBool{ value };
		}
	};
	class ImplExprConstantFloat : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		#undef FIELDS
		#define FIELDS(Q) Q(value, Number)

		IMPL_DEFINE_EXPR(Kind::ConstantFloat)

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			return value;
		}
	};
	class ImplExprConstantString : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		#undef FIELDS
		#define FIELDS(Q) Q(value, std::string_view)

		IMPL_DEFINE_EXPR(Kind::ConstantString)

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
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
			[&](Number o) {
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
			[&](Number o) -> ImplExpr* {
				auto ret = ei.code_buffer.preallocate<ImplExprConstantFloat>(line);
				ret->init(o);
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
	void AstExprConstant::initialize_serialization_functions(std::span<std::function<ImplExpr*(Deserializer&, Line)>> functions)
	{
		functions[(size_t)ImplExpr::Kind::ConstantEmpty] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplExprConstantEmpty>(line); };
		functions[(size_t)ImplExpr::Kind::ConstantBool] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplExprConstantBool>(line); };
		functions[(size_t)ImplExpr::Kind::ConstantFloat] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplExprConstantFloat>(line); };
		functions[(size_t)ImplExpr::Kind::ConstantString] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplExprConstantString>(line); };
	}
}