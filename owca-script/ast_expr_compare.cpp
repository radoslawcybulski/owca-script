#include "stdafx.h"
#include "ast_expr_compare.h"
#include "vm.h"
#include "owca_vm.h"
#include "owca_iterator.h"
#include "array.h"
#include "dictionary.h"
#include "owca_value.h"

namespace OwcaScript::Internal {
	namespace {
		enum class Result {
			False, True, NotExec
		};
	}

	static Result compare(OwcaVM vm, CompareKind kind, OwcaValue left, OwcaValue right);
	static Result build_result(CompareKind kind, bool is_eq) {
		switch (kind) {
		case CompareKind::Eq:
			return is_eq ? Result::True : Result::False;
		case CompareKind::NotEq:
			return is_eq ? Result::False : Result::True;
		case CompareKind::LessEq:
		case CompareKind::MoreEq:
		case CompareKind::Less:
		case CompareKind::More: break;
		case CompareKind::Is: assert(false);
		}
		return Result::NotExec;
	}
	static bool compare_impl2(CompareKind kind, const auto &left, const auto &right) {
		switch (kind) {
		case CompareKind::Eq: return left == right;
		case CompareKind::NotEq: return left != right;
		case CompareKind::LessEq: return left <= right;
		case CompareKind::MoreEq: return left >= right;
		case CompareKind::Less: return left < right;
		case CompareKind::More: return left > right;
		case CompareKind::Is: return left == right;
		}
		assert(0);
		return false;
	}
	static bool compare_impl2(CompareKind kind, OwcaString left, OwcaString right) {
		return false;
	}
	static Result compare_impl(CompareKind kind, const auto& left, const auto& right) {
		if (kind == CompareKind::Is) return Result::NotExec;
		return compare_impl2(kind, left, right) ? Result::True : Result::False;
	}
	static Result compare_split(OwcaVM, CompareKind kind, const OwcaEmpty& l, const OwcaEmpty& r) {
		return build_result(kind, true);
	}
	static Result compare_array_iter(OwcaVM vm, CompareKind kind, const std::vector<OwcaValue> &left, const std::vector<OwcaValue> &right) {
		if (kind == CompareKind::Eq) {
			if (left.size() != right.size()) return Result::False;
		}
		else if (kind == CompareKind::NotEq) {
			if (left.size() != right.size()) return Result::True;
		}

		for(auto i = 0u; i < std::min(left.size(), right.size()); ++i) {
			auto &l = left[i];
			auto &r = right[i];
			auto rr = compare(vm, CompareKind::Eq, l, r);
			switch(rr) {
			case Result::NotExec: return Result::NotExec;
			case Result::True:
				continue;
			case Result::False:
				switch (kind) {
				case CompareKind::Eq: return Result::False;
				case CompareKind::NotEq: return Result::True;
				case CompareKind::LessEq:
				case CompareKind::MoreEq:
				case CompareKind::Less:
				case CompareKind::More: break;
				case CompareKind::Is: assert(false);
				}
			}

			rr = compare(vm, kind, l, r);
			return rr;
		}

		switch (kind) {
		case CompareKind::Eq: return left.size() == right.size() ? Result::True : Result::False;
		case CompareKind::NotEq: return left.size() == right.size() ? Result::False : Result::True;
		case CompareKind::LessEq: return left.size() <= right.size() ? Result::True : Result::False;
		case CompareKind::MoreEq: return left.size() >= right.size() ? Result::True : Result::False;
		case CompareKind::Less: return left.size() < right.size() ? Result::True : Result::False;
		case CompareKind::More: return left.size() > right.size() ? Result::True : Result::False;
		case CompareKind::Is: assert(false);
		}
		assert(false);
		return Result::NotExec;
	}

