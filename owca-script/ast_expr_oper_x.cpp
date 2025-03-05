#include "stdafx.h"
#include "ast_expr_oper_x.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	class ImplExprOperXBase : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		#define FIELDS(Q) \
			Q(args, std::span<ImplExpr*>)

		IMPL_DEFINE_EXPR(Kind::Call)
	};
	class ImplExprCall : public ImplExprOperXBase {
	public:
		using ImplExprOperXBase::ImplExprOperXBase;

		Kind kind() const override { return Kind::Call; }
		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto f = args[0]->execute_expression(vm);
			std::vector<OwcaValue> arguments;
			arguments.reserve(args.size() - 1);
			for(auto i = 1u; i < args.size(); ++i) {
				arguments.push_back(args[i]->execute_expression(vm));
			}
			return VM::get(vm).execute_call(std::move(f), arguments);
		}
	};
	class ImplExprCreateArray : public ImplExprOperXBase {
	public:
		using ImplExprOperXBase::ImplExprOperXBase;

		Kind kind() const override { return Kind::CreateArray; }
		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			std::deque<OwcaValue> arguments;
			for(auto &a : args) {
				arguments.push_back(a->execute_expression(vm));
			}
			return VM::get(vm).create_array(std::move(arguments));
		}
	};
	class ImplExprCreateTuple : public ImplExprOperXBase {
	public:
		using ImplExprOperXBase::ImplExprOperXBase;

		Kind kind() const override { return Kind::CreateTuple; }
		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			std::vector<OwcaValue> arguments;
			arguments.reserve(args.size());
			for(auto &a : args) {
				arguments.push_back(a->execute_expression(vm));
			}
			return VM::get(vm).create_tuple(std::move(arguments));
		}
	};
	class ImplExprCreateSet : public ImplExprOperXBase {
	public:
		using ImplExprOperXBase::ImplExprOperXBase;

		Kind kind() const override { return Kind::CreateSet; }
		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			std::vector<OwcaValue> arguments;
			arguments.reserve(args.size());
			for(auto &a : args) {
				arguments.push_back(a->execute_expression(vm));
			}
			return VM::get(vm).create_set(std::move(arguments));
		}
	};
	class ImplExprCreateMap : public ImplExprOperXBase {
	public:
		using ImplExprOperXBase::ImplExprOperXBase;

		Kind kind() const override { return Kind::CreateMap; }
		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			std::vector<OwcaValue> arguments;
			arguments.reserve(args.size());
			for(auto &a : args) {
				arguments.push_back(a->execute_expression(vm));
			}
			return VM::get(vm).create_map(std::move(arguments));
		}
	};

	template <typename T> static T* make(AstBase::EmitInfo& ei, Line line, const std::vector<std::unique_ptr<AstExpr>>& args) {
		auto ret = ei.code_buffer.preallocate<T>(line);
		auto tmp = ei.code_buffer.preallocate_array<ImplExpr*>(args.size());
		for (auto i = 0u; i < args.size(); ++i) {
			tmp[i] = args[i]->emit(ei);
		}
		ret->init(tmp);
		return ret;
	}
	void AstExprOperX::calculate_size(CodeBufferSizeCalculator& ei) const
	{
		switch (kind) {
		case Kind::Call: ei.code_buffer.preallocate<ImplExprCall>(line); break;
		case Kind::CreateArray: ei.code_buffer.preallocate<ImplExprCreateArray>(line); break;
		case Kind::CreateTuple: ei.code_buffer.preallocate<ImplExprCreateTuple>(line); break;
		case Kind::CreateSet: ei.code_buffer.preallocate<ImplExprCreateSet>(line); break;
		case Kind::CreateMap: ei.code_buffer.preallocate<ImplExprCreateMap>(line); break;
		}
		ei.code_buffer.preallocate_array<ImplExpr*>(args.size());
		for (auto i = 0u; i < args.size(); ++i) {
			args[i]->calculate_size(ei);
		}
	}
	ImplExpr* AstExprOperX::emit(EmitInfo& ei) {
		switch (kind) {
		case Kind::Call: return make<ImplExprCall>(ei, line, args);
		case Kind::CreateArray: return make<ImplExprCreateArray>(ei, line, args);
		case Kind::CreateTuple: return make<ImplExprCreateTuple>(ei, line, args);
		case Kind::CreateSet: return make<ImplExprCreateSet>(ei, line, args);
		case Kind::CreateMap: return make<ImplExprCreateMap>(ei, line, args);
		}
		assert(0);
		return nullptr;
	}

	void AstExprOperX::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprOperX::visit_children(AstVisitor& vis) {
		for(auto &a : args) a->visit(vis);
	}
	void AstExprOperX::initialize_serialization_functions(std::span<std::function<ImplExpr*(Deserializer&, Line)>> functions)
	{
		functions[(size_t)ImplExpr::Kind::Call] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplExprCall>(line); };
		functions[(size_t)ImplExpr::Kind::CreateArray] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplExprCreateArray>(line); };
		functions[(size_t)ImplExpr::Kind::CreateTuple] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplExprCreateTuple>(line); };
		functions[(size_t)ImplExpr::Kind::CreateSet] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplExprCreateSet>(line); };
		functions[(size_t)ImplExpr::Kind::CreateMap] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplExprCreateMap>(line); };
	}
}