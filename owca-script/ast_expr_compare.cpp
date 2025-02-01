#include "stdafx.h"
#include "ast_expr_compare.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	class ImplExprCompare : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		enum class Result {
			False, True, NotExec
		};

		ImplExpr* first;
		std::span<std::tuple<AstExprCompare::Kind, Line, ImplExpr*>> nexts;

		void init(ImplExpr* first, std::span<std::tuple<AstExprCompare::Kind, Line, ImplExpr*>> nexts) {
			this->first = first;
			this->nexts = nexts;
		}

		static bool compare_impl2(AstExprCompare::Kind kind, const auto &left, const auto &right) {
			switch (kind) {
			case AstExprCompare::Kind::Eq: return left == right;
			case AstExprCompare::Kind::NotEq: return left != right;
			case AstExprCompare::Kind::LessEq: return left <= right;
			case AstExprCompare::Kind::MoreEq: return left >= right;
			case AstExprCompare::Kind::Less: return left < right;
			case AstExprCompare::Kind::More: return left > right;
			case AstExprCompare::Kind::Is: return left == right;
			}
			assert(0);
			return false;
		}
		static Result compare_impl(AstExprCompare::Kind kind, const auto& left, const auto& right) {
			if (kind == AstExprCompare::Kind::Is) return Result::NotExec;
			return compare_impl2(kind, left, right) ? Result::True : Result::False;
		}
		static Result compare_split(OwcaVM&, AstExprCompare::Kind kind, const OwcaEmpty& l, const OwcaEmpty& r) {
			switch (kind) {
			case AstExprCompare::Kind::Eq: return Result::True;
			case AstExprCompare::Kind::NotEq: return Result::False;
			case AstExprCompare::Kind::LessEq:
			case AstExprCompare::Kind::MoreEq:
			case AstExprCompare::Kind::Less:
			case AstExprCompare::Kind::More: break;
			case AstExprCompare::Kind::Is: return Result::True;
			}
			return Result::NotExec;
		}
		static Result compare_split(OwcaVM&, AstExprCompare::Kind kind, const OwcaInt& l, const OwcaInt& r) { return compare_impl(kind, l.internal_value(), r.internal_value()); }
		static Result compare_split(OwcaVM&, AstExprCompare::Kind kind, const OwcaInt& l, const OwcaFloat& r) { return compare_impl(kind, l.internal_value(), r.internal_value()); }
		static Result compare_split(OwcaVM&, AstExprCompare::Kind kind, const OwcaFloat& l, const OwcaInt& r) { return compare_impl(kind, l.internal_value(), r.internal_value()); }
		static Result compare_split(OwcaVM&, AstExprCompare::Kind kind, const OwcaFloat& l, const OwcaFloat& r) { return compare_impl(kind, l.internal_value(), r.internal_value()); }
		static Result compare_split(OwcaVM&, AstExprCompare::Kind kind, const OwcaString& l, const OwcaString& r) { return compare_impl(kind, l.internal_value(), r.internal_value()); }
		static Result compare_split(OwcaVM& vm, AstExprCompare::Kind kind, const auto& l, const auto& r) {
			return Result::NotExec;
		}
		OwcaValue execute_impl(OwcaVM &vm) const override {
			auto left = first->execute(vm);
			auto [li, lf] = left.get_int_or_float();

			for (auto i = 0u; i < nexts.size(); ++i) {
				auto [kind, compare_line, next_value] = nexts[i];
				auto right = next_value->execute(vm);
				VM::get(vm).update_execution_line(compare_line);

				auto res = left.visit(
					[&](const auto& ll) {
						return right.visit(
							[&](const auto& rr) {
								return compare_split(vm, kind, ll, rr);
							}
						);
					}
				);
				if (res == Result::False) 
					return OwcaBool{ false };
				if (res == Result::True) continue;
				if (kind == AstExprCompare::Kind::Is) {
					if (left.kind() != right.kind()) 
						return OwcaBool{ false };

					switch (left.kind()) {
					case OwcaValueKind::Empty: continue;
					case OwcaValueKind::Bool:
						if (left.as_bool(vm).internal_value() == right.as_bool(vm).internal_value()) continue;
					case OwcaValueKind::Int:
						if (left.as_int(vm).internal_value() == right.as_int(vm).internal_value()) continue;
					case OwcaValueKind::String:
						if (left.as_string(vm).internal_value() == right.as_string(vm).internal_value()) continue;
					case OwcaValueKind::Float:
						if (left.as_float(vm).internal_value() == right.as_float(vm).internal_value()) continue;
					}
					return OwcaBool{ false };
				}

				VM::get(vm).throw_cant_compare(kind, left.type(), right.type());
			}
			return OwcaBool{ true };
		}
	};

	bool AstExprCompare::compare_equal(OwcaVM& vm, const OwcaValue& left, const OwcaValue& right)
	{
		auto res = left.visit(
			[&](const auto& ll) {
				return right.visit(
					[&](const auto& rr) {
						return ImplExprCompare::compare_split(vm, AstExprCompare::Kind::Eq, ll, rr);
					}
				);
			}
		);
		switch (res) {
		case ImplExprCompare::Result::True: return true;
		case ImplExprCompare::Result::False: return false;
		case ImplExprCompare::Result::NotExec: break;
		}
		VM::get(vm).throw_cant_compare(AstExprCompare::Kind::Eq, left.type(), right.type());
	}

	void AstExprCompare::calculate_size(CodeBufferSizeCalculator &ei) const {
		ei.code_buffer.preallocate<ImplExprCompare>(line);
		first->calculate_size(ei);
		ei.code_buffer.preallocate_array<std::tuple<AstExprCompare::Kind, Line, ImplExpr*>>(nexts.size());
		for (auto i = 0u; i < nexts.size(); ++i) {
			std::get<2>(nexts[i])->calculate_size(ei);
		}
	}
	ImplExpr* AstExprCompare::emit(EmitInfo& ei) {
		auto ret = ei.code_buffer.preallocate<ImplExprCompare>(line);
		auto f = first->emit(ei);
		auto arr = ei.code_buffer.preallocate_array<std::tuple<AstExprCompare::Kind, Line, ImplExpr*>>(nexts.size());
		for (auto i = 0u; i < nexts.size(); ++i) {
			std::get<0>(arr[i]) = std::get<0>(nexts[i]);
			std::get<1>(arr[i]) = std::get<1>(nexts[i]);
			std::get<2>(arr[i]) = std::get<2>(nexts[i])->emit(ei);
		}
		ret->init(f, arr);
		return ret;
	}
	void AstExprCompare::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprCompare::visit_children(AstVisitor& vis) {
		first->visit(vis);
		for (auto& q : nexts) {
			std::get<2>(q)->visit(vis);
		}
	}
}