	static Result compare_split(OwcaVM, CompareKind kind, const OwcaBool& l, const OwcaBool& r) { 
		return build_result(kind, l.internal_value() == r.internal_value());
	}
	static Result compare_split(OwcaVM, CompareKind kind, const OwcaInt& l, const OwcaInt& r) { return compare_impl(kind, l.internal_value(), r.internal_value()); }
	static Result compare_split(OwcaVM, CompareKind kind, const OwcaInt& l, const OwcaFloat& r) { return compare_impl(kind, l.internal_value(), r.internal_value()); }
	static Result compare_split(OwcaVM, CompareKind kind, const OwcaFloat& l, const OwcaInt& r) { return compare_impl(kind, l.internal_value(), r.internal_value()); }
	static Result compare_split(OwcaVM, CompareKind kind, const OwcaFloat& l, const OwcaFloat& r) { return compare_impl(kind, l.internal_value(), r.internal_value()); }
	static Result compare_split(OwcaVM, CompareKind kind, const OwcaString& l, const OwcaString& r) { return compare_impl(kind, l.internal_value(), r.internal_value()); }
	static Result compare_split(OwcaVM, CompareKind kind, const OwcaRange& l, const OwcaRange& r) {
		return build_result(kind, l.lower().internal_value() == r.lower().internal_value() && l.upper().internal_value());
	}
	static Result compare_split(OwcaVM, CompareKind kind, const OwcaFunctions& l, const OwcaFunctions& r) {
		return build_result(kind, l.internal_value() == r.internal_value() && l.internal_self_object() == r.internal_self_object());
	}
	static Result compare_split(OwcaVM, CompareKind kind, const OwcaClass& l, const OwcaClass& r) {
		return build_result(kind, l.internal_value() == r.internal_value());
	}
	static Result compare_split(OwcaVM, CompareKind kind, const OwcaObject& l, const OwcaObject& r) {
		return build_result(kind, l.internal_value() == r.internal_value());
	}
	static Result compare_split(OwcaVM vm, CompareKind kind, const OwcaArray& l, const OwcaArray& r) {
		return compare_array_iter(vm, kind, l.internal_value()->values, r.internal_value()->values);
	}
	static Result compare_split(OwcaVM vm, CompareKind kind, const OwcaTuple& l, const OwcaTuple& r) {
		return compare_array_iter(vm, kind, l.internal_value()->values, r.internal_value()->values);
	}
	static Result compare_split(OwcaVM vm, CompareKind kind, const OwcaMap& l, const OwcaMap& r) {
		switch (kind) {
		case CompareKind::Eq:
			if (l.size() != r.size()) return Result::False;
			break;
		case CompareKind::NotEq:
			if (l.size() != r.size()) return Result::True;
			break;
		case CompareKind::LessEq:
		case CompareKind::MoreEq:
		case CompareKind::Less:
		case CompareKind::More: return Result::NotExec;
		case CompareKind::Is: assert(false);
		}
		for(auto it : l) {
			auto rv = r.value(it.first);
			if (!rv) return kind == CompareKind::Eq ? Result::False : Result::True;
			auto rr = compare(vm, CompareKind::Eq, it.second, *rv);
			switch(rr) {
			case Result::NotExec: return rr;
			case Result::False:
				return kind == CompareKind::Eq ? Result::False : Result::True;
			case Result::True:
				break;
			}
		}
		return kind == CompareKind::Eq ? Result::True : Result::False;
	}
	static Result compare_split(OwcaVM vm, CompareKind kind, const OwcaSet& l, const OwcaSet& r) {
		switch (kind) {
		case CompareKind::Eq:
			if (l.size() != r.size()) return Result::False;
			break;
		case CompareKind::NotEq:
			if (l.size() != r.size()) return Result::True;
			break;
		case CompareKind::LessEq:
		case CompareKind::MoreEq:
		case CompareKind::Less:
		case CompareKind::More: return Result::NotExec;
		case CompareKind::Is: assert(false);
		}
		for(auto it : l) {
			auto rv = r.has_value(it);
			if (!rv) return kind == CompareKind::Eq ? Result::False : Result::True;
		}
		return kind == CompareKind::Eq ? Result::True : Result::False;
	}
	static Result compare_split(OwcaVM vm, CompareKind kind, const auto& l, const auto& r) {
		return Result::NotExec;
	}
	static Result compare(OwcaVM vm, CompareKind kind, OwcaValue left, OwcaValue right) {
		return left.visit(
				[&](const auto& ll) {
					return right.visit(
						[&](const auto& rr) {
							return compare_split(vm, kind, ll, rr);
						}
					);
				}
			);
	}

