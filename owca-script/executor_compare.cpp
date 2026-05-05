#include "stdafx.h"
#include "executor_compare.h"
#include "ast_expr_compare.h"
#include "string.h"
#include "array.h"
#include "tuple.h"
#include "dictionary.h"
#include "vm.h"

namespace OwcaScript::Internal {
	namespace {
		enum class Result {
			False, True, NotExec
		};

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
            assert(false);
            return false;
        }
        static Result compare_impl(CompareKind kind, const auto& left, const auto& right) {
            if (kind == CompareKind::Is) return Result::NotExec;
            return compare_impl2(kind, left, right) ? Result::True : Result::False;
        }
        static Result compare_split(OwcaVM, CompareKind kind, const OwcaEmpty& l, const OwcaEmpty& r) {
            return build_result(kind, true);
        }
        template <typename T> static Result compare_array_iter(OwcaVM vm, CompareKind kind, const T &left, const T &right) {
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

        static Result compare_split(OwcaVM, CompareKind kind, const bool& l, const bool& r) { 
            return build_result(kind, l == r);
        }
        static Result compare_split(OwcaVM, CompareKind kind, const Number& l, const Number& r) { return compare_impl(kind, l, r); }
        static Result compare_split(OwcaVM, CompareKind kind, const OwcaString& l, const OwcaString& r) {
            return compare_impl(kind, l.internal_value()->text(), r.internal_value()->text());
        }
        static Result compare_split(OwcaVM, CompareKind kind, const OwcaRange& l, const OwcaRange& r) {
            return build_result(kind, l.lower() == r.lower() && l.upper() == r.upper() && l.step() == r.step());
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
    }

	CompareResult execute_compare(VM *vm, CompareKind kind, OwcaValue left, OwcaValue right)
	{
		if (kind == CompareKind::Is) {
			if (left.kind() != right.kind()) 
				return CompareResult::False;

			switch (left.kind()) {
			case OwcaValueKind::Empty: return CompareResult::True;
			case OwcaValueKind::Range: return (left.as_range(vm).lower() == right.as_range(vm).lower() && left.as_range(vm).upper() == right.as_range(vm).upper() && left.as_range(vm).step() == right.as_range(vm).step()) ? CompareResult::True : CompareResult::False;
			case OwcaValueKind::Bool: return (left.as_bool(vm) == right.as_bool(vm)) ? CompareResult::True : CompareResult::False;
			case OwcaValueKind::Float: return (left.as_float(vm) == right.as_float(vm)) ? CompareResult::True : CompareResult::False;
			case OwcaValueKind::String: return (compare(vm, CompareKind::Eq, left, right) == Result::True) ? CompareResult::True : CompareResult::False;
			case OwcaValueKind::Functions: return (left.as_functions(vm).internal_value() == right.as_functions(vm).internal_value() && left.as_functions(vm).internal_self_object() == right.as_functions(vm).internal_self_object()) ? CompareResult::True : CompareResult::False;
			case OwcaValueKind::Map: return (left.as_map(vm).internal_value() == right.as_map(vm).internal_value()) ? CompareResult::True : CompareResult::False;
			case OwcaValueKind::Class: return (left.as_class(vm).internal_value() == right.as_class(vm).internal_value()) ? CompareResult::True : CompareResult::False;
			case OwcaValueKind::Object: return (left.as_object(vm).internal_value() == right.as_object(vm).internal_value()) ? CompareResult::True : CompareResult::False;
			case OwcaValueKind::Array: return (left.as_array(vm).internal_value() == right.as_array(vm).internal_value()) ? CompareResult::True : CompareResult::False;
			case OwcaValueKind::Tuple: return (left.as_tuple(vm).internal_value() == right.as_tuple(vm).internal_value()) ? CompareResult::True : CompareResult::False;
			case OwcaValueKind::Set: return (left.as_set(vm).internal_value() == right.as_set(vm).internal_value()) ? CompareResult::True : CompareResult::False;
			case OwcaValueKind::Iterator: return (left.as_iterator(vm).internal_value() == right.as_iterator(vm).internal_value()) ? CompareResult::True : CompareResult::False;
			case OwcaValueKind::Exception: return (left.as_exception(vm).internal_owner() == right.as_exception(vm).internal_owner()) ? CompareResult::True : CompareResult::False;
            case OwcaValueKind::Namespace: return (left.as_namespace(vm).internal_value() == right.as_namespace(vm).internal_value()) ? CompareResult::True : CompareResult::False;
			case OwcaValueKind::Completed: return CompareResult::True;
			case OwcaValueKind::_Count: break;
			}
			assert(false);
			return CompareResult::False;
		}
		
        auto res = compare(vm, kind, left, right);
		// auto res = left.visit(
		// 	[&](const auto& ll) {
		// 		return right.visit(
		// 			[&](const auto& rr) {
		// 				return compare_split(vm, kind, ll, rr);
		// 			}
		// 		);
		// 	}
		// );
		switch (res) {
		case Result::True: return CompareResult::True;
		case Result::False: return CompareResult::False;
		case Result::NotExec: break;
		}
		if (kind == CompareKind::Eq) return CompareResult::False;
		if (kind == CompareKind::NotEq) return CompareResult::True;
        return CompareResult::NotExecuted;
	}
}