#include "stdafx.h"
#include "ast_expr_oper_x.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	class ImplExprOperXBase : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		std::span<ImplExpr*> args;

		void init(std::span<ImplExpr*> args) {
			this->args = args;
		}
	};
	class ImplExprCall : public ImplExprOperXBase {
	public:
		using ImplExprOperXBase::ImplExprOperXBase;

		OwcaValue execute(OwcaVM &vm) const override {
			auto f = args[0]->execute(vm);
			std::vector<OwcaValue> arguments;
			arguments.reserve(args.size() - 1);
			for(auto i = 1u; i < args.size(); ++i) {
				arguments.push_back(args[i]->execute(vm));
			}
			return vm.vm->execute_call(std::move(f), std::move(arguments));
		}
	};
	class ImplExprIndex : public ImplExprOperXBase {
	public:
		using ImplExprOperXBase::ImplExprOperXBase;

		OwcaValue execute(OwcaVM &vm) const override {
			std::optional<OwcaIntInternal> a2;

			assert(args.size() == 2 || args.size() == 3);
			auto v = args[0]->execute(vm);
			auto a1 = args[1]->execute(vm).convert_to_int(vm);
			if (args.size() == 3)
				a2 = args[2]->execute(vm).convert_to_int(vm);

			auto orig_a1 = a1;
			auto orig_a2 = a2;
			return v.visit(
				[&](const OwcaString& o) -> OwcaValue {
					const auto size = (OwcaIntInternal)o.internal_value().size();
					if (size != o.internal_value().size()) {
						vm.vm->throw_index_out_of_range(std::format("string size {} is too large for OwcaIntInternal size to properly handle indexing, maximum value is {}", o.internal_value().size(), std::numeric_limits<OwcaIntInternal>::max()));
					}
					if (a1 < 0) a1 += size;
					if (a2 && *a2 < 0) *a2 += size;

					if (a2) {
						if (a1 < 0 || a1 >= size) {
							vm.vm->throw_index_out_of_range(std::format("index value {} is out of range for string of size {}", orig_a1, size));
						}
						if (*a2 < 0 || *a2 >= size) {
							vm.vm->throw_index_out_of_range(std::format("index value {} is out of range for string of size {}", *orig_a2, size));
						}
						if (*a2 < a1) return OwcaString{ "" };
						return OwcaString{ o.internal_value().substr(a1, *a2 - a1) };
					}
					else {
						if (a1 < 0 || a1 >= size) {
							vm.vm->throw_index_out_of_range(std::format("index value {} is out of range for string of size {}", orig_a1, size));
						}
						return OwcaString{ o.internal_value().substr(a1, 1) };
					}
				},
				[&](const auto&) -> OwcaValue {
					vm.vm->throw_value_not_indexable(v.type());
				}
			);
		}
	};
	class ImplExprCreateArray : public ImplExprOperXBase {
	public:
		using ImplExprOperXBase::ImplExprOperXBase;

		OwcaValue execute(OwcaVM &vm) const override {
			std::vector<OwcaValue> arguments;
			arguments.reserve(args.size());
			for(auto &a : args) {
				arguments.push_back(a->execute(vm));
			}
			return vm.vm->create_array(std::move(arguments));
		}
	};
	class ImplExprCreateSet : public ImplExprOperXBase {
	public:
		using ImplExprOperXBase::ImplExprOperXBase;

		OwcaValue execute(OwcaVM &vm) const override {
			std::vector<OwcaValue> arguments;
			arguments.reserve(args.size());
			for(auto &a : args) {
				arguments.push_back(a->execute(vm));
			}
			return vm.vm->create_set(std::move(arguments));
		}
	};
	class ImplExprCreateMap : public ImplExprOperXBase {
	public:
		using ImplExprOperXBase::ImplExprOperXBase;

		OwcaValue execute(OwcaVM &vm) const override {
			std::vector<OwcaValue> arguments;
			arguments.reserve(args.size());
			for(auto &a : args) {
				arguments.push_back(a->execute(vm));
			}
			return vm.vm->create_map(std::move(arguments));
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

	ImplExpr* AstExprOperX::emit(EmitInfo& ei) {
		switch (kind) {
		case Kind::Call: return make<ImplExprCall>(ei, line, args);
		case Kind::Index: return make<ImplExprCall>(ei, line, args);
		case Kind::CreateArray: return make<ImplExprCreateArray>(ei, line, args);
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
}