	class ImplExprCompare : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		ImplExpr* first;
		std::span<std::tuple<CompareKind, Line, ImplExpr*>> nexts;

		void init(ImplExpr* first, std::span<std::tuple<CompareKind, Line, ImplExpr*>> nexts) {
			this->first = first;
			this->nexts = nexts;
		}
		
		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto left = first->execute_expression(vm);

			for (auto i = 0u; i < nexts.size(); ++i) {
				auto [kind, compare_line, next_value] = nexts[i];
				auto right = next_value->execute_expression(vm);
				VM::get(vm).update_execution_line(compare_line);

				if (!AstExprCompare::execute_compare(vm, kind, left, right)) return OwcaBool{ false };
				left = right;
			}
			return OwcaBool{ true };
		}
	};

	bool AstExprCompare::execute_compare(OwcaVM vm, CompareKind kind, OwcaValue left, OwcaValue right)
	{
		if (kind == CompareKind::Is) {
			if (left.kind() != right.kind()) 
				return false;

			switch (left.kind()) {
			case OwcaValueKind::Empty: return true;
			case OwcaValueKind::Range: return left.as_range(vm).internal_values() == right.as_range(vm).internal_values();
			case OwcaValueKind::Bool: return left.as_bool(vm).internal_value() == right.as_bool(vm).internal_value();
			case OwcaValueKind::Int: return left.as_int(vm).internal_value() == right.as_int(vm).internal_value();
			case OwcaValueKind::Float: return left.as_float(vm).internal_value() == right.as_float(vm).internal_value();
			case OwcaValueKind::String: return compare(vm, CompareKind::Eq, left, right) == Result::True;
			case OwcaValueKind::Functions: return left.as_functions(vm).internal_value() == right.as_functions(vm).internal_value() && left.as_functions(vm).internal_self_object() == right.as_functions(vm).internal_self_object();
			case OwcaValueKind::Map: return left.as_map(vm).internal_value() == right.as_map(vm).internal_value();
			case OwcaValueKind::Class: return left.as_class(vm).internal_value() == right.as_class(vm).internal_value();
			case OwcaValueKind::Object: return left.as_object(vm).internal_value() == right.as_object(vm).internal_value();
			case OwcaValueKind::Array: return left.as_array(vm).internal_value() == right.as_array(vm).internal_value();
			case OwcaValueKind::Tuple: return left.as_tuple(vm).internal_value() == right.as_tuple(vm).internal_value();
			}
			assert(false);
			return false;
		}
		
		auto res = left.visit(
			[&](const auto& ll) {
				return right.visit(
					[&](const auto& rr) {
						return compare_split(vm, kind, ll, rr);
					}
				);
			}
		);
		switch (res) {
		case Result::True: return true;
		case Result::False: return false;
		case Result::NotExec: break;
		}
		if (kind == CompareKind::Eq) return false;
		if (kind == CompareKind::NotEq) return true;
		VM::get(vm).throw_cant_compare(kind, left.type(), right.type());
	}

	void AstExprCompare::calculate_size(CodeBufferSizeCalculator &ei) const {
		ei.code_buffer.preallocate<ImplExprCompare>(line);
		first->calculate_size(ei);
		ei.code_buffer.preallocate_array<std::tuple<CompareKind, Line, ImplExpr*>>(nexts.size());
		for (auto i = 0u; i < nexts.size(); ++i) {
			std::get<2>(nexts[i])->calculate_size(ei);
		}
	}
	ImplExpr* AstExprCompare::emit(EmitInfo& ei) {
		auto ret = ei.code_buffer.preallocate<ImplExprCompare>(line);
		auto f = first->emit(ei);
		auto arr = ei.code_buffer.preallocate_array<std::tuple<CompareKind, Line, ImplExpr*>>(nexts.size());
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