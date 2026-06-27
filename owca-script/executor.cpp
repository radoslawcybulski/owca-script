#include "owca-script/owca_namespace.h"
#include "owca-script/owca_value.h"
#include "owca_exception.h"
#include "owca_iterator.h"
#include "owca_map.h"
#include "stdafx.h"
#include "executor.h"
#include "vm.h"
#include "object.h"
#include "runtime_function.h"
#include "iterator.h"
#include "exec_buffer.h"
#include "range.h"
#include "string.h"
#include "array.h"
#include "tuple.h"
#include "dictionary.h"
#include "ast_function.h"
#include "exception.h"
#include "namespace.h"
#include <utility>

#ifdef DEBUG
#define OWCA_SCRIPT_EXEC_LOG
#endif

//#define MEASURE

namespace OwcaScript::Internal {
    enum class CompareResult : std::uint8_t;

    static OwcaValue op_add_cant(OwcaValue left, OwcaValue right) { current_vm().throw_unsupported_operation_2("+", left.type(), right.type()); }
    static OwcaValue op_sub_cant(OwcaValue left, OwcaValue right) { current_vm().throw_unsupported_operation_2("-", left.type(), right.type()); }
    static OwcaValue op_mul_cant(OwcaValue left, OwcaValue right) { current_vm().throw_unsupported_operation_2("*", left.type(), right.type()); }
    static OwcaValue op_div_cant(OwcaValue left, OwcaValue right) { current_vm().throw_unsupported_operation_2("/", left.type(), right.type()); }
    static OwcaValue op_mod_cant(OwcaValue left, OwcaValue right) { current_vm().throw_unsupported_operation_2("%", left.type(), right.type()); }
    static OwcaValue op_bin_and_cant(OwcaValue left, OwcaValue right) { current_vm().throw_unsupported_operation_2("&", left.type(), right.type()); }
    static OwcaValue op_bin_or_cant(OwcaValue left, OwcaValue right) { current_vm().throw_unsupported_operation_2("|", left.type(), right.type()); }
    static OwcaValue op_bin_xor_cant(OwcaValue left, OwcaValue right) { current_vm().throw_unsupported_operation_2("^", left.type(), right.type()); }
    static OwcaValue op_bin_lshift_cant(OwcaValue left, OwcaValue right) { current_vm().throw_unsupported_operation_2("<<", left.type(), right.type()); }
    static OwcaValue op_bin_rshift_cant(OwcaValue left, OwcaValue right) { current_vm().throw_unsupported_operation_2(">>", left.type(), right.type()); }
    static bool op_compare_eq_default(OwcaValue left, OwcaValue right) { return false; }
    static bool op_compare_lt_cant(OwcaValue left, OwcaValue right) { current_vm().throw_unsupported_operation_2("<", left.type(), right.type()); }
    static bool op_compare_is_default(OwcaValue left, OwcaValue right) { return false; }

    Operators2::Operators2() {
        add = op_add_cant;
        sub = op_sub_cant;
        mul = op_mul_cant;
        div = op_div_cant;
        mod = op_mod_cant;
        bin_and = op_bin_and_cant;
        bin_or = op_bin_or_cant;
        bin_xor = op_bin_xor_cant;
        bin_lshift = op_bin_lshift_cant;
        bin_rshift = op_bin_rshift_cant;

        eq = op_compare_eq_default;
        less = op_compare_lt_cant;
        is = op_compare_is_default;
    }

    static OwcaValue op_add_number_number(OwcaValue left, OwcaValue right) { return left.as_float_certainly() + right.as_float_certainly(); }
    static OwcaValue op_sub_number_number(OwcaValue left, OwcaValue right) { return left.as_float_certainly() - right.as_float_certainly(); }
    static OwcaValue op_mul_number_number(OwcaValue left, OwcaValue right) { return left.as_float_certainly() * right.as_float_certainly(); }
    static OwcaValue op_div_number_number(OwcaValue left, OwcaValue right) {
        if (right.as_float_certainly() == 0) {
            current_vm().throw_division_by_zero();
        }
        return left.as_float_certainly() / right.as_float_certainly();
    }
    static OwcaValue op_mod_number_number(OwcaValue left, OwcaValue right) {
        if (right.as_float_certainly() == 0) {
            current_vm().throw_division_by_zero();
        }
        return (std::int64_t)left.as_float_certainly() % (std::int64_t)right.as_float_certainly();
    }
    static OwcaValue op_bin_and_number_number(OwcaValue left, OwcaValue right) { return (std::int64_t)left.as_float_certainly() & (std::int64_t)right.as_float_certainly(); }
    static OwcaValue op_bin_or_number_number(OwcaValue left, OwcaValue right) { return (std::int64_t)left.as_float_certainly() | (std::int64_t)right.as_float_certainly(); }
    static OwcaValue op_bin_xor_number_number(OwcaValue left, OwcaValue right) { return (std::int64_t)left.as_float_certainly() ^ (std::int64_t)right.as_float_certainly(); }
    static OwcaValue op_bin_lshift_number_number(OwcaValue left, OwcaValue right) { return (std::int64_t)left.as_float_certainly() << (std::int64_t)right.as_float_certainly(); }
    static OwcaValue op_bin_rshift_number_number(OwcaValue left, OwcaValue right) { return (std::int64_t)left.as_float_certainly() >> (std::int64_t)right.as_float_certainly(); }

    static bool op_compare_eq_number_number(OwcaValue left, OwcaValue right) { return left.as_float_certainly() == right.as_float_certainly(); }
    static bool op_compare_lt_number_number(OwcaValue left, OwcaValue right) { return left.as_float_certainly() < right.as_float_certainly(); }
    static bool op_compare_is_number_number(OwcaValue left, OwcaValue right) { return left.as_float_certainly() == right.as_float_certainly(); }

    static bool op_compare_eq_string_string(OwcaValue left, OwcaValue right) { return left.as_string_certainly().text() == right.as_string_certainly().text(); }
    static bool op_compare_lt_string_string(OwcaValue left, OwcaValue right) { return left.as_string_certainly().text() < right.as_string_certainly().text(); }
    static bool op_compare_is_string_string(OwcaValue left, OwcaValue right) { return left.as_string_certainly().text() == right.as_string_certainly().text(); }

    static bool op_compare_eq_bool_bool(OwcaValue left, OwcaValue right) { return left.as_bool_certainly() == right.as_bool_certainly(); }
    static bool op_compare_ne_bool_bool(OwcaValue left, OwcaValue right) { return left.as_bool_certainly() != right.as_bool_certainly(); }
    static bool op_compare_is_bool_bool(OwcaValue left, OwcaValue right) { return left.as_bool_certainly() == right.as_bool_certainly(); }

    static bool op_compare_eq_nul_nul(OwcaValue left, OwcaValue right) { return true; }
    static bool op_compare_ne_nul_nul(OwcaValue left, OwcaValue right) { return false; }
    static bool op_compare_is_nul_nul(OwcaValue left, OwcaValue right) { return true; }

    static bool op_compare_eq_range_range(OwcaValue left, OwcaValue right) { return left.as_range_certainly() == right.as_range_certainly(); }
    static bool op_compare_ne_range_range(OwcaValue left, OwcaValue right) { return left.as_range_certainly() != right.as_range_certainly(); }
    static bool op_compare_is_range_range(OwcaValue left, OwcaValue right) { return left.as_range_certainly().is(right.as_range_certainly()); }

    static bool op_compare_eq_functions_functions(OwcaValue left, OwcaValue right) { return left.as_functions_certainly() == right.as_functions_certainly(); }
    static bool op_compare_ne_functions_functions(OwcaValue left, OwcaValue right) { return left.as_functions_certainly() != right.as_functions_certainly(); }
    static bool op_compare_is_functions_functions(OwcaValue left, OwcaValue right) { return left.as_functions_certainly().is(right.as_functions_certainly()); }

    static bool op_compare_eq_map_map(OwcaValue left, OwcaValue right) { return left.as_map_certainly() == right.as_map_certainly(); }
    static bool op_compare_ne_map_map(OwcaValue left, OwcaValue right) { return left.as_map_certainly() != right.as_map_certainly(); }
    static bool op_compare_is_map_map(OwcaValue left, OwcaValue right) { return left.as_map_certainly().is(right.as_map_certainly()); }

    static bool op_compare_eq_set_set(OwcaValue left, OwcaValue right) { return left.as_set_certainly() == right.as_set_certainly(); }
    static bool op_compare_ne_set_set(OwcaValue left, OwcaValue right) { return left.as_set_certainly() != right.as_set_certainly(); }
    static bool op_compare_is_set_set(OwcaValue left, OwcaValue right) { return left.as_set_certainly().is(right.as_set_certainly()); }

    static bool op_compare_eq_class_class(OwcaValue left, OwcaValue right) { return left.as_class_certainly() == right.as_class_certainly(); }
    static bool op_compare_ne_class_class(OwcaValue left, OwcaValue right) { return left.as_class_certainly() != right.as_class_certainly(); }
    static bool op_compare_is_class_class(OwcaValue left, OwcaValue right) { return left.as_class_certainly().is(right.as_class_certainly()); }

    static bool op_compare_eq_object_object(OwcaValue left, OwcaValue right) { return left.as_object_certainly() == right.as_object_certainly(); }
    static bool op_compare_ne_object_object(OwcaValue left, OwcaValue right) { return left.as_object_certainly() != right.as_object_certainly(); }
    static bool op_compare_is_object_object(OwcaValue left, OwcaValue right) { return left.as_object_certainly().is(right.as_object_certainly()); }

    static bool op_compare_eq_tuple_tuple(OwcaValue left, OwcaValue right) { return left.as_tuple_certainly() == right.as_tuple_certainly(); }
    static bool op_compare_lt_tuple_tuple(OwcaValue left, OwcaValue right) { return left.as_tuple_certainly() < right.as_tuple_certainly(); }
    static bool op_compare_is_tuple_tuple(OwcaValue left, OwcaValue right) { return left.as_tuple_certainly().is(right.as_tuple_certainly()); }

