#include "stdafx.h"
#include "ast_expr_compare.h"
#include "vm.h"
#include "owca_vm.h"
#include "owca_iterator.h"
#include "array.h"
#include "dictionary.h"
#include "owca_value.h"

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
		
		static Result build_result(AstExprCompare::Kind kind, bool is_eq) {
			switch (kind) {
			case AstExprCompare::Kind::Eq:
				return is_eq ? Result::True : Result::False;
			case AstExprCompare::Kind::NotEq:
				return is_eq ? Result::False : Result::True;
			case AstExprCompare::Kind::LessEq:
			case AstExprCompare::Kind::MoreEq:
			case AstExprCompare::Kind::Less:
			case AstExprCompare::Kind::More: break;
			case AstExprCompare::Kind::Is: assert(false);
			}
			return Result::NotExec;
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
			return build_result(kind, true);
		}
		static Result compare_array_iter(OwcaVM &vm, AstExprCompare::Kind kind, const std::vector<OwcaValue> &left, const std::vector<OwcaValue> &right) {
			if (kind == AstExprCompare::Kind::Eq) {
				if (left.size() != right.size()) return Result::False;
			}
			else if (kind == AstExprCompare::Kind::NotEq) {
				if (left.size() != right.size()) return Result::True;
			}

			for(auto i = 0u; i < std::min(left.size(), right.size()); ++i) {
				auto &l = left[i];
				auto &r = right[i];
				auto rr = compare(vm, AstExprCompare::Kind::Eq, l, r);
				switch(rr) {
				case Result::NotExec: return Result::NotExec;
				case Result::True:
					continue;
				case Result::False:
					switch (kind) {
					case AstExprCompare::Kind::Eq: return Result::False;
					case AstExprCompare::Kind::NotEq: return Result::True;
					case AstExprCompare::Kind::LessEq:
					case AstExprCompare::Kind::MoreEq:
					case AstExprCompare::Kind::Less:
					case AstExprCompare::Kind::More: break;
					case AstExprCompare::Kind::Is: assert(false);
					}
				}

				rr = compare(vm, kind, l, r);
				return rr;
			}

			switch (kind) {
			case AstExprCompare::Kind::Eq: return left.size() == right.size() ? Result::True : Result::False;
			case AstExprCompare::Kind::NotEq: return left.size() == right.size() ? Result::False : Result::True;
			case AstExprCompare::Kind::LessEq: return left.size() <= right.size() ? Result::True : Result::False;
			case AstExprCompare::Kind::MoreEq: return left.size() >= right.size() ? Result::True : Result::False;
			case AstExprCompare::Kind::Less: return left.size() < right.size() ? Result::True : Result::False;
			case AstExprCompare::Kind::More: return left.size() > right.size() ? Result::True : Result::False;
			case AstExprCompare::Kind::Is: assert(false);
			}
			assert(false);
			return Result::NotExec;
		}

		static Result compare_split(OwcaVM&, AstExprCompare::Kind kind, const OwcaBool& l, const OwcaBool& r) { 
			return build_result(kind, l.internal_value() == r.internal_value());
		}
		static Result compare_split(OwcaVM&, AstExprCompare::Kind kind, const OwcaInt& l, const OwcaInt& r) { return compare_impl(kind, l.internal_value(), r.internal_value()); }
		static Result compare_split(OwcaVM&, AstExprCompare::Kind kind, const OwcaInt& l, const OwcaFloat& r) { return compare_impl(kind, l.internal_value(), r.internal_value()); }
		static Result compare_split(OwcaVM&, AstExprCompare::Kind kind, const OwcaFloat& l, const OwcaInt& r) { return compare_impl(kind, l.internal_value(), r.internal_value()); }
		static Result compare_split(OwcaVM&, AstExprCompare::Kind kind, const OwcaFloat& l, const OwcaFloat& r) { return compare_impl(kind, l.internal_value(), r.internal_value()); }
		static Result compare_split(OwcaVM&, AstExprCompare::Kind kind, const OwcaString& l, const OwcaString& r) { return compare_impl(kind, l.internal_value(), r.internal_value()); }
		static Result compare_split(OwcaVM&, AstExprCompare::Kind kind, const OwcaRange& l, const OwcaRange& r) {
			return build_result(kind, l.lower().internal_value() == r.lower().internal_value() && l.upper().internal_value());
		}
		static Result compare_split(OwcaVM&, AstExprCompare::Kind kind, const OwcaFunctions& l, const OwcaFunctions& r) {
			return build_result(kind, l.functions == r.functions && l.self_object == r.self_object);
		}
		static Result compare_split(OwcaVM&, AstExprCompare::Kind kind, const OwcaClass& l, const OwcaClass& r) {
			return build_result(kind, l.object == r.object);
		}
		static Result compare_split(OwcaVM&, AstExprCompare::Kind kind, const OwcaObject& l, const OwcaObject& r) {
			return build_result(kind, l.object == r.object);
		}
		static Result compare_split(OwcaVM &vm, AstExprCompare::Kind kind, const OwcaArray& l, const OwcaArray& r) {
			return compare_array_iter(vm, kind, l.object->values, r.object->values);
		}
		static Result compare_split(OwcaVM &vm, AstExprCompare::Kind kind, const OwcaTuple& l, const OwcaTuple& r) {
			return compare_array_iter(vm, kind, l.object->values, r.object->values);
		}
		static Result compare_split(OwcaVM &vm, AstExprCompare::Kind kind, const OwcaMap& l, const OwcaMap& r) {
			switch (kind) {
			case AstExprCompare::Kind::Eq:
				if (l.size() != r.size()) return Result::False;
				break;
			case AstExprCompare::Kind::NotEq:
				if (l.size() != r.size()) return Result::True;
				break;
			case AstExprCompare::Kind::LessEq:
			case AstExprCompare::Kind::MoreEq:
			case AstExprCompare::Kind::Less:
			case AstExprCompare::Kind::More: return Result::NotExec;
			case AstExprCompare::Kind::Is: assert(false);
			}
			for(auto it : l) {
				auto rv = r.value(it.first);
				if (!rv) return kind == AstExprCompare::Kind::Eq ? Result::False : Result::True;
				auto rr = compare(vm, AstExprCompare::Kind::Eq, it.second, *rv);
				switch(rr) {
				case Result::NotExec: return rr;
				case Result::False:
					return kind == AstExprCompare::Kind::Eq ? Result::False : Result::True;
				case Result::True:
					break;
				}
			}
			return kind == AstExprCompare::Kind::Eq ? Result::True : Result::False;
		}
		static Result compare_split(OwcaVM &vm, AstExprCompare::Kind kind, const OwcaSet& l, const OwcaSet& r) {
			switch (kind) {
			case AstExprCompare::Kind::Eq:
				if (l.size() != r.size()) return Result::False;
				break;
			case AstExprCompare::Kind::NotEq:
				if (l.size() != r.size()) return Result::True;
				break;
			case AstExprCompare::Kind::LessEq:
			case AstExprCompare::Kind::MoreEq:
			case AstExprCompare::Kind::Less:
			case AstExprCompare::Kind::More: return Result::NotExec;
			case AstExprCompare::Kind::Is: assert(false);
			}
			for(auto it : l) {
				auto rv = r.has_value(it);
				if (!rv) return kind == AstExprCompare::Kind::Eq ? Result::False : Result::True;
			}
			return kind == AstExprCompare::Kind::Eq ? Result::True : Result::False;
		}
		static Result compare_split(OwcaVM& vm, AstExprCompare::Kind kind, const auto& l, const auto& r) {
			return Result::NotExec;
		}
		static Result compare(OwcaVM &vm, AstExprCompare::Kind kind, const OwcaValue& left, const OwcaValue& right) {
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
		OwcaValue execute_impl(OwcaVM &vm) const override {
			auto left = first->execute(vm);
			auto [li, lf] = left.get_int_or_float();

			for (auto i = 0u; i < nexts.size(); ++i) {
				auto [kind, compare_line, next_value] = nexts[i];
				auto right = next_value->execute(vm);
				VM::get(vm).update_execution_line(compare_line);

				if (kind == AstExprCompare::Kind::Is) {
					if (left.kind() != right.kind()) 
						return OwcaBool{ false };

					switch (left.kind()) {
					case OwcaValueKind::Empty: continue;
					case OwcaValueKind::Range:
						if (left.as_range(vm).internal_values() == right.as_range(vm).internal_values()) continue;
						break;
					case OwcaValueKind::Bool:
						if (left.as_bool(vm).internal_value() == right.as_bool(vm).internal_value()) continue;
						break;
					case OwcaValueKind::Int:
						if (left.as_int(vm).internal_value() == right.as_int(vm).internal_value()) continue;
						break;
					case OwcaValueKind::Float:
						if (left.as_float(vm).internal_value() == right.as_float(vm).internal_value()) continue;
						break;
					case OwcaValueKind::String:
						if (left.as_string(vm).internal_value() == right.as_string(vm).internal_value()) continue;
						break;
					case OwcaValueKind::Functions:
						if (left.as_functions(vm).functions == right.as_functions(vm).functions) continue;
						break;
					case OwcaValueKind::Map:
						if (left.as_map(vm).dictionary == right.as_map(vm).dictionary) continue;
						break;
					case OwcaValueKind::Class:
						if (left.as_class(vm).object == right.as_class(vm).object) continue;
						break;
					case OwcaValueKind::Object:
						if (left.as_object(vm).object == right.as_object(vm).object) continue;
						break;
					case OwcaValueKind::Array:
						if (left.as_array(vm).object == right.as_array(vm).object) continue;
						break;
					case OwcaValueKind::Tuple:
						if (left.as_tuple(vm).object == right.as_tuple(vm).object) continue;
						break;
					}
					return OwcaBool{ false };
				}

				auto res = compare(vm, kind, left, right);
				switch(res) {
				case Result::False: return OwcaBool{ false };
				case Result::True: continue;
				case Result::NotExec:
					if (kind == AstExprCompare::Kind::Eq)
						return OwcaBool{ false };
					if (kind == AstExprCompare::Kind::NotEq)
						return OwcaBool{ true };
					break;
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