    static bool op_compare_eq_array_array(OwcaValue left, OwcaValue right) { return left.as_array_certainly() == right.as_array_certainly(); }
    static bool op_compare_lt_array_array(OwcaValue left, OwcaValue right) { return left.as_array_certainly() < right.as_array_certainly(); }
    static bool op_compare_is_array_array(OwcaValue left, OwcaValue right) { return left.as_array_certainly().is(right.as_array_certainly()); }

    static bool op_compare_eq_iterator_iterator(OwcaValue left, OwcaValue right) { return left.as_iterator_certainly() == right.as_iterator_certainly(); }
    static bool op_compare_ne_iterator_iterator(OwcaValue left, OwcaValue right) { return left.as_iterator_certainly() != right.as_iterator_certainly(); }
    static bool op_compare_is_iterator_iterator(OwcaValue left, OwcaValue right) { return left.as_iterator_certainly().is(right.as_iterator_certainly()); }

    static bool op_compare_eq_exception_exception(OwcaValue left, OwcaValue right) { return left.as_exception_certainly() == right.as_exception_certainly(); }
    static bool op_compare_ne_exception_exception(OwcaValue left, OwcaValue right) { return left.as_exception_certainly() != right.as_exception_certainly(); }
    static bool op_compare_is_exception_exception(OwcaValue left, OwcaValue right) { return left.as_exception_certainly().is(right.as_exception_certainly()); }

    static bool op_compare_eq_namespace_namespace(OwcaValue left, OwcaValue right) { return left.as_namespace_certainly() == right.as_namespace_certainly(); }
    static bool op_compare_ne_namespace_namespace(OwcaValue left, OwcaValue right) { return left.as_namespace_certainly() != right.as_namespace_certainly(); }
    static bool op_compare_is_namespace_namespace(OwcaValue left, OwcaValue right) { return left.as_namespace_certainly().is(right.as_namespace_certainly()); }

    static OwcaValue op_add_string_string(OwcaValue left, OwcaValue right) {
        return left.as_string_certainly() + right.as_string_certainly();
    }
    static OwcaValue op_mul_string_number(OwcaValue left, OwcaValue right) {
        return left.as_string_certainly() * right.as_float_certainly();
    }
    static OwcaValue op_mul_number_string(OwcaValue left, OwcaValue right) {
        return left.as_float_certainly() * right.as_string_certainly();
    }
    static OwcaValue op_mul_array_number(OwcaValue left, OwcaValue right) {
        return left.as_array_certainly() * right.as_float_certainly();
    }
    static OwcaValue op_mul_number_array(OwcaValue left, OwcaValue right) {
        return left.as_float_certainly() * right.as_array_certainly();
    }
    static OwcaValue op_mul_tuple_number(OwcaValue left, OwcaValue right) {
        return left.as_tuple_certainly() * right.as_float_certainly();
    }
    static OwcaValue op_mul_number_tuple(OwcaValue left, OwcaValue right) {
        return left.as_float_certainly() * right.as_tuple_certainly();
    }
    static OwcaValue op_bin_and_map_map(OwcaValue left, OwcaValue right) {
        return left.as_map_certainly() & right.as_map_certainly();
    }
    static OwcaValue op_bin_or_map_map(OwcaValue left, OwcaValue right) {
        return left.as_map_certainly() | right.as_map_certainly();
    }
    static OwcaValue op_bin_xor_map_map(OwcaValue left, OwcaValue right) {
        return left.as_map_certainly() - right.as_map_certainly();
    }
    static OwcaValue op_bin_and_set_set(OwcaValue left, OwcaValue right) {
        return left.as_set_certainly() & right.as_set_certainly();
    }
    static OwcaValue op_bin_or_set_set(OwcaValue left, OwcaValue right) {
        return left.as_set_certainly() | right.as_set_certainly();
    }
    static OwcaValue op_bin_xor_set_set(OwcaValue left, OwcaValue right) {
        return left.as_set_certainly() - right.as_set_certainly();
    }

    constexpr const size_t OwcaValuesCount = static_cast<size_t>(OwcaValueKind::_Count);
    using Oper2TypeArray = std::array<Operators2, OwcaValuesCount * OwcaValuesCount>;

    #define OPER2_GET(oper, left_kind, right_kind) oper2_functions[static_cast<size_t>(left_kind) * OwcaValuesCount + static_cast<size_t>(right_kind)].oper
    #define OPER2_SET(oper, left_kind, right_kind, func) OPER2_GET(oper, left_kind, right_kind) = func
    Oper2TypeArray oper2_functions = []() {
        constexpr size_t kind_count = static_cast<size_t>(OwcaValueKind::_Count);
        Oper2TypeArray oper2_functions;

        OPER2_SET(add, OwcaValueKind::Float, OwcaValueKind::Float, op_add_number_number);
        OPER2_SET(sub, OwcaValueKind::Float, OwcaValueKind::Float, op_sub_number_number);
        OPER2_SET(mul, OwcaValueKind::Float, OwcaValueKind::Float, op_mul_number_number);
        OPER2_SET(div, OwcaValueKind::Float, OwcaValueKind::Float, op_div_number_number);
        OPER2_SET(mod, OwcaValueKind::Float, OwcaValueKind::Float, op_mod_number_number);
        OPER2_SET(bin_and, OwcaValueKind::Float, OwcaValueKind::Float, op_bin_and_number_number);
        OPER2_SET(bin_or, OwcaValueKind::Float, OwcaValueKind::Float, op_bin_or_number_number);
        OPER2_SET(bin_xor, OwcaValueKind::Float, OwcaValueKind::Float, op_bin_xor_number_number);
        OPER2_SET(bin_lshift, OwcaValueKind::Float, OwcaValueKind::Float, op_bin_lshift_number_number);
        OPER2_SET(bin_rshift, OwcaValueKind::Float, OwcaValueKind::Float, op_bin_rshift_number_number);

        OPER2_SET(add, OwcaValueKind::String, OwcaValueKind::String, op_add_string_string);
        OPER2_SET(mul, OwcaValueKind::String, OwcaValueKind::Float, op_mul_string_number);
        OPER2_SET(mul, OwcaValueKind::Float, OwcaValueKind::String, op_mul_number_string);

        OPER2_SET(mul, OwcaValueKind::Array, OwcaValueKind::Float, op_mul_array_number);
        OPER2_SET(mul, OwcaValueKind::Float, OwcaValueKind::Array, op_mul_number_array);

        OPER2_SET(mul, OwcaValueKind::Tuple, OwcaValueKind::Float, op_mul_tuple_number);
        OPER2_SET(mul, OwcaValueKind::Float, OwcaValueKind::Tuple, op_mul_number_tuple);

        OPER2_SET(bin_and, OwcaValueKind::Map, OwcaValueKind::Map, op_bin_and_map_map);
        OPER2_SET(bin_or, OwcaValueKind::Map, OwcaValueKind::Map, op_bin_or_map_map);
        OPER2_SET(bin_xor, OwcaValueKind::Map, OwcaValueKind::Map, op_bin_xor_map_map);

        OPER2_SET(bin_and, OwcaValueKind::Set, OwcaValueKind::Set, op_bin_and_set_set);
        OPER2_SET(bin_or, OwcaValueKind::Set, OwcaValueKind::Set, op_bin_or_set_set);
        OPER2_SET(bin_xor, OwcaValueKind::Set, OwcaValueKind::Set, op_bin_xor_set_set);

        OPER2_SET(eq, OwcaValueKind::Float, OwcaValueKind::Float, op_compare_eq_number_number);
        OPER2_SET(less, OwcaValueKind::Float, OwcaValueKind::Float, op_compare_lt_number_number);

        OPER2_SET(eq, OwcaValueKind::String, OwcaValueKind::String, op_compare_eq_string_string);
        OPER2_SET(less, OwcaValueKind::String, OwcaValueKind::String, op_compare_lt_string_string);

        OPER2_SET(eq, OwcaValueKind::Bool, OwcaValueKind::Bool, op_compare_eq_bool_bool);

        OPER2_SET(eq, OwcaValueKind::Range, OwcaValueKind::Range, op_compare_eq_range_range);

        OPER2_SET(eq, OwcaValueKind::Functions, OwcaValueKind::Functions, op_compare_eq_functions_functions);

        OPER2_SET(eq, OwcaValueKind::Empty, OwcaValueKind::Empty, op_compare_eq_nul_nul);
        OPER2_SET(eq, OwcaValueKind::Empty, OwcaValueKind::Completed, op_compare_eq_nul_nul);
        OPER2_SET(eq, OwcaValueKind::Completed, OwcaValueKind::Empty, op_compare_eq_nul_nul);
        OPER2_SET(eq, OwcaValueKind::Completed, OwcaValueKind::Completed, op_compare_eq_nul_nul);

        OPER2_SET(eq, OwcaValueKind::Set, OwcaValueKind::Set, op_compare_eq_set_set);

        OPER2_SET(eq, OwcaValueKind::Map, OwcaValueKind::Map, op_compare_eq_map_map);

        OPER2_SET(eq, OwcaValueKind::Class, OwcaValueKind::Class, op_compare_eq_class_class);

        OPER2_SET(eq, OwcaValueKind::Object, OwcaValueKind::Object, op_compare_eq_object_object);

        OPER2_SET(eq, OwcaValueKind::Tuple, OwcaValueKind::Tuple, op_compare_eq_tuple_tuple);
        OPER2_SET(less, OwcaValueKind::Tuple, OwcaValueKind::Tuple, op_compare_lt_tuple_tuple);

        OPER2_SET(eq, OwcaValueKind::Array, OwcaValueKind::Array, op_compare_eq_array_array);
        OPER2_SET(less, OwcaValueKind::Array, OwcaValueKind::Array, op_compare_lt_array_array);

        OPER2_SET(eq, OwcaValueKind::Iterator, OwcaValueKind::Iterator, op_compare_eq_iterator_iterator);

        OPER2_SET(eq, OwcaValueKind::Namespace, OwcaValueKind::Namespace, op_compare_eq_namespace_namespace);

        return oper2_functions;
    }();

    Executor::Executor() : stacktrace_vector(1024), values_vector(1024 * 1024), temporary_ptr_current_top(values_vector.data()) {
        stacktrace_current = stacktrace_vector.data();
    }

#define POP_STATE() stacktrace_current->states.pop_back();
#define STATE(tp) std::get<tp>(stacktrace_current->states.back())
#define PUSH_STATE(tp) do { stacktrace_current->states.push_back(tp); } while(0)
#define TRY_STATE(tp) (std::get_if<tp>(&stacktrace_current->states.back()))
#define HAS_STATE() (!stacktrace_current->states.empty())
#define PEEK_VALUES(offset, count) std::span<OwcaValue>{ temporary_ptr[{ (offset), (count) }] }
#define PEEK_VALUE(offset) temporary_ptr[(offset)]
#define POP_VALUES(count) do { temporary_ptr = temporary_ptr - (count); } while(0)
#define PUSH_VALUE(val) do { temporary_ptr[0] = (val); ++temporary_ptr; } while(0)
#define LOCAL_VAR(index) (locals_ptr[index])

    void Executor::process_thrown_exception(CodePosition *code_pos, OwcaException exception)
    {
        while(HAS_STATE()) {
            if (auto state = TRY_STATE(TryState)) {
                *code_pos = state->catches_pos;
                exception_being_thrown = exception;
#ifdef OWCA_SCRIPT_EXEC_LOG
                std::cout << __FILE__ << ":" << __LINE__ << ": setting code position (" << (void*)code_pos << ") to " << (void*)code_pos->value() << std::endl;
#endif
                return;
            }
            POP_STATE();
        }
        throw exception;
    }
    
	std::tuple<Number, Number, Number> Executor::parse_key(OwcaValue v, OwcaValue key, Number size) {
		return key.visit(
			[&](Number o) -> std::tuple<Number, Number, Number> {
				auto v = key.as_int();
				if (v < 0) v += size;
				if (v < 0 || v >= size) {
					throw_index_out_of_range(std::format("index value {} is out of range for object of size {}", key, size));
				}
				return std::make_tuple(v, v + 1, 0);
			},
			[&](OwcaRange o) -> std::tuple<Number, Number, Number> {
				auto lower = o.lower();
				auto upper = o.upper();
				auto step = o.step();
				if (lower < 0) lower += size;
				if (upper < 0) upper += size;
				if (step > 0) {
					if (lower >= upper) return { 0, 0, 1 };
					if (upper <= 0) return { 0, 0, 1 };
					if (lower >= size) return { 0, 0, 1 };
					if (lower < 0) {
						auto skip = std::max(Number(0), std::floor(-lower / step) - 1);
						lower += skip * step;
					}
					if (step == 1 && lower < 0) {
						lower = 0;
						if (upper < 0) upper = 0;
					}
					if (upper > size) upper = size;
					return { lower, upper, step };
				}
				else {
					if (lower <= upper) return { 0, 0, 1 };
					if (upper >= size) return { 0, 0, 1 };
					if (lower < 0) return { 0, 0, 1 };
					if (lower > size) {
						auto skip = std::max(Number(0), std::floor((lower - size) / -step) - 1);
						lower -= skip * step;
					}
					if (upper < 0) upper = Number{ -1 };
					return { lower, upper, step };
				}
			},
			[&](const auto&) -> std::tuple<Number, Number, Number> {
				throw_value_not_indexable(v.type(), key.type());
			}
		);
	}
	size_t Executor::verify_key(Number v, size_t size, OwcaValue orig_key, std::string_view name) {
		if (v < 0 || v >= (Number)size) {
			throw_index_out_of_range(std::format("index value {} is out of range for {} of size {}", orig_key, name, size));
		}
		auto v2 = (size_t)v;
		if (v2 != v) {
			throw_index_out_of_range(std::format("index value {} is out of range for {} of size {} - size_t overflows", orig_key, name, size));
		}
		return v2;
	}
	std::pair<size_t, size_t> Executor::verify_key(OwcaRange k, size_t size, OwcaValue orig_key, std::string_view name) {
		auto v1 = k.lower();
		auto v2 = k.upper();
		if (v2 <= v1)
			return std::pair<size_t, size_t>{ 0, 0 };
		if (v1 < 0) v1 = 0;
		if (v2 > (Number)size) v2 = size;
		size_t v3 = (size_t)v1, v4 = (size_t)v2;
		if (v3 != v1 || v2 != v4) {
			throw_index_out_of_range(std::format("index values {} is out of range for array of size {} - size_t overflows", orig_key, size));
		}
		return std::pair<size_t, size_t>{ v3, v4 };
	}

    OwcaValue Executor::set_identifier_function(OwcaValue target, OwcaValue value) {
        assert(value.kind() == OwcaValueKind::Functions);
        auto fnc = value.as_functions_certainly();
        if (target.kind() == OwcaValueKind::Functions) {
            auto dst_fnc = target.as_functions_certainly();
            for(auto i = 0u; i < fnc.internal_value()->functions.size(); ++i) {
                if (fnc.internal_value()->functions[i]) {
                    dst_fnc.internal_value()->functions[i] = fnc.internal_value()->functions[i];
                }
            }
            return target;
        }
        return value;
    }
    
    OwcaValue Executor::index_write(OwcaValue self, OwcaValue key, OwcaValue value) {
        return self.visit(
            [&](const OwcaMap& o) -> OwcaValue {
                o.internal_value()->dict.write(key, value);
                return value;
            },
            [&](OwcaArray o) -> OwcaValue {
                const auto size = (Number)o.internal_value()->values.size();
                if (size != o.internal_value()->values.size()) {
                    throw_index_out_of_range(std::format("array size {} is too large for Number size to properly handle indexing", o.internal_value()->values.size()));
                }
                auto [ lower, upper, step ] = parse_key(key, key, size);
                if (step == 0) {
                    o[lower] = value;
                    return value;
                }
                if (step != 1) {
                    throw_range_step_must_be_one_in_left_side_of_write_assign();
                }

                auto iter = current_vm().create_iterator(value);
                auto write = lower;
                auto &values = o.internal_value()->values;
                std::vector<OwcaValue> temp;
                while(auto val = iter.next()) {
                    assert(write <= upper);
                    if (write < upper) {
                        values[write++] = *val;
                    }
                    else {
                        assert(write == upper);
                        temp.push_back(*val);
                    }
                }
                if (write < upper) {
                    assert(temp.empty());

                    for(size_t i = upper; i < values.size(); ++i) {
                        values[write++] = values[i];
                    }
                    assert(write < values.size());
                    values.resize(write);
                }
                else if (!temp.empty()) {
                    auto old_size = values.size();
                    values.resize(values.size() + temp.size());
                    auto new_size = values.size();
                    for(size_t i = old_size; i > upper; --i, --new_size) {
                        values[new_size] = values[i];
                    }
                    for(auto q : temp) {
                        values[write++] = q;
                    }
                    assert(write == new_size);
                }
                return value;
            },
            [&](OwcaTuple o) -> OwcaValue {
                throw_readonly("tuple is readonly");
            },
            [&](const auto&) -> OwcaValue {
                throw_value_not_indexable(self.type());
            }
        );
    }
    OwcaValue Executor::index_read(OwcaValue self, OwcaValue key) {
        return self.visit(
            [&](const OwcaString& o) -> OwcaValue {
                const auto size = (Number)o.internal_value()->size();
                if (size != o.internal_value()->size()) {
                    throw_index_out_of_range(std::format("string size {} is too large for Number size to properly handle indexing", o.internal_value()->size()));
                }
                auto [ lower, upper, step ] = parse_key(key, key, size);
                if (step == 0) return o.substr(lower, lower + 1);
                if (step == 1) return o.substr(lower, upper);
                std::string result;
                result.reserve((size_t)std::abs(std::ceil((upper - lower + step) / step)));
                RangeIterator iter(lower, upper, step);
                for(RangeIterator iter(lower, upper, step); !iter.done(); iter.next()) {
                    result += o.text()[iter.get()];
                }
                return current_vm().create_string_from_view(result);
            },
            [&](const OwcaArray& o) -> OwcaValue {
                const auto size = (Number)o.internal_value()->values.size();
                if (size != o.internal_value()->values.size()) {
                    throw_index_out_of_range(std::format("array size {} is too large for Number size to properly handle indexing", o.internal_value()->values.size()));
                }

                auto [ lower, upper, step ] = parse_key(key, key, size);
                if (step == 0) return o[lower];
                if (step == 1) return current_vm().create_array(o.internal_value()->sub_deque(lower, upper));
                std::deque<OwcaValue> result;
                RangeIterator iter(lower, upper, step);
                for(RangeIterator iter(lower, upper, step); !iter.done(); iter.next()) {
                    result.push_back(o[iter.get()]);
                }
                return current_vm().create_array(std::move(result));
            },
            [&](const OwcaTuple& o) -> OwcaValue {
                const auto size = (Number)o.internal_value()->values.size();
                if (size != o.internal_value()->values.size()) {
                    throw_index_out_of_range(std::format("tuple size {} is too large for Number size to properly handle indexing", o.internal_value()->values.size()));
                }

                auto [ lower, upper, step ] = parse_key(key, key, size);
                if (step == 0) return o[lower];
                if (step == 1) return current_vm().create_tuple(o.internal_value()->sub_array(lower, upper));
                std::vector<OwcaValue> result;
                result.reserve((size_t)std::abs(std::ceil((upper - lower + step) / step)));
                RangeIterator iter(lower, upper, step);
                for(RangeIterator iter(lower, upper, step); !iter.done(); iter.next()) {
                    result.push_back(o[iter.get()]);
                }
                return current_vm().create_tuple(std::move(result));
            },
            [&](const OwcaMap& o) -> OwcaValue {
                return o.internal_value()->dict.read(key);
            },
            [&](const auto&) -> OwcaValue {
                throw_value_not_indexable(self.type());
            }
        );
    }

    OwcaValue Executor::create_function(CodePosition &code_pos, GlobalsPtr globals_ptr, LocalsPtr locals_ptr)
    {
        auto &code_object = stacktrace_current->runtime_function->code;
        auto name = code_pos.decode<std::string_view>();
        auto full_name = code_pos.decode<std::string_view>();
        auto is_native = code_pos.decode<bool>();
        auto is_generator = code_pos.decode<bool>();
        auto is_method = code_pos.decode<bool>();
        auto param_count = code_pos.decode<std::uint16_t>();
        auto value_count = code_pos.decode<std::uint16_t>();
        auto temporaries_count = code_pos.decode<std::uint16_t>();
        auto state_count = code_pos.decode<std::uint16_t>();
        std::vector<std::string_view> identifier_names;
        identifier_names.resize(value_count);
        for(auto &n : identifier_names) n = code_pos.decode<std::string_view>();

        RuntimeFunction *fnc = nullptr;
        if (is_native) {
            auto &native_provider = code_object.native_code_provider();
            auto line = code_object.get_line_by_position(code_pos).line;
            if (is_generator) {
                auto f = current_vm().allocate<RuntimeFunctionNativeGenerator>(0, code_object, name, full_name, is_method, line);
                fnc = f;
                f->parameter_names = std::move(identifier_names);
                if (native_provider) {
                    if (auto impl = native_provider->native_generator(full_name, f->parameter_names)) {
                        f->generator = std::move(*impl);
                    }
                }
                if (!f->generator) {
                    throw_missing_native(std::format("missing native generator {}", full_name));
                }
            }
            else {
                auto f = current_vm().allocate<RuntimeFunctionNativeFunction>(0, code_object, name, full_name, is_method, line);
                fnc = f;
                f->parameter_names = std::move(identifier_names);
                if (native_provider) {
                    if (auto impl = native_provider->native_function(full_name, f->parameter_names)) {
                        f->function = std::move(*impl);
                    }
                }
                if (!f->function) {
                    throw_missing_native(std::format("missing native function {}", full_name));
                }
            }
        }
        else {
            std::vector<AstFunction::CopyFromParent> copy_from_parents;
            auto copy_from_parent_count = code_pos.decode<std::uint32_t>();
            copy_from_parents.reserve(copy_from_parent_count);
            for(auto i = 0u; i < copy_from_parent_count; ++i) {
                auto index_in_parent = code_pos.decode<std::uint32_t>();
                auto identifier_index = code_pos.decode<std::uint32_t>();
                copy_from_parents.push_back({ index_in_parent, identifier_index });
            }
            auto z = code_pos.decode_jump();
            auto entry_point = code_pos;
            code_pos = z;

            RuntimeFunctionScript *f;
            if (is_generator) {
                f = current_vm().allocate<RuntimeFunctionScriptGenerator>(0, code_object, globals_ptr, name, full_name, is_method, entry_point);
            }
            else {
                f = current_vm().allocate<RuntimeFunctionScriptFunction>(0, code_object, globals_ptr, name, full_name, is_method, entry_point);
            }
            fnc = f;
            f->identifier_names = std::move(identifier_names);
            f->copy_from_parents = std::move(copy_from_parents);
            f->values_from_parents.reserve(f->copy_from_parents.size());
            for(auto c : f->copy_from_parents) {
                f->values_from_parents.push_back(LOCAL_VAR(c.index_in_parent));
            }
        }
        if (is_method) {
            assert(param_count > 0);
            --param_count;
        }
        fnc->param_count = param_count;
        fnc->max_temporaries = temporaries_count;
        fnc->max_states = state_count;
        fnc->max_values = value_count;
        auto rfs = current_vm().allocate<RuntimeFunctions>(0, name, full_name);
        rfs->functions[fnc->param_count] = fnc;
        return OwcaFunctions{ rfs };
    }

    std::tuple<OwcaValue, Executor::TemporariesPtr, CodePosition> Executor::run_opcodes(GlobalsPtr globals_ptr, const LocalsPtr locals_ptr, TemporariesPtr temporary_ptr, CodePosition code_pos)
    {
        auto * const stacktrace_current_copy = stacktrace_current;
#ifdef OWCA_SCRIPT_EXEC_LOG
        auto &code_object = stacktrace_current->runtime_function->code;
        auto temporary_ptr_start = temporary_ptr;
#endif        
#ifdef MEASURE        
        std::array<std::uint64_t, (size_t)Internal::ExecuteOp::_Count> times;
        std::array<std::uint64_t, (size_t)Internal::ExecuteOp::_Count> counts;

        for(auto &t : times) t = 0;
        for(auto &c : counts) c = 0;
#endif

restart:
        try {
            for(;;) {
#ifdef MEASURE            
                auto start = std::chrono::high_resolution_clock::now();
#endif
                //std::cout << "Running opcode at position " << reader.position() << std::endl;
#ifdef OWCA_SCRIPT_EXEC_LOG
                auto line = code_object.get_line_by_position(code_pos);
#endif
                auto opcode = code_pos.decode<ExecuteOp>();
                // static std::chrono::high_resolution_clock::time_point last_time = std::chrono::high_resolution_clock::now();
                // auto now = std::chrono::high_resolution_clock::now();
                // auto df = now - last_time;
                // last_time = now;
                // std::cout << std::setw(10) << (std::chrono::duration_cast<std::chrono::nanoseconds>(df).count()) << " ns ";
#ifdef OWCA_SCRIPT_EXEC_LOG
                std::string states_debug;
                for(auto &s : stacktrace_current->states) {
                    visit_variant(s,
                        [&](const ClassState& c) { states_debug += "S"; },
                        [&](const ForState& c) { states_debug += "F"; },
                        [&](const WhileState& c) { states_debug += "W"; },
                        [&](const TryState& t) { states_debug += "T"; },
                        [&](const CatchState& t) { states_debug += "C"; },
                        [&](const WithState& t) { states_debug += "H"; }
                    );
                }
                std::cout << "Running opcode at line " << std::setw(4) << line.line << " position " << std::setw(5) << (code_pos.value() - stacktrace_current->runtime_function->code.code().data() - 1) << " temporaries " << std::setw(2) << (temporary_ptr.temporaries_ptr - temporary_ptr_start.temporaries_ptr) << 
                    " states " << std::setw(6) << states_debug <<
                    " stack " << std::setw(2) << (stacktrace_current - stacktrace_vector.data()) <<
                    " opcode " << std::setw(30) << to_string(opcode);
                if (exception_being_thrown) std::cout << " (exception in progress)";
                if (exception_being_handled) std::cout << " (exception being handled)";
                std::cout << std::endl;
#endif

                //last_time = std::chrono::high_resolution_clock::now();
                switch(opcode) {
                case ExecuteOp::_Count:
                    assert(false);
                    break;
                case ExecuteOp::ClassInit: {
                    auto &code_object = stacktrace_current->runtime_function->code;
                    auto line = code_object.get_line_by_position(code_pos - 1);
                    auto name = code_pos.decode<std::string_view>();
                    auto full_name = code_pos.decode<std::string_view>();
                    auto cls = current_vm().allocate<Class>(0, line, name, full_name, code_object);
                    PUSH_STATE(ClassState{});
                    STATE(ClassState).cls = cls;
                    break; }
                case ExecuteOp::ClassCreate: {
                    auto cls = STATE(ClassState).cls;
                    POP_STATE();

                    auto native = code_pos.decode<bool>();
                    auto base_class_count = code_pos.decode<std::uint32_t>();
                    auto member_count = code_pos.decode<std::uint32_t>();
                    auto all_variable_names = code_pos.decode<bool>();
                    auto variable_name_count = code_pos.decode<std::uint32_t>();

                    if (native) {
                        auto &native_provider = cls->code.native_code_provider();
                        if (native_provider) {
                            if (auto impl = native_provider->native_class(cls->full_name)) {
                                auto size = impl->native_storage_size();
                                cls->initialize_set_native_class_info(impl->get_token(), size);
                                cls->native = std::move(impl);
                            }
                        }
                        if (!cls->native) {
                            throw_missing_native(std::format("missing native class {}", cls->full_name));
                        }
                    }


                    auto base_classes = PEEK_VALUES(base_class_count, base_class_count);
                    auto members = PEEK_VALUES(member_count + base_class_count, member_count);
                    for(auto b : base_classes) {
                        cls->initialize_add_base_class(b.as_class());
                    }
                    for(auto f : members) {
                        cls->initialize_add_function(f.as_functions());
                    }
                    if (all_variable_names) {
                        cls->initialize_set_all_variables();
                    }
                    else {
                        for(auto i = 0u; i < variable_name_count; ++i) {
                            auto var_name = code_pos.decode<std::string_view>();
                            cls->initialize_add_variable(var_name);
                        }
                    }

                    cls->finalize_initializing();

                    POP_VALUES(base_class_count + member_count);
                    PUSH_VALUE(OwcaClass{ cls });
                    break; }
                case ExecuteOp::ExprPopAndIgnore: {
                    POP_VALUES(1);
                    break; }
#define OPER2_RUN(oper) do { \
        auto left = temporary_ptr[2]; \
        auto right = temporary_ptr[1]; \
        temporary_ptr[2] = OPER2_GET(oper, left.kind(), right.kind())(left, right); \
        POP_VALUES(1); \
    } while(0)
#define CMP2_RUN(oper, reverse, upd) do { \
        auto jump_dest = code_pos.decode_jump();        \
        const auto last = code_pos.decode<bool>();      \
        auto left = PEEK_VALUE(2);                                                                 \
        auto right = PEEK_VALUE(1);                                                                 \
        auto res = (!reverse) ?                                                                      \
            OPER2_GET(oper, left.kind(), right.kind())(left, right) :                        \
            OPER2_GET(oper, right.kind(), left.kind())(right, left);                         \
        res = (upd);                                                                                \
        if (res) {                                                                                  \
            PEEK_VALUE(2) = last ? OwcaValue{ true } : right;                                                \
        }                                                                                           \
        else {                                                                                      \
            PEEK_VALUE(2) = false;                                                                           \
            code_pos = jump_dest;                                                                   \
        }                                                                                           \
        POP_VALUES(1);                                                                              \
    } while(0)

                case ExecuteOp::ExprCompareEq: {
                    CMP2_RUN(eq, 0, res);
                    break; }
                case ExecuteOp::ExprCompareNotEq: {
                    CMP2_RUN(eq, 0, !res);
                    break; }
                case ExecuteOp::ExprCompareLess: {
                    CMP2_RUN(less, 0, res);
                    break; }
                case ExecuteOp::ExprCompareMoreEq: {
                    CMP2_RUN(less, 0, !res);
                    break; }
                case ExecuteOp::ExprCompareMore: {
                    CMP2_RUN(less, 1, res);
                    break; }
                case ExecuteOp::ExprCompareLessEq: {
                    CMP2_RUN(less, 1, !res);
                    break; }
                case ExecuteOp::ExprCompareIs: {
                    CMP2_RUN(is, 0, res);
                    break; }
                case ExecuteOp::ExprConstantEmpty: {
                    PUSH_VALUE(OwcaEmpty{});
                    break; }
                case ExecuteOp::ExprConstantBool: {
                    auto value = code_pos.decode<bool>();
                    PUSH_VALUE(value);
                    break; }
                case ExecuteOp::ExprConstantFloat: {
                    auto value = code_pos.decode<Number>();
                    PUSH_VALUE(value);
                    break; }
                case ExecuteOp::ExprConstantString: {
                    auto value = code_pos.decode<std::string_view>();
                    PUSH_VALUE(current_vm().create_string_from_view(value));
                    break; }
                case ExecuteOp::ExprConstantStringInterpolated: {
                    auto strings = code_pos.decode<std::string_view>();
                    auto expr_count = code_pos.decode<std::uint32_t>();
                    size_t size = strings.size();
                    auto values = PEEK_VALUES(expr_count, expr_count);
                    for(auto i = 0u; i < expr_count; ++i) {
                        size += values[i].as_string_certainly().size();
                    }
                    auto new_str = current_vm().precreate_string(size);
                    auto new_str_pt = new_str->pointer();
                    const char *strings_ptr = strings.data();
                    for(auto i = 0u; i < expr_count; ++i) {
                        auto sz = code_pos.decode<std::uint32_t>();
                        std::memcpy(new_str_pt, strings_ptr, sz);
                        new_str_pt += sz;
                        strings_ptr += sz;
                        auto str = values[i].as_string_certainly();
                        std::memcpy(new_str_pt, str.text().data(), str.size());
                        new_str_pt += str.size();
                    }
                    auto remaining = strings.data() + strings.size() - strings_ptr;
                    std::memcpy(new_str_pt, strings_ptr, remaining);
                    new_str_pt += remaining;
                    assert(new_str_pt == new_str->pointer() + new_str->size());
                    POP_VALUES(expr_count);
                    PUSH_VALUE(OwcaString{ new_str });
                    break; }
                case ExecuteOp::ExprIdentifierRead: {
                    auto index = code_pos.decode<std::uint32_t>();
                    PUSH_VALUE(LOCAL_VAR(index));
                    break; }
                case ExecuteOp::ExprIdentifierWrite: {
                    auto index = code_pos.decode<std::uint32_t>();
                    LOCAL_VAR(index) = PEEK_VALUE(1);
                    break; }
                case ExecuteOp::ExprIdentifierFunctionWrite: {
                    auto index = code_pos.decode<std::uint32_t>();
                    auto &val = PEEK_VALUE(1);
                    auto &tgt = LOCAL_VAR(index);
                    tgt = set_identifier_function(tgt, val);
                    val = tgt;
                    break; }
                case ExecuteOp::ExprGlobalRead: {
                    auto index = code_pos.decode<std::uint32_t>();
                    PUSH_VALUE(globals_ptr[index]);
                    break; }
                case ExecuteOp::ExprGlobalWrite: {
                    auto index = code_pos.decode<std::uint32_t>();
                    globals_ptr[index] = PEEK_VALUE(1);
                    break; }
                case ExecuteOp::ExprGlobalFunctionWrite: {
                    auto index = code_pos.decode<std::uint32_t>();
                    auto &val = PEEK_VALUE(1);
                    auto &tgt = globals_ptr[index];
                    tgt = set_identifier_function(tgt, val);
                    val = tgt;
                    break; }
                case ExecuteOp::ExprMemberRead: {
                    auto self = PEEK_VALUE(1);
                    auto member = code_pos.decode<std::string_view>();
                    PEEK_VALUE(1) = current_vm().member(self, member);
                    break; }
                case ExecuteOp::ExprMemberWrite: {
                    auto val_to_write = PEEK_VALUE(1);
                    auto self = PEEK_VALUE(2);
                    auto member = code_pos.decode<std::string_view>();
                    current_vm().member(self, member, val_to_write);
                    PEEK_VALUE(2) = val_to_write;
                    POP_VALUES(1);
                    break; }
                case ExecuteOp::ExprOper1BinNeg: {
                    auto &left = PEEK_VALUE(1);
                    left = -(std::int64_t)left.as_float();
                    break; }
                case ExecuteOp::ExprOper1LogNot: {
                    auto &left = PEEK_VALUE(1);
                    left = !left.is_true();
                    break; }
                case ExecuteOp::ExprOper1Negate: {
                    auto &left = PEEK_VALUE(1);
                    left = -left.as_float();
                    break; }
                case ExecuteOp::ExprRetTrueAndJumpIfTrue: {
                    auto jump_dest = code_pos.decode_jump();
                    if (PEEK_VALUE(1).is_true()) {
                        code_pos = jump_dest;
                    }
                    else {
                        POP_VALUES(1);
                    }
                    break; }
                case ExecuteOp::ExprRetFalseAndJumpIfFalse: {
                    auto jump_dest = code_pos.decode_jump();
                    if (!PEEK_VALUE(1).is_true()) {
                        code_pos = jump_dest;
                    }
                    else {
                        POP_VALUES(1);
                    }
                    break; }
                case ExecuteOp::ExprToString: {
                    auto v = PEEK_VALUE(1);
                    if (v.kind() == OwcaValueKind::String) {
                        break;
                    }
                    PEEK_VALUE(1) = current_vm().create_string_from_view(v.to_string());
                    break; }
                case ExecuteOp::ExprToIterator: {
                    auto &val = PEEK_VALUE(1);
                    if (val.kind() != OwcaValueKind::Iterator) {
                        auto func = current_vm().try_member(val, "__iter__");
                        if (!func) {
                            throw_not_iterable(val.type());
                        }
                        val = *func;
                        val = execute_call_from_values(temporary_ptr, 1);
                    }
                    break; }
                case ExecuteOp::ExprOper2BinOr: {
                    OPER2_RUN(bin_or); 
                    break; }
                case ExecuteOp::ExprOper2BinAnd: {
                    OPER2_RUN(bin_and);
                    break; }
                case ExecuteOp::ExprOper2BinXor: {
                    OPER2_RUN(bin_xor);
                    break; }
                case ExecuteOp::ExprOper2BinLShift: {
                    OPER2_RUN(bin_lshift);
                    break; }
                case ExecuteOp::ExprOper2BinRShift: {
                    OPER2_RUN(bin_rshift);
                    break; }
                case ExecuteOp::ExprOper2Add: {
                    OPER2_RUN(add);
                    break; }
                case ExecuteOp::ExprOper2Sub: {
                    OPER2_RUN(sub);
                    break; }
                case ExecuteOp::ExprOper2Mul: {
                    OPER2_RUN(mul);
                    break; }
                case ExecuteOp::ExprOper2Div: {
                    OPER2_RUN(div);
                    break; }
                case ExecuteOp::ExprOper2Mod: {
                    OPER2_RUN(mod);
                    break; }
                case ExecuteOp::ExprOper2MakeRange: {
                    auto mode = code_pos.decode<std::uint8_t>();
                    Number first, second, third;
                    if (mode & 4) {
                        third = PEEK_VALUE(1).as_float();
                        POP_VALUES(1);
                    }
                    else {
                        third = 1;
                    }
                    if (mode & 2) {
                        second = PEEK_VALUE(1).as_float();
                        POP_VALUES(1);
                    }
                    else {
                        second = std::numeric_limits<Number>::max();
                    }
                    if (mode & 1) {   
                        first = PEEK_VALUE(1).as_float();
                        POP_VALUES(1);
                    }
                    else {
                        first = 0;
                    }
                    if (third == 0) {
                        throw_range_step_is_zero();
                    }
                    auto ret = current_vm().allocate<Range>(0);
                    ret->from = first;
                    ret->to = second;
                    ret->step = third;
                    PUSH_VALUE(OwcaRange{ ret });
                    break; }
                case ExecuteOp::ExprOper2IndexRead: {
                    auto key = PEEK_VALUE(1);
                    auto self = PEEK_VALUE(2);
                    auto &ret = PEEK_VALUE(2);
                    POP_VALUES(1);
                    ret = index_read(self, key);
                    break; }
                case ExecuteOp::ExprOper2IndexWrite: {
                    auto value = PEEK_VALUE(1);
                    auto key = PEEK_VALUE(2);
                    auto &self = PEEK_VALUE(3);
                    POP_VALUES(2);
                    self = index_write(self, key, value);
                    break; }
                case ExecuteOp::ExprOperXCall: {
                    auto size = code_pos.decode<std::uint32_t>();
                    PEEK_VALUE(size) = execute_call_from_values(temporary_ptr, size);
                    POP_VALUES(size - 1);
                    break; }
                case ExecuteOp::ExprOperXCreateArray: {
                    auto size = code_pos.decode<std::uint32_t>();
                    auto args = PEEK_VALUES(size, size);
                    auto arguments = std::deque<OwcaValue>{ args.begin(), args.end() };
                    POP_VALUES(size);
                    PUSH_VALUE(current_vm().create_array(std::move(arguments)));
                    break; }
                case ExecuteOp::ExprOperXCreateTuple: {
                    auto size = code_pos.decode<std::uint32_t>();
                    auto args = PEEK_VALUES(size, size);
                    auto arguments = std::vector<OwcaValue>{ args.begin(), args.end() };
                    POP_VALUES(size);
                    PUSH_VALUE(current_vm().create_tuple(std::move(arguments)));
                    break; }
                case ExecuteOp::ExprOperXCreateSet: {
                    auto size = code_pos.decode<std::uint32_t>();
                    auto args = PEEK_VALUES(size, size);
                    POP_VALUES(size);
                    PUSH_VALUE(current_vm().create_set(args));
                    break; }
                case ExecuteOp::ExprOperXCreateMap: {
                    auto size = code_pos.decode<std::uint32_t>();
                    auto args = PEEK_VALUES(size, size);
                    POP_VALUES(size);
                    PUSH_VALUE(current_vm().create_map(args));
                    break; }
                case ExecuteOp::ForInit: {
                    auto iterator = PEEK_VALUE(1).as_iterator();
                    POP_VALUES(1);
                    PUSH_STATE(ForState{ iterator });
                    auto &state = STATE(ForState);
                    state.end_position = code_pos.decode_jump();
                    state.loop_control_depth = code_pos.decode<std::uint8_t>();
                    state.continue_position = code_pos;
                    break; }
                case ExecuteOp::ForCondition: {
                    auto &state = STATE(ForState);
                    if (state.iterator.completed()) [[unlikely]] {
                        code_pos = state.end_position;
                        break;
                    }
                    auto val = continue_iterator(state.iterator);
                    if (!val) [[unlikely]] {
                        code_pos = state.end_position;
                        break;
                    }
                    PUSH_VALUE(*val);
                    break; }
                case ExecuteOp::ForCompleted: {
                    auto &state = STATE(ForState);
                    POP_STATE();
                    break; }
                case ExecuteOp::Function: {
                    PUSH_VALUE(create_function(code_pos, globals_ptr, locals_ptr));
                    break; }
                case ExecuteOp::If: {
                    auto val = PEEK_VALUE(1).is_true();
                    POP_VALUES(1);
                    auto else_position = code_pos.decode_jump();
                    if (!val) {
                        code_pos = else_position;
                    }
                    break; }
                case ExecuteOp::ReturnCloseIterator: {
                    complete_all(temporary_ptr);
                    return { OwcaCompleted{}, temporary_ptr, code_pos };
                    }
                case ExecuteOp::Return: {
                    complete_all(temporary_ptr);
                    return { OwcaEmpty{}, temporary_ptr, code_pos };
                    }
                case ExecuteOp::ReturnValue: {
                    auto val = PEEK_VALUE(1);
                    POP_VALUES(1);
                    complete_all(temporary_ptr);
                    return { val, temporary_ptr, code_pos };
                    }
                case ExecuteOp::Throw: {
                    auto exception = PEEK_VALUE(1);
                    POP_VALUES(1);
                    throw exception.as_exception();
                    }
                case ExecuteOp::TryInit: {
                    PUSH_STATE(TryState{temporary_ptr});
                    auto &state = STATE(TryState);
                    state.begin_position = code_pos.decode_jump();
                    state.end_position = code_pos.decode_jump();
                    state.catches_pos = code_pos;
                    code_pos = state.begin_position;
                    state.temporary_ptr = temporary_ptr;
                    state.original_exception_being_handled = exception_being_handled;
                    break; }
                case ExecuteOp::TryCompleted: {
                    if (auto state = TRY_STATE(TryState)) {
                        assert(exception_being_handled == state->original_exception_being_handled);
                    }
                    else if (auto state = TRY_STATE(CatchState)) {
                        assert(exception_being_handled == state->original_exception_being_handled);
                    }
                    else {
                        assert(false);
                    }
                    POP_STATE();
                    break; }
                case ExecuteOp::TryCatchType: {
                    auto values = code_pos.decode<std::uint32_t>();
                    auto ident = code_pos.decode<std::uint32_t>();
                    auto skip_jump = code_pos.decode_jump();

                    auto exc_types = PEEK_VALUES(values, values);
                    POP_VALUES(values);
                    assert(exception_being_thrown);

                    bool found = false;
                    for(auto e : exc_types) {
                        auto exc_type = e.as_class();
                        if (exception_being_thrown->type().has_base_class(exc_type)) {
                            found = true;
                            break;
                        }
                    }
                    if (found) {
                        if (ident != std::numeric_limits<std::uint32_t>::max()) {
                            LOCAL_VAR(ident) = *exception_being_thrown;
                        }
                        auto &state = STATE(TryState);
                        auto original_exception_being_handled = state.original_exception_being_handled;
                        POP_STATE();
                        PUSH_STATE(CatchState{});
                        auto &state2 = STATE(CatchState);
                        state2.exception_being_handled = exception_being_handled = exception_being_thrown;
                        state2.original_exception_being_handled = original_exception_being_handled;
                    }
                    else {
                        code_pos = skip_jump;
                    }
                    break; }
                case ExecuteOp::TryCatchTypeCompleted: {
                    auto &state = STATE(TryState);
                    POP_STATE();
                    throw *exception_being_thrown;
                    }
                case ExecuteOp::TryBlockCompleted: {
                    auto &state = STATE(CatchState);
                    assert(exception_being_thrown);
                    assert(exception_being_handled);
                    exception_being_thrown = std::nullopt;
                    exception_being_handled = std::nullopt;
                    code_pos = code_pos.decode_jump();
                    break; }
                case ExecuteOp::WhileInit: {
                    PUSH_STATE(WhileState{});
                    auto &state = STATE(WhileState);
                    state.end_position = code_pos.decode_jump();
                    state.loop_control_depth = code_pos.decode<std::uint8_t>();
                    state.continue_position = code_pos;
                    break; }
                case ExecuteOp::WhileNext: {
                    auto &state = STATE(WhileState);

                    auto value = PEEK_VALUE(1).as_bool();
                    POP_VALUES(1);
                    if (!value) {
                        code_pos = state.end_position;
                    }
                    break; }
                case ExecuteOp::WhileCompleted: {
                    POP_STATE();
                    break; }
                case ExecuteOp::WithInit: {
                    PUSH_STATE(WithState{});
                    auto &state = STATE(WithState);
                    auto &obj = PEEK_VALUE(1);
                    state.context = obj;
                    obj = current_vm().member(obj, "__enter__");
                    obj = execute_call_from_values(temporary_ptr, 1);
                    state.entered = true;
                    auto index = code_pos.decode<std::uint32_t>();
                    if (index != std::numeric_limits<std::uint32_t>::max()) {
                        LOCAL_VAR(index) = obj;
                    }
                    POP_VALUES(1);
                    break; }
                case ExecuteOp::WithCompleted: {
                    auto &state = STATE(WithState);
                    complete(state, temporary_ptr);
                    POP_STATE();
                    break; }
                case ExecuteOp::Yield: {
                    auto val = PEEK_VALUE(1);
                    POP_VALUES(1);
                    return { val, temporary_ptr, code_pos };
                    }
                case ExecuteOp::Jump: {
                    auto dest = code_pos.decode_jump();
                    code_pos = dest;
                    break; }
                }
next_iteration:
                assert(stacktrace_current == stacktrace_current_copy);
                stacktrace_current_copy->code_position = code_pos;
#ifdef MEASURE
                auto end = std::chrono::high_resolution_clock::now();
                times[(size_t)opcode] += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                counts[(size_t)opcode]++;
#endif
                //std::cout << "setting code position (" << (void*)&frame.code_position << ") to " << frame.code_position << std::endl;
            }
        }
        catch(OwcaException oe) {
            process_thrown_exception(&code_pos, oe);
            goto restart;
        }

#ifdef MEASURE
        std::cout << "\n\n";
        for(auto i = 0u; i < (size_t)ExecuteOp::_Count; ++i) {
            auto t = times[i];
            auto c = counts[i];
            if (t > 0) {
                std::cout << std::setw(40) << to_string((Internal::ExecuteOp)i) << " " << (t / c) << " (count " << c << ")\n";
            }
        }
#endif
    }
    void Executor::complete_all(TemporariesPtr temporary_ptr) {
        auto sc = stacktrace_current;
        while(HAS_STATE()) {
            if (auto s = TRY_STATE(WithState)) {
                complete(*s, temporary_ptr);
            }
            POP_STATE();
        }
    }
    void Executor::complete(WithState state, TemporariesPtr temporary_ptr) {
        if (state.entered) {
            state.entered = false;
            auto mbm = current_vm().member(state.context, "__exit__");
            PUSH_VALUE(mbm);
            execute_call_from_values(temporary_ptr, 1);
            POP_VALUES(1);
        }
    }
    // template <typename Tag> std::string_view tag_name = "unknown";
    // template <> std::string_view tag_name<Executor::TagAdd> = "addition";
    // template <> std::string_view tag_name<Executor::TagSub> = "subtraction";
    // template <> std::string_view tag_name<Executor::TagMul> = "multiplication";
    // template <> std::string_view tag_name<Executor::TagDiv> = "division";
    // template <> std::string_view tag_name<Executor::TagMod> = "modulus";

    // Number Executor::expr_oper_2(Executor::TagAdd, Number left, Number right) {
    //     return left + right;
    // }
    // OwcaArray Executor::expr_oper_2(TagAdd, OwcaArray left, OwcaArray right) {
    //     auto ret = current_vm().allocate<Array>(0);
    //     ret->values = left.internal_value()->values;
    //     for(auto &q : right.internal_value()->values) {
    //         ret->values.push_back(q);
    //     }
    //     return OwcaArray{ ret };
    // }
    // OwcaTuple Executor::expr_oper_2(TagAdd, OwcaTuple left, OwcaTuple right) {
    //     auto ret = current_vm().allocate<Tuple>(0);
    //     ret->values.reserve(left.internal_value()->values.size() + right.internal_value()->values.size());
    //     for(auto &q : left.internal_value()->values) {
    //         ret->values.push_back(q);
    //     }
    //     for(auto &q : right.internal_value()->values) {
    //         ret->values.push_back(q);
    //     }
    //     return OwcaTuple{ ret };
    // }

    // OwcaString Executor::expr_oper_2(Executor::TagAdd, OwcaString left, OwcaString right) {
    //     return current_vm().create_string(left, right);
    // }
    // Number Executor::expr_oper_2(Executor::TagSub, Number left, Number right) {
    //     return left - right;
    // }
    // Number Executor::expr_oper_2(Executor::TagMul, Number left, Number right) {
    //     return left * right;
    // }
    // Number Executor::expr_oper_2(Executor::TagDiv, Number left, Number right) {
    //     if (right == 0) 
    //         throw_division_by_zero();
    //     return left / right;
    // }
    // Number Executor::expr_oper_2(Executor::TagMod, Number left, Number right) {
    //     if (right == 0)
    //         throw_division_by_zero();
    //     return (std::int64_t)left % (std::int64_t)right;
    // }
    // OwcaString Executor::expr_oper_2(Executor::TagMul, OwcaString left, Number right) {
    //     return current_vm().create_string(left, right);
    // }
    // OwcaString Executor::expr_oper_2(Executor::TagMul, Number left, OwcaString right) {
    //     return current_vm().create_string(right, left);
    // }
    // OwcaArray Executor::expr_oper_2(TagMul, OwcaArray left, Number right) {
    //     auto ret = current_vm().allocate<Array>(0);
    //     for(auto i = 0u; i < right; ++i) {
    //         ret->values.insert(ret->values.end(), left.internal_value()->values.begin(), left.internal_value()->values.end());
    //     }
    //     return OwcaArray{ ret };
    // }
    // OwcaArray Executor::expr_oper_2(TagMul, Number left, OwcaArray right) {
    //         auto ret = current_vm().allocate<Array>(0);
    //         for(auto i = 0u; i < left; ++i) {
    //             ret->values.insert(ret->values.end(), right.internal_value()->values.begin(), right.internal_value()->values.end());
    //         }
    //         return OwcaArray{ ret };
    // }
    // OwcaTuple Executor::expr_oper_2(TagMul, OwcaTuple left, Number right) {
    //         auto ret = current_vm().allocate<Tuple>(0);
    //         ret->values.reserve((size_t)(right * left.internal_value()->values.size()));
    //         for(auto i = 0u; i < right; ++i) {
    //             ret->values.insert(ret->values.end(), left.internal_value()->values.begin(), left.internal_value()->values.end());
    //         }
    //         return OwcaTuple{ ret };
    // }
    // OwcaTuple Executor::expr_oper_2(TagMul, Number left, OwcaTuple right) {
    //         auto ret = current_vm().allocate<Tuple>(0);
    //         ret->values.reserve((size_t)(left * right.internal_value()->values.size()));
    //         for(auto i = 0u; i < left; ++i) {
    //             ret->values.insert(ret->values.end(), right.internal_value()->values.begin(), right.internal_value()->values.end());
    //         }
    //         return OwcaTuple{ ret };
    // }
    // Number Executor::expr_oper_2(TagBinOr, Number left, Number right) {
    //     return (std::uint64_t)left | (std::uint64_t)right;
    // }
    // Number Executor::expr_oper_2(TagBinAnd, Number left, Number right) {
    //     return (std::uint64_t)left & (std::uint64_t)right;
    // }
    // Number Executor::expr_oper_2(TagBinXor, Number left, Number right) {
    //     return (std::uint64_t)left ^ (std::uint64_t)right;
    // }
    // Number Executor::expr_oper_2(TagBinLShift, Number left, Number right) {
    //     return (std::uint64_t)left << (std::uint64_t)right;
    // }
    // Number Executor::expr_oper_2(TagBinRShift, Number left, Number right) {
    //     return (std::uint64_t)left >> (std::uint64_t)right;
    // }

    // template <typename A, typename B, typename C> OwcaEmpty Executor::expr_oper_2(A, B b, C c) {
    //     throw_unsupported_operation_2(tag_name<A>, OwcaValue{ b }.type(), OwcaValue{ c }.type());
    // }
    // template <typename Tag> void Executor::run_impl_opcodes_execute_expr_oper2(TemporariesPtr &temporary_ptr) {
    //     auto right = PEEK_VALUE(1);
    //     auto left = PEEK_VALUE(2);
    //     auto &ret = PEEK_VALUE(2);
    //     ret = left.visit([&](auto left_val) -> OwcaValue {
    //         return right.visit([&](auto right_val) -> OwcaValue {
    //             return expr_oper_2(Tag{}, left_val, right_val);
    //         });
    //     });
    //     POP_VALUES(1);
    // }

    OwcaValue Executor::run_script_code(RuntimeFunctionScriptFunction *function, GlobalsPtr globals_ptr, TemporariesPtr temporary_ptr, unsigned int arg_count, bool clear_locals) {
        auto locals_ptr = temporary_ptr.locals(arg_count + 1);
        const auto max_values = function->max_values;
        temporary_ptr = temporary_ptr + max_values - arg_count;

        assert(locals_ptr.local_values_ptr + max_values + function->max_temporaries <= values_vector.data() + values_vector.size());
        if (clear_locals) [[likely]] {
            for(auto i = arg_count + 1; i < max_values; ++i) {
                LOCAL_VAR(i) = OwcaEmpty{};
            }
        }

        assert(function->copy_from_parents.size() == function->values_from_parents.size());

        for (auto i = 0u; i < function->copy_from_parents.size(); ++i) {
            LOCAL_VAR(function->copy_from_parents[i].index_in_child) = function->values_from_parents[i];
        }

        auto est = StackTraceState{ *this, function, function->entry_point };
        auto [ retval, new_values_ptr, new_code_pos ] = run_opcodes(globals_ptr, locals_ptr, temporary_ptr, function->entry_point);
        return retval;
    }
    Generator Executor::run_script_generator(Iterator *iter_object, RuntimeFunction *function, GlobalsPtr globals_ptr, std::vector<OwcaValue> values_vec, std::vector<StatesType> states_vec, CodePosition code_pos)
    {
        const auto locals_ptr = LocalsPtr{ values_vec.data() };
        const auto temporary_ptr = temporary_ptr_current_top;
        while(true) {
            OwcaValue val;
            {
                auto est = StackTraceState{ *this, function, code_pos };
                auto sc = stacktrace_current;
                std::swap(sc->states, states_vec);
                auto [ retval, new_temporary_ptr, new_code_pos ] = run_opcodes(globals_ptr, locals_ptr, temporary_ptr, code_pos);
                assert(sc == stacktrace_current);
                std::swap(sc->states, states_vec);
                assert(new_temporary_ptr.temporaries_ptr == temporary_ptr.temporaries_ptr);
                val = retval;
                code_pos = new_code_pos;
            }

            iter_object->first_time = false;
            if (val.kind() == OwcaValueKind::Completed) {
                break;
            }
            else {
                co_yield val;
            }
        }
    }

    std::optional<OwcaValue> Executor::continue_iterator(OwcaIterator oi) {
        if (!oi.internal_value()->generator) [[unlikely]] {
            return std::nullopt;
        }
        auto val = oi.internal_value()->generator->next();
        if (!val) {
            oi.internal_value()->generator.reset();
            return std::nullopt;
        }
        return val;
    }

	OwcaValue Executor::allocate_user_class_from_values(TemporariesPtr temporary_ptr, unsigned int arg_count) {
		OwcaValue obj;
        
        assert(arg_count > 0);
        
        auto locals_ptr = temporary_ptr.locals(arg_count);
        auto cls = LOCAL_VAR(0).as_class_certainly().internal_value();

		if (cls->allocator_override) {
			obj = cls->allocator_override();
		}
		else if (cls->reload_self) {
			obj = {};
		}
		else {
			obj = OwcaObject{ current_vm().allocate<Object>(cls->native_storage_total, cls) };
            if (auto exc = current_vm().is_exception(obj.as_object_certainly())) {
                obj = OwcaException{ obj.as_object_certainly().internal_value(), exc };
            }
		}

		auto it = cls->values.find(std::string_view{ "__init__" });
		if (it == cls->values.end()) {
			if (arg_count > 1) {
				throw_cant_call(std::format("type {} has no __init__ function defined - expected constructor's call with no parameters, instead got {} parameters", cls->full_name, arg_count - 1));
			}
            return obj;
		}
        if (auto state = std::get_if<RuntimeFunctions*>(&it->second)) [[likely]] {
            PEEK_VALUE(arg_count) = obj;
            auto retval = execute_function_call_from_values(*state, temporary_ptr, arg_count);
            if (!cls->reload_self) [[likely]] {
                retval = obj;
            }
            return retval;
        }
        throw_cant_call(std::format("type {} has __init__ variable, not a function", std::get<Class*>(it->second)->full_name));
	}
    OwcaValue Executor::execute_call_from_values(TemporariesPtr temporary_ptr, unsigned int argument_count) {
        auto func = PEEK_VALUE(argument_count);
        if (func.kind() == OwcaValueKind::Functions) [[likely]] {
            auto f = func.as_functions_certainly();
            auto runtime_functions = f.internal_value();
            PEEK_VALUE(argument_count) = f.self().value_or(OwcaValue{});
            
            return execute_function_call_from_values(runtime_functions, temporary_ptr, argument_count);
        }
        else {
            if (func.kind() == OwcaValueKind::Class) [[likely]] {
                return allocate_user_class_from_values(temporary_ptr, argument_count);
            }
            throw_cant_call(std::format("can't call {} with {} parameters", func.type(), argument_count - 1));
        }
    }
    OwcaValue Executor::execute_function_call_from_values(RuntimeFunctions* runtime_functions, TemporariesPtr temporary_ptr, unsigned int arg_count) {
        assert(arg_count > 0);
        auto runtime_function = runtime_functions->functions[arg_count - 1];
        if (!runtime_function) [[unlikely]] {
            auto tmp = std::string{ "function " };
            tmp += runtime_functions->name;
            throw_not_callable_wrong_number_of_params(std::move(tmp), arg_count - 1);
        }
        return runtime_function->call(*this, temporary_ptr);
    }


	OwcaNamespace Executor::execute_code_block(OwcaCode oc)
	{
        auto it = namespaces.find(oc.filename());
        if (it != namespaces.end()) {
            return OwcaNamespace{ it->second };
        }

#ifdef OWCA_SCRIPT_EXEC_LOG
        std::cout << "Executing code block from file " << oc.filename() << std::endl;
#endif        
        auto tpk = TopPtrsKeeper{ *this };
        auto code_pos = oc.code_position();

        auto global_count = code_pos.decode<std::uint32_t>();
        std::unordered_map<std::string_view, size_t> identifier_to_global_index;
        for(auto i = 0u; i < global_count; ++i) {
            auto ident = code_pos.decode<std::string_view>();
            identifier_to_global_index[ident] = i;
        }
        auto ns = current_vm().create_namespace(std::move(oc), std::move(identifier_to_global_index));
        namespaces.insert({ ns.internal_value()->code.filename(), ns});
        if (namespaces.size() > 1) {
            auto ns_it = namespaces.at(current_vm().builtin_filename);
            for(auto it : ns_it.internal_value()->identifier_to_global_index) {
                auto val = ns_it.internal_value()->globals[it.second];
                ns.try_member(it.first, val);
            }
        }

        auto temporary_ptr = temporary_ptr_current_top;
        auto locals_ptr = temporary_ptr.locals(0);
        auto globals_ptr = GlobalsPtr{ ns.internal_value()->globals.data() };

        auto function = current_vm().allocate<RuntimeFunctionScriptFunction>(0, ns.internal_value()->code, globals_ptr, std::string_view("main-code-block"), std::string_view("main-code-block"), false, code_pos);
        auto est = StackTraceState{ *this, function, function->entry_point };
        run_opcodes(globals_ptr, locals_ptr, temporary_ptr, code_pos);
        return ns;
    }

    OwcaValue Executor::allocate_user_class(Class *cls, std::span<OwcaValue> arguments) {
        auto tpk = TopPtrsKeeper{ *this };

        auto temporary_ptr = temporary_ptr_current_top;
        auto locals_ptr = temporary_ptr.locals(0);
        PUSH_VALUE(OwcaClass{ cls });
        for(auto &v : arguments) {
            PUSH_VALUE(v);
        }

        return allocate_user_class_from_values(temporary_ptr, (unsigned int)arguments.size() + 1);
    }
    OwcaValue Executor::execute_call(OwcaValue func, std::span<OwcaValue> arguments) {
        auto tpk = TopPtrsKeeper{ *this };

        auto temporary_ptr = temporary_ptr_current_top;
        auto locals_ptr = temporary_ptr.locals(0);
        LOCAL_VAR(0) = func;
        for(size_t i = 0; i < arguments.size(); ++i) {
            LOCAL_VAR(i + 1) = arguments[i];
        }
        return execute_call_from_values(temporary_ptr + (unsigned int)arguments.size() + 1, (unsigned int)arguments.size() + 1);
    }
	void Executor::throw_too_many_elements(size_t expected)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(std::format("too many values to unpack (expected {})", expected));
        throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}
	void Executor::throw_not_enough_elements(size_t expected, size_t got)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(std::format("not enough values to unpack (expected {}, got {})", expected, got));
        throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}
	void Executor::throw_dictionary_changed(bool is_dict)
	{
        OwcaValue temp_arg = is_dict ? current_vm().create_string_from_view("dictionary changed during iteration") : current_vm().create_string_from_view("set changed during iteration");
        throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}
	void Executor::throw_not_implemented(std::string_view msg)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(msg);
        throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}
	void Executor::throw_range_step_is_zero()
	{
        OwcaValue temp_arg = current_vm().create_string_from_view("range step is zero");
        throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}
	void Executor::throw_division_by_zero()
	{
        OwcaValue temp_arg = current_vm().create_string_from_view("division by zero");
        throw allocate_user_class(current_vm().c_math_exception, std::span{ &temp_arg, 1 }).as_exception();
	}
	void Executor::throw_mod_division_by_zero()
	{
        OwcaValue temp_arg = current_vm().create_string_from_view("modulo by zero");
        throw allocate_user_class(current_vm().c_math_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_cant_convert_to_float_message(std::string_view msg)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(msg);
        throw allocate_user_class(current_vm().c_math_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_cant_convert_to_float(std::string_view type)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(std::format("can't convert value of type `{}` to floating point", type));
        throw allocate_user_class(current_vm().c_math_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_cant_convert_to_integer(Number val)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(std::format("can't convert {} to integer", val));
        throw allocate_user_class(current_vm().c_math_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_cant_convert_to_integer(std::string_view type)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(std::format("can't convert {} to integer", type));
        throw allocate_user_class(current_vm().c_math_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_not_a_number(std::string_view type)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(std::format("{} is not a number", type));
        throw allocate_user_class(current_vm().c_math_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_overflow(std::string_view msg)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(msg);
        throw allocate_user_class(current_vm().c_math_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_range_step_must_be_one_in_left_side_of_write_assign()
	{
        OwcaValue temp_arg = current_vm().create_string_from_view("step of a range must be 1 in left side of write assignment");
		throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_cant_compare(CompareKind kind, std::string_view left, std::string_view right)
	{
		const char *oper;
		switch(kind) {
		case CompareKind::Is: oper = "is"; break;
		case CompareKind::Eq: oper = "=="; break;
		case CompareKind::NotEq: oper = "!="; break;
		case CompareKind::LessEq: oper = "<=>"; break;
		case CompareKind::MoreEq: oper = ">="; break;
		case CompareKind::Less: oper = "<"; break;
		case CompareKind::More: oper = ">"; break;
        case CompareKind::_Count: assert(false);
		}
        OwcaValue temp_arg = current_vm().create_string_from_view(std::format("can't execute {} {} {}", left, oper, right));
        throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_string_too_large(size_t size)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(std::format("string is too large ({} bytes)", size));
        throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}
	void Executor::throw_index_out_of_range(std::string msg)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(msg);
        throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_value_not_indexable(std::string_view type, std::string_view key_type)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(std::format("{} is not indexable with key {}", type, key_type));
		throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_missing_member(std::string_view type, std::string_view ident)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(std::format("{} doesn't have a member {}", type, ident));
		throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_cant_call(std::string_view msg)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(msg);
		throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_not_callable(std::string_view type)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(std::format("{} is not callable", type));
		throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}
	
	void Executor::throw_not_callable_wrong_number_of_params(std::string_view type, unsigned int params)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(std::format("{} is not callable - wrong number of parameters ({})", type, params));
		throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_wrong_type(std::string_view type, std::string_view expected)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(std::format("wrong type {} - expected {}", type, expected));
		throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_wrong_type(std::string_view msg)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(msg);
		throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_unsupported_operation_2(std::string_view oper, std::string_view left, std::string_view right)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(std::format("can't execute {} {} {}", left, oper, right));
		throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_invalid_operand_for_mul_string(std::string_view type, std::string_view val)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(std::format("can't multiply {} by {}", type, val));
		throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_missing_key(std::string_view key)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(std::format("missing key {}", key));
		throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_not_hashable(std::string_view type)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(std::format("{} is not hashable", type));
		throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_value_cant_have_fields(std::string_view type)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(std::format("{} can't have fields", type));
		throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_missing_native(std::string_view msg)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(msg);
		throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_not_iterable(std::string_view msg)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(msg);
		throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_readonly(std::string_view msg)
	{
        OwcaValue temp_arg = current_vm().create_string_from_view(msg);
		throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_cant_return_value_from_generator()
	{
        OwcaValue temp_arg = current_vm().create_string_from_view("can't return value from generator");
		throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}

	void Executor::throw_container_is_empty()
	{
        OwcaValue temp_arg = current_vm().create_string_from_view("container is empty");
		throw allocate_user_class(current_vm().c_invalid_operation_exception, std::span{ &temp_arg, 1 }).as_exception();
	}
    bool Executor::execute_compare_eq(OwcaValue left, OwcaValue right) {
        return OPER2_GET(eq, left.kind(), right.kind())(left, right);
    }
    bool Executor::execute_compare_less(OwcaValue left, OwcaValue right) {
        return OPER2_GET(less, left.kind(), right.kind())(left, right);
    }
    bool Executor::execute_compare_is(OwcaValue left, OwcaValue right) {
        return OPER2_GET(is, left.kind(), right.kind())(left, right);
    }
    
    bool Executor::execute_compare(OwcaValue left, OwcaValue right, CompareKind kind)
    {
        switch(kind) {
        case CompareKind::Eq: return execute_compare_eq(left, right);
        case CompareKind::NotEq: return !execute_compare_eq(left, right);
        case CompareKind::Less: return execute_compare_less(left, right);
        case CompareKind::MoreEq: return !execute_compare_less(left, right);
        case CompareKind::More: return execute_compare_less(right, left);
        case CompareKind::LessEq: return !execute_compare_less(right, left);
        case CompareKind::Is: return execute_compare_is(left, right);
        case CompareKind::_Count: break;
        }
        assert(false);
        return false;
    }

    void gc_mark_value(GenerationGC generation_gc, const Executor::WhileState &e) {
    }
    void gc_mark_value(GenerationGC generation_gc, const Executor::ClassState &e) {
        gc_mark_value(generation_gc, e.cls);
    }
    void gc_mark_value(GenerationGC generation_gc, const Executor::ForState &e) {
        gc_mark_value(generation_gc, e.iterator);
    }
    void gc_mark_value(GenerationGC generation_gc, const Executor::TryState &e) {
        if (e.original_exception_being_handled)
            gc_mark_value(generation_gc, *e.original_exception_being_handled);
    }
    void gc_mark_value(GenerationGC generation_gc, const Executor::CatchState &e) {
        if (e.exception_being_handled)
            gc_mark_value(generation_gc, *e.exception_being_handled);
        if (e.original_exception_being_handled)
            gc_mark_value(generation_gc, *e.original_exception_being_handled);
    }
    void gc_mark_value(GenerationGC generation_gc, const Executor::WithState &e) {
        gc_mark_value(generation_gc, e.context);
    }
    void gc_mark_value(GenerationGC generation_gc, const Executor::StatesType &e) {
        visit_variant(e,
            [&](const auto &a) { gc_mark_value(generation_gc, a); }
        );
    }
    void gc_mark_value(GenerationGC ggc, const Executor &e) {
        for(auto it : e.namespaces) {
            gc_mark_value(ggc, it.second);
        }
        for(auto v = e.values_vector.data(); v < e.temporary_ptr_current_top.temporaries_ptr; ++v) {
            gc_mark_value(ggc, *v);
        }
        for(auto sc = e.stacktrace_vector.data() + 1; sc <= e.stacktrace_current; ++sc) {
            gc_mark_value(ggc, sc->runtime_function);
            gc_mark_value(ggc, sc->states);
        }
        if (e.exception_being_thrown) {
            gc_mark_value(ggc, *e.exception_being_thrown);
        }
        if (e.exception_being_handled) {
            gc_mark_value(ggc, *e.exception_being_handled);
        }
    }
}