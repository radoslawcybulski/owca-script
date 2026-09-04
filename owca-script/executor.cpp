#include "owca-script/identifier_index.h"
#include "owca-script/owca_code.h"
#include "owca-script/variable_index.h"
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
#include <chrono>
#include <string_view>
#include <utility>

#ifdef DEBUG
#define OWCA_SCRIPT_EXEC_LOG
#endif

//#define MEASURE

namespace OwcaScript::Internal {
    enum class CompareResult : std::uint8_t;

    enum {
        Add = 0,
        Sub = 1,
        Mul = 2,
        Div = 3,
        Mod = 4,
        BinOr = 5,
        BinAnd = 6,
        BinXor = 7,
        BinLShift = 8,
        BinRShift = 9,
    };

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
        math_opers[Add] = op_add_cant;
        math_opers[Sub] = op_sub_cant;
        math_opers[Mul] = op_mul_cant;
        math_opers[Div] = op_div_cant;
        math_opers[Mod] = op_mod_cant;
        math_opers[BinAnd] = op_bin_and_cant;
        math_opers[BinOr] = op_bin_or_cant;
        math_opers[BinXor] = op_bin_xor_cant;
        math_opers[BinLShift] = op_bin_lshift_cant;
        math_opers[BinRShift] = op_bin_rshift_cant;

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
    static bool op_compare_is_bool_bool(OwcaValue left, OwcaValue right) { return left.as_bool_certainly() == right.as_bool_certainly(); }

    static bool op_compare_eq_nul_nul(OwcaValue left, OwcaValue right) { return true; }
    static bool op_compare_is_nul_nul(OwcaValue left, OwcaValue right) { return true; }

    static bool op_compare_eq_range_range(OwcaValue left, OwcaValue right) { return left.as_range_certainly() == right.as_range_certainly(); }
    static bool op_compare_is_range_range(OwcaValue left, OwcaValue right) { return left.as_range_certainly().is(right.as_range_certainly()); }

    static bool op_compare_eq_functions_functions(OwcaValue left, OwcaValue right) { return left.as_functions_certainly() == right.as_functions_certainly(); }
    static bool op_compare_is_functions_functions(OwcaValue left, OwcaValue right) { return left.as_functions_certainly().is(right.as_functions_certainly()); }

    static bool op_compare_eq_map_map(OwcaValue left, OwcaValue right) { return left.as_map_certainly() == right.as_map_certainly(); }
    static bool op_compare_is_map_map(OwcaValue left, OwcaValue right) { return left.as_map_certainly().is(right.as_map_certainly()); }

    static bool op_compare_eq_set_set(OwcaValue left, OwcaValue right) { return left.as_set_certainly() == right.as_set_certainly(); }
    static bool op_compare_is_set_set(OwcaValue left, OwcaValue right) { return left.as_set_certainly().is(right.as_set_certainly()); }

    static bool op_compare_eq_class_class(OwcaValue left, OwcaValue right) { return left.as_class_certainly() == right.as_class_certainly(); }
    static bool op_compare_is_class_class(OwcaValue left, OwcaValue right) { return left.as_class_certainly().is(right.as_class_certainly()); }

    static bool op_compare_eq_object_object(OwcaValue left, OwcaValue right) { return left.as_object_certainly() == right.as_object_certainly(); }
    static bool op_compare_is_object_object(OwcaValue left, OwcaValue right) { return left.as_object_certainly().is(right.as_object_certainly()); }

    static bool op_compare_eq_ptr_object_ptr_object(OwcaValue left, OwcaValue right) { return left.as_ptr_object_certainly() == right.as_ptr_object_certainly(); }
    static bool op_compare_is_ptr_object_ptr_object(OwcaValue left, OwcaValue right) { return left.as_ptr_object_certainly().is(right.as_ptr_object_certainly()); }

    static bool op_compare_eq_tuple_tuple(OwcaValue left, OwcaValue right) { return left.as_tuple_certainly() == right.as_tuple_certainly(); }
    static bool op_compare_lt_tuple_tuple(OwcaValue left, OwcaValue right) { return left.as_tuple_certainly() < right.as_tuple_certainly(); }
    static bool op_compare_is_tuple_tuple(OwcaValue left, OwcaValue right) { return left.as_tuple_certainly().is(right.as_tuple_certainly()); }

    static bool op_compare_eq_array_array(OwcaValue left, OwcaValue right) { return left.as_array_certainly() == right.as_array_certainly(); }
    static bool op_compare_lt_array_array(OwcaValue left, OwcaValue right) { return left.as_array_certainly() < right.as_array_certainly(); }
    static bool op_compare_is_array_array(OwcaValue left, OwcaValue right) { return left.as_array_certainly().is(right.as_array_certainly()); }

    static bool op_compare_eq_iterator_iterator(OwcaValue left, OwcaValue right) { return left.as_iterator_certainly() == right.as_iterator_certainly(); }
    static bool op_compare_is_iterator_iterator(OwcaValue left, OwcaValue right) { return left.as_iterator_certainly().is(right.as_iterator_certainly()); }

    static bool op_compare_eq_exception_exception(OwcaValue left, OwcaValue right) { return left.as_exception_certainly() == right.as_exception_certainly(); }
    static bool op_compare_is_exception_exception(OwcaValue left, OwcaValue right) { return left.as_exception_certainly().is(right.as_exception_certainly()); }

    static bool op_compare_eq_namespace_namespace(OwcaValue left, OwcaValue right) { return left.as_namespace_certainly() == right.as_namespace_certainly(); }
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
    static OwcaValue op_bin_sub_map_map(OwcaValue left, OwcaValue right) {
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

    #define OPER2_MATH_GET(oper, left_kind, right_kind) oper2_functions[static_cast<size_t>(left_kind) * OwcaValuesCount + static_cast<size_t>(right_kind)].math_opers[oper]
    #define OPER2_MATH_SET(oper, left_kind, right_kind, func) OPER2_MATH_GET(oper, left_kind, right_kind) = func

    #define OPER2_GET(oper, left_kind, right_kind) oper2_functions[static_cast<size_t>(left_kind) * OwcaValuesCount + static_cast<size_t>(right_kind)].oper
    #define OPER2_SET(oper, left_kind, right_kind, func) OPER2_GET(oper, left_kind, right_kind) = func
    Oper2TypeArray oper2_functions = []() {
        constexpr size_t kind_count = static_cast<size_t>(OwcaValueKind::_Count);
        Oper2TypeArray oper2_functions;

        OPER2_MATH_SET(Add, OwcaValueKind::Float, OwcaValueKind::Float, op_add_number_number);
        OPER2_MATH_SET(Sub, OwcaValueKind::Float, OwcaValueKind::Float, op_sub_number_number);
        OPER2_MATH_SET(Mul, OwcaValueKind::Float, OwcaValueKind::Float, op_mul_number_number);
        OPER2_MATH_SET(Div, OwcaValueKind::Float, OwcaValueKind::Float, op_div_number_number);
        OPER2_MATH_SET(Mod, OwcaValueKind::Float, OwcaValueKind::Float, op_mod_number_number);
        OPER2_MATH_SET(BinAnd, OwcaValueKind::Float, OwcaValueKind::Float, op_bin_and_number_number);
        OPER2_MATH_SET(BinOr, OwcaValueKind::Float, OwcaValueKind::Float, op_bin_or_number_number);
        OPER2_MATH_SET(BinXor, OwcaValueKind::Float, OwcaValueKind::Float, op_bin_xor_number_number);
        OPER2_MATH_SET(BinLShift, OwcaValueKind::Float, OwcaValueKind::Float, op_bin_lshift_number_number);
        OPER2_MATH_SET(BinRShift, OwcaValueKind::Float, OwcaValueKind::Float, op_bin_rshift_number_number);

        OPER2_MATH_SET(Add, OwcaValueKind::String, OwcaValueKind::String, op_add_string_string);
        OPER2_MATH_SET(Mul, OwcaValueKind::String, OwcaValueKind::Float, op_mul_string_number);
        OPER2_MATH_SET(Mul, OwcaValueKind::Float, OwcaValueKind::String, op_mul_number_string);

        OPER2_MATH_SET(Mul, OwcaValueKind::Array, OwcaValueKind::Float, op_mul_array_number);
        OPER2_MATH_SET(Mul, OwcaValueKind::Float, OwcaValueKind::Array, op_mul_number_array);

        OPER2_MATH_SET(Mul, OwcaValueKind::Tuple, OwcaValueKind::Float, op_mul_tuple_number);
        OPER2_MATH_SET(Mul, OwcaValueKind::Float, OwcaValueKind::Tuple, op_mul_number_tuple);

        OPER2_MATH_SET(BinAnd, OwcaValueKind::Map, OwcaValueKind::Map, op_bin_and_map_map);
        OPER2_MATH_SET(BinOr, OwcaValueKind::Map, OwcaValueKind::Map, op_bin_or_map_map);
        OPER2_MATH_SET(Sub, OwcaValueKind::Map, OwcaValueKind::Map, op_bin_sub_map_map);

        OPER2_MATH_SET(BinAnd, OwcaValueKind::Set, OwcaValueKind::Set, op_bin_and_set_set);
        OPER2_MATH_SET(BinOr, OwcaValueKind::Set, OwcaValueKind::Set, op_bin_or_set_set);
        OPER2_MATH_SET(BinXor, OwcaValueKind::Set, OwcaValueKind::Set, op_bin_xor_set_set);

        OPER2_SET(eq, OwcaValueKind::Float, OwcaValueKind::Float, op_compare_eq_number_number);
        OPER2_SET(is, OwcaValueKind::Float, OwcaValueKind::Float, op_compare_is_number_number);
        OPER2_SET(less, OwcaValueKind::Float, OwcaValueKind::Float, op_compare_lt_number_number);

        OPER2_SET(eq, OwcaValueKind::String, OwcaValueKind::String, op_compare_eq_string_string);
        OPER2_SET(is, OwcaValueKind::String, OwcaValueKind::String, op_compare_is_string_string);
        OPER2_SET(less, OwcaValueKind::String, OwcaValueKind::String, op_compare_lt_string_string);

        OPER2_SET(eq, OwcaValueKind::Bool, OwcaValueKind::Bool, op_compare_eq_bool_bool);
        OPER2_SET(is, OwcaValueKind::Bool, OwcaValueKind::Bool, op_compare_is_bool_bool);

        OPER2_SET(eq, OwcaValueKind::Range, OwcaValueKind::Range, op_compare_eq_range_range);
        OPER2_SET(is, OwcaValueKind::Range, OwcaValueKind::Range, op_compare_is_range_range);

        OPER2_SET(eq, OwcaValueKind::Functions, OwcaValueKind::Functions, op_compare_eq_functions_functions);
        OPER2_SET(is, OwcaValueKind::Functions, OwcaValueKind::Functions, op_compare_is_functions_functions);

        OPER2_SET(eq, OwcaValueKind::Empty, OwcaValueKind::Empty, op_compare_eq_nul_nul);
        OPER2_SET(eq, OwcaValueKind::Empty, OwcaValueKind::Completed, op_compare_eq_nul_nul);
        OPER2_SET(eq, OwcaValueKind::Completed, OwcaValueKind::Empty, op_compare_eq_nul_nul);
        OPER2_SET(eq, OwcaValueKind::Completed, OwcaValueKind::Completed, op_compare_eq_nul_nul);
        OPER2_SET(is, OwcaValueKind::Empty, OwcaValueKind::Empty, op_compare_is_nul_nul);
        OPER2_SET(is, OwcaValueKind::Completed, OwcaValueKind::Completed, op_compare_is_nul_nul);

        OPER2_SET(eq, OwcaValueKind::Set, OwcaValueKind::Set, op_compare_eq_set_set);
        OPER2_SET(is, OwcaValueKind::Set, OwcaValueKind::Set, op_compare_is_set_set);

        OPER2_SET(eq, OwcaValueKind::Map, OwcaValueKind::Map, op_compare_eq_map_map);
        OPER2_SET(is, OwcaValueKind::Map, OwcaValueKind::Map, op_compare_is_map_map);

        OPER2_SET(eq, OwcaValueKind::Class, OwcaValueKind::Class, op_compare_eq_class_class);
        OPER2_SET(is, OwcaValueKind::Class, OwcaValueKind::Class, op_compare_is_class_class);

        OPER2_SET(eq, OwcaValueKind::Object, OwcaValueKind::Object, op_compare_eq_object_object);
        OPER2_SET(is, OwcaValueKind::Object, OwcaValueKind::Object, op_compare_is_object_object);
        OPER2_SET(eq, OwcaValueKind::PtrObject, OwcaValueKind::PtrObject, op_compare_eq_ptr_object_ptr_object);
        OPER2_SET(is, OwcaValueKind::PtrObject, OwcaValueKind::PtrObject, op_compare_is_ptr_object_ptr_object);

        OPER2_SET(eq, OwcaValueKind::Tuple, OwcaValueKind::Tuple, op_compare_eq_tuple_tuple);
        OPER2_SET(is, OwcaValueKind::Tuple, OwcaValueKind::Tuple, op_compare_is_tuple_tuple);
        OPER2_SET(less, OwcaValueKind::Tuple, OwcaValueKind::Tuple, op_compare_lt_tuple_tuple);

        OPER2_SET(eq, OwcaValueKind::Array, OwcaValueKind::Array, op_compare_eq_array_array);
        OPER2_SET(is, OwcaValueKind::Array, OwcaValueKind::Array, op_compare_is_array_array);
        OPER2_SET(less, OwcaValueKind::Array, OwcaValueKind::Array, op_compare_lt_array_array);

        OPER2_SET(eq, OwcaValueKind::Iterator, OwcaValueKind::Iterator, op_compare_eq_iterator_iterator);
        OPER2_SET(is, OwcaValueKind::Iterator, OwcaValueKind::Iterator, op_compare_is_iterator_iterator);

        OPER2_SET(eq, OwcaValueKind::Namespace, OwcaValueKind::Namespace, op_compare_eq_namespace_namespace);
        OPER2_SET(is, OwcaValueKind::Namespace, OwcaValueKind::Namespace, op_compare_is_namespace_namespace);

        return oper2_functions;
    }();

#ifdef MEASURE
    struct MeasureItem {
        std::uint64_t opcode : 6 = 0;
        std::uint64_t time : 58 = 0;

        MeasureItem() = default;
        MeasureItem(Internal::ExecuteOp op) {
            auto n = std::chrono::high_resolution_clock::now();
            time = n.time_since_epoch().count();
            opcode = static_cast<std::uint32_t>(op);
        }
    };
    static std::vector<MeasureItem> measure_items{ 4ull * 1024 * 1024 * 1024 };
    static std::uint32_t measure_item = 0;

    static void process_measure_items() {
        std::array<std::uint64_t, 64> times;
        std::array<std::uint64_t, 64> counts;

        for(auto &v : times) v = 0;
        for(auto &v : counts) v = 0;

        for(auto i = 1u; i < measure_item; ++i) {
            auto rt = measure_items[i].time - measure_items[i - 1].time;
            auto op = measure_items[i - 1].opcode;
            times[op] += rt;
            counts[op] += 1;
        }

        std::cout << "measure results:" << std::endl;
        for(auto i = 0u; i < times.size(); ++i) {
            if (counts[i] > 100) {
                std::cout << "opcode " << to_string(static_cast<Internal::ExecuteOp>(i)) << ": count = " << counts[i] << ", total time = " << times[i] << ", average time = " << (times[i] / counts[i]) << std::endl;
            }
        }
    }
#endif

    using Oper1TypeArray = std::array<Operators1, OwcaValuesCount>;

    static OwcaValue op_call_cant(size_t count) {
        auto locals = current_vm().get_executor().get_current_unused_locals_ptr();
        current_vm().throw_cant_call(std::format("can't call {} with {} parameters", locals[0].type(), count - 1));
    };
    static OwcaValue op_call_functions(size_t count) {
        return current_vm().get_executor().execute_function_call_from_values(count);
    };
    static OwcaValue op_call_class(size_t count) {
        return current_vm().get_executor().allocate_user_class_from_values(count);
    };

    static bool op_is_true_empty(OwcaValue self) { return false; }
    static bool op_is_true_completed(OwcaValue self) { return false;}
    static bool op_is_true_range(OwcaValue self) { return true;}
    static bool op_is_true_bool(OwcaValue self) { return self.as_bool_certainly(); }
    static bool op_is_true_float(OwcaValue self) { return self.as_float_certainly() != 0; }
    static bool op_is_true_string(OwcaValue self) { return (bool)self.as_string_certainly(); }
    static bool op_is_true_functions(OwcaValue self) { return true; }
    static bool op_is_true_map(OwcaValue self) { return (bool)self.as_map_certainly(); }
    static bool op_is_true_set(OwcaValue self) { return (bool)self.as_set_certainly(); }
    static bool op_is_true_class(OwcaValue self) { return true; }
    static bool op_is_true_object(OwcaValue self) { return true; }
    static bool op_is_true_tuple(OwcaValue self) { return (bool)self.as_tuple_certainly(); }
    static bool op_is_true_array(OwcaValue self) { return (bool)self.as_array_certainly(); }
    static bool op_is_true_iterator(OwcaValue self) { return (bool)self.as_iterator_certainly(); }
    static bool op_is_true_namespace(OwcaValue self) { return true; }

    #define OPER1_GET(oper, left_kind) oper1_functions[static_cast<size_t>(left_kind)].oper
    #define OPER1_SET(oper, left_kind, func) OPER1_GET(oper, left_kind) = func
    Oper1TypeArray oper1_functions = []() {
        Oper1TypeArray oper1_functions;

        for(auto i =0u; i < OwcaValuesCount; ++i) {
            OPER1_SET(call, (OwcaValueKind)i, op_call_cant);
        }
        OPER1_SET(call, OwcaValueKind::Functions, op_call_functions);
        OPER1_SET(call, OwcaValueKind::Class, op_call_class);
        OPER1_SET(is_true, OwcaValueKind::Empty, op_is_true_empty);
        OPER1_SET(is_true, OwcaValueKind::Completed, op_is_true_completed);
        OPER1_SET(is_true, OwcaValueKind::Range, op_is_true_range);
        OPER1_SET(is_true, OwcaValueKind::Bool, op_is_true_bool);
        OPER1_SET(is_true, OwcaValueKind::Float, op_is_true_float);
        OPER1_SET(is_true, OwcaValueKind::String, op_is_true_string);
        OPER1_SET(is_true, OwcaValueKind::Functions, op_is_true_functions);
        OPER1_SET(is_true, OwcaValueKind::Map, op_is_true_map);
        OPER1_SET(is_true, OwcaValueKind::Set, op_is_true_set);
        OPER1_SET(is_true, OwcaValueKind::Class, op_is_true_class);
        OPER1_SET(is_true, OwcaValueKind::Object, op_is_true_object);
        OPER1_SET(is_true, OwcaValueKind::Tuple, op_is_true_tuple);
        OPER1_SET(is_true, OwcaValueKind::Array, op_is_true_array);
        OPER1_SET(is_true, OwcaValueKind::Iterator, op_is_true_iterator);
        OPER1_SET(is_true, OwcaValueKind::Namespace, op_is_true_namespace);

        return oper1_functions;
    }();


    Executor::Executor() : stacktrace_vector(1024), values_vector(1024 * 1024), current_unused_locals_ptr(values_vector.data()) {
        stacktrace_current = stacktrace_vector.data();
    }

    Executor::~Executor() {
#ifdef MEASURE
        process_measure_items();
#endif
    }

    void Executor::process_thrown_exception(CodePosition *code_pos, OwcaException exception)
    {
        exception_being_thrown = exception;
        auto sf = static_cast<RuntimeFunctionScript*>(stacktrace_current->runtime_function);
        current_try_with_block_when_thrown = nullptr;
        for(auto i = sf->try_with_blocks.size(); i > 0; --i) {
            auto &cb = sf->try_with_blocks[i - 1];
            if (code_pos->value() > cb.begin.value() && code_pos->value() <= cb.end.value()) {
                current_try_with_block_when_thrown = &cb;
                break;
            }
        }
        if (!current_try_with_block_when_thrown) throw exception;

        *code_pos = current_try_with_block_when_thrown->jump;
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

    OwcaValue Executor::create_function(CodePosition &code_pos, const IdentifierPtrs &identifier_ptrs)
    {
        auto &code_object = stacktrace_current->runtime_function->code;
        auto name_index = code_pos.decode<IdentifierIndex>();
        auto name = code_pos.decode<std::string_view>();
        auto full_name = code_pos.decode<std::string_view>();
        auto is_native = code_pos.decode<bool>();
        auto is_generator = code_pos.decode<bool>();
        auto is_method = code_pos.decode<bool>();
        auto param_count = code_pos.decode<std::uint16_t>();
        auto value_count = code_pos.decode<std::uint16_t>();
        auto identifier_count = code_pos.decode<std::uint16_t>();
        std::vector<std::string_view> identifier_names;
        identifier_names.resize(identifier_count);
        for(auto &n : identifier_names) n = code_pos.decode<std::string_view>();

        RuntimeFunction *fnc = nullptr;
        if (is_native) {
            auto &native_provider = code_object.native_code_provider();
            auto line = code_object.get_line_by_position(code_pos).line;
            if (is_generator) {
                auto f = current_vm().allocate<RuntimeFunctionNativeGenerator>(0, code_object, stacktrace_current->runtime_function->owning_namespace, name_index, name, full_name, is_method, line);
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
                auto f = current_vm().allocate<RuntimeFunctionNativeFunction>(0, code_object, stacktrace_current->runtime_function->owning_namespace, name_index, name, full_name, is_method, line);
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
            auto copy_from_parent_count = code_pos.decode<std::uint32_t>();
            std::vector<OwcaValue> values_from_parents;
            values_from_parents.reserve(copy_from_parent_count);
            for(auto i = 0u; i < copy_from_parent_count; ++i) {
                auto index = code_pos.decode<VariableIndex>();
                values_from_parents.push_back(identifier_ptrs[index]);
            }
            auto z = code_pos.decode_jump();
            auto entry_point = code_pos;
            code_pos = z;
            auto count = code_pos.decode<std::uint32_t>();
            std::vector<TryWithBlockInfo> try_with_blocks;
            try_with_blocks.resize(count);
            for(auto &block : try_with_blocks) {
                block.begin = code_pos.decode_jump();
                block.end = code_pos.decode_jump();
                block.jump = code_pos.decode_jump();
                block.next = code_pos.decode<std::uint32_t>();
            }
            
            RuntimeFunctionScript *f;
            if (is_generator) {
                f = current_vm().allocate<RuntimeFunctionScriptGenerator>(0, code_object, stacktrace_current->runtime_function->owning_namespace, identifier_ptrs.get_globals_pointer(), name_index, name, full_name, is_method, entry_point);
            }
            else {
                f = current_vm().allocate<RuntimeFunctionScriptFunction>(0, code_object, stacktrace_current->runtime_function->owning_namespace, identifier_ptrs.get_globals_pointer(), name_index, name, full_name, is_method, entry_point);
            }
            f->try_with_blocks = std::move(try_with_blocks);
            fnc = f;
            f->identifier_names = std::move(identifier_names);
            f->values_from_parents = std::move(values_from_parents);
        }
        fnc->param_count = param_count;
        fnc->max_values = value_count;
        auto rfs = current_vm().allocate<RuntimeFunctions>(0, name_index, name, full_name);
        rfs->functions[fnc->param_count] = fnc;
        return OwcaFunctions{ rfs };
    }

    std::tuple<OwcaValue, CodePosition> Executor::run_opcodes(const LocalsPtr locals_ptr, CodePosition code_pos)
    {
        auto sf = static_cast<RuntimeFunctionScript*>(stacktrace_current->runtime_function);
        IdentifierPtrs identifier_ptrs{
            locals_ptr.local_values_ptr,
            sf->constants_ptr,
            sf->globals_ptr.global_values_ptr,
            sf->values_from_parents.data(),
        };

        auto * const stacktrace_current_copy = stacktrace_current;
#ifdef OWCA_SCRIPT_EXEC_LOG
        auto &code_object = stacktrace_current->runtime_function->code;
#endif

restart:
        try {
            for(;;) {
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
                std::cout << "Running opcode at line " << std::setw(4) << line.line << " position " << std::setw(5) << (code_pos.value() - stacktrace_current->runtime_function->code.code().data() - 1 + stacktrace_current->runtime_function->code.code_offset()) <<
                    " stack " << std::setw(2) << (stacktrace_current - stacktrace_vector.data()) <<
                    " opcode " << std::setw(30) << to_string(opcode);
                if (exception_being_thrown) std::cout << " (exception in progress)";
                if (exception_being_handled) std::cout << " (exception being handled)";
                std::cout << std::endl;
#endif

#ifdef MEASURE
                measure_items[measure_item++] = MeasureItem{ opcode };
#endif
                //last_time = std::chrono::high_resolution_clock::now();
                switch(opcode) {
                case ExecuteOp::_Count:
                    assert(false);
                    break;
                case ExecuteOp::ClassCreate: {
                    auto &code_object = stacktrace_current->runtime_function->code;
                    const auto line = code_object.get_line_by_position(code_pos - 1);
                    auto &dest = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    
                    auto name = code_pos.decode<std::string_view>();
                    auto full_name = code_pos.decode<std::string_view>();
                    auto native = code_pos.decode<bool>();
                    auto base_class_count = code_pos.decode<std::uint32_t>();
                    auto member_count = code_pos.decode<std::uint32_t>();
                    auto all_variable_names = code_pos.decode<bool>();
                    auto variable_name_count = code_pos.decode<std::uint32_t>();
                    std::vector<IdentifierIndex> variable_names;
                    variable_names.reserve(variable_name_count);
                    for(auto i = 0u; i < variable_name_count; ++i) {
                        variable_names.push_back(code_pos.decode<IdentifierIndex>());
                    }
                    std::vector<OwcaValue> base_classes;
                    base_classes.reserve(base_class_count);
                    std::vector<OwcaValue> members;
                    members.reserve(member_count);
                    for(auto i = 0u; i < base_class_count; ++i) {
                        auto d = code_pos.decode<VariableIndex>();
                        base_classes.push_back(identifier_ptrs[d]);
                    }
                    for(auto i = 0u; i < member_count; ++i) {
                        auto d = code_pos.decode<VariableIndex>();
                        members.push_back(identifier_ptrs[d]);
                    }

                    auto cls = current_vm().allocate<Class>(0, line, name, full_name, code_object);

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
                            cls->initialize_add_variable(variable_names[i]);
                        }
                    }

                    cls->finalize_initializing();

                    dest = OwcaClass{ cls };
                    break; }
#define IS_TRUE(v) OPER1_GET(is_true, (v).kind())(v)
#define CMP2_RUN(oper, reverse, upd) do {                                   \
        auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()]; \
        auto left = identifier_ptrs[code_pos.decode<VariableIndex>()];    \
        auto right = identifier_ptrs[code_pos.decode<VariableIndex>()];   \
        auto jump_dest = code_pos.decode_jump();                            \
        const auto last = code_pos.decode<bool>();                          \
        auto res = (!reverse) ?                                             \
            OPER2_GET(oper, left.kind(), right.kind())(left, right) :       \
            OPER2_GET(oper, right.kind(), left.kind())(right, left);        \
        res = (upd);                                                        \
        if (res) {                                                          \
            target = last ? OwcaValue{ true } : right;                      \
        }                                                                   \
        else {                                                              \
            target = false;                                                 \
            code_pos = jump_dest;                                           \
        }                                                                   \
    } while(0)

                case ExecuteOp::ExprMove: {
                    auto dest = code_pos.decode<VariableIndex>();
                    auto src = code_pos.decode<VariableIndex>();
                    identifier_ptrs[dest] = identifier_ptrs[src];
                    break; }
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
                case ExecuteOp::ExprConstantStringInterpolated: {
                    auto target = code_pos.decode<VariableIndex>();
                    auto strings = code_pos.decode<std::string_view>();
                    auto expr_count = code_pos.decode<std::uint32_t>();
                    std::vector<std::string_view> exprs;
                    exprs.reserve(expr_count);
                    size_t size = strings.size();
                    for(auto i = 0u; i < expr_count; ++i) {
                        exprs.push_back(identifier_ptrs[code_pos.decode<VariableIndex>()].as_string_certainly().text());
                        size += exprs.back().size();
                    }
                    auto new_str = current_vm().precreate_string(size);
                    auto new_str_pt = new_str->pointer();
                    const char *strings_ptr = strings.data();
                    for(auto i = 0u; i < expr_count; ++i) {
                        auto sz = code_pos.decode<std::uint32_t>();
                        std::memcpy(new_str_pt, strings_ptr, sz);
                        new_str_pt += sz;
                        strings_ptr += sz;
                        auto str = exprs[i];
                        std::memcpy(new_str_pt, str.data(), str.size());
                        new_str_pt += str.size();
                    }
                    auto remaining = strings.data() + strings.size() - strings_ptr;
                    std::memcpy(new_str_pt, strings_ptr, remaining);
                    new_str_pt += remaining;
                    assert(new_str_pt == new_str->pointer() + new_str->size());
                    identifier_ptrs[target] = OwcaString{ new_str };
                    break; }
                case ExecuteOp::ExprIdentifierFunctionWrite: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto &func = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto src = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    func = set_identifier_function(func, src);
                    target = func;
                    break; }
                case ExecuteOp::ExprMemberRead: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto self = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto member = code_pos.decode<IdentifierIndex>();
                    target = current_vm().member(self, member);
                    break; }
                case ExecuteOp::ExprMemberWrite: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto self = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto member = code_pos.decode<IdentifierIndex>();
                    auto val_to_write = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    current_vm().member(self, member, val_to_write);
                    target = val_to_write;
                    break; }
                case ExecuteOp::ExprOper2MemberAdd:
                case ExecuteOp::ExprOper2MemberSub:
                case ExecuteOp::ExprOper2MemberMul:
                case ExecuteOp::ExprOper2MemberDiv:
                case ExecuteOp::ExprOper2MemberMod:
                case ExecuteOp::ExprOper2MemberBinOr:
                case ExecuteOp::ExprOper2MemberBinAnd:
                case ExecuteOp::ExprOper2MemberBinXor:
                case ExecuteOp::ExprOper2MemberBinLShift:
                case ExecuteOp::ExprOper2MemberBinRShift: {
                    auto oper_index = static_cast<std::uint8_t>(opcode) - static_cast<std::uint8_t>(ExecuteOp::ExprOper2MemberFirst);
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto self = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto member = code_pos.decode<IdentifierIndex>();
                    auto right = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto left = current_vm().member(self, member);
                    auto v = OPER2_MATH_GET(oper_index, left.kind(), right.kind())(left, right);
                    current_vm().member(self, member, v);
                    target = v;
                    break; }
                case ExecuteOp::ExprOper1BinNeg: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto self = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    target = -(std::int64_t)self.as_float();
                    break; }
                case ExecuteOp::ExprOper1LogNot: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto self = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    target = !IS_TRUE(self);
                    break; }
                case ExecuteOp::ExprOper1Negate: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto self = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    target = -self.as_float();
                    break; }
                case ExecuteOp::ExprRetTrueAndJumpIfTrue: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto condition = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto jump_dest = code_pos.decode_jump();
                    if (IS_TRUE(condition)) {
                        target = true;
                        code_pos = jump_dest;
                    }
                    break; }
                case ExecuteOp::ExprRetFalseAndJumpIfFalse: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto condition = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto jump_dest = code_pos.decode_jump();
                    if (!IS_TRUE(condition)) {
                        target = false;
                        code_pos = jump_dest;
                    }
                    break; }
                case ExecuteOp::ExprToString: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto self = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    if (self.kind() == OwcaValueKind::String) {
                        target = self;
                    }
                    target = current_vm().create_string_from_view(self.to_string());
                    break; }
                case ExecuteOp::ExprToIterator: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto self = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    if (self.kind() != OwcaValueKind::Iterator) {
                        auto func = current_vm().try_member(self, current_vm().get_ii_iter());
                        if (!func) {
                            throw_not_iterable(self.type());
                        }
                        self = *func;
                        auto mv = stacktrace_current->runtime_function->max_values;
                        current_unused_locals_ptr[0] = self;
                        target = execute_call_from_values(1);
                    }
                    else {
                        target = self;
                    }
                    break; }
                case ExecuteOp::ExprOper2Add:
                case ExecuteOp::ExprOper2Sub:
                case ExecuteOp::ExprOper2Mul:
                case ExecuteOp::ExprOper2Div:
                case ExecuteOp::ExprOper2Mod:
                case ExecuteOp::ExprOper2BinOr:
                case ExecuteOp::ExprOper2BinAnd:
                case ExecuteOp::ExprOper2BinXor:
                case ExecuteOp::ExprOper2BinLShift:
                case ExecuteOp::ExprOper2BinRShift: {
                    auto target_index = code_pos.decode<VariableIndex>();
                    auto left_index = code_pos.decode<VariableIndex>();
                    auto right_index = code_pos.decode<VariableIndex>();
                    auto &target = identifier_ptrs[target_index];
                    auto left = identifier_ptrs[left_index];
                    auto right = identifier_ptrs[right_index];
                    auto oper_index = static_cast<std::uint8_t>(opcode) - static_cast<std::uint8_t>(ExecuteOp::ExprOper2Add);
                    target = OPER2_MATH_GET(oper_index, left.kind(), right.kind())(left, right);
                    break; }
                case ExecuteOp::ExprOper2MakeRange: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto first_index = code_pos.decode<VariableIndex>();
                    auto second_index = code_pos.decode<VariableIndex>();
                    auto third_index = code_pos.decode<VariableIndex>();

                    Number first, second, third;
                    if (third_index) {
                        third = identifier_ptrs[third_index].as_float();
                    }
                    else {
                        third = 1;
                    }
                    if (second_index) {
                        second = identifier_ptrs[second_index].as_float();
                    }
                    else {
                        second = std::numeric_limits<Number>::max();
                    }
                    if (first_index) {
                        first = identifier_ptrs[first_index].as_float();
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
                    target = OwcaRange{ ret };
                    break; }
                case ExecuteOp::ExprOper2IndexAdd:
                case ExecuteOp::ExprOper2IndexSub:
                case ExecuteOp::ExprOper2IndexMul:
                case ExecuteOp::ExprOper2IndexDiv:
                case ExecuteOp::ExprOper2IndexMod:
                case ExecuteOp::ExprOper2IndexBinOr:
                case ExecuteOp::ExprOper2IndexBinAnd:
                case ExecuteOp::ExprOper2IndexBinXor:
                case ExecuteOp::ExprOper2IndexBinLShift:
                case ExecuteOp::ExprOper2IndexBinRShift: {
                    auto oper_index = static_cast<std::uint8_t>(opcode) - static_cast<std::uint8_t>(ExecuteOp::ExprOper2IndexFirst);
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto self = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto key = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto right = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto left = index_read(self, key);
                    auto v = OPER2_MATH_GET(oper_index, left.kind(), right.kind())(left, right);
                    index_write(self, key, v);
                    target = v;
                    break; }

                case ExecuteOp::ExprOper2IndexRead: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto self = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto key = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    target = index_read(self, key);
                    break; }
                case ExecuteOp::ExprOper2IndexWrite: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto self = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto key = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto value_to_write = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    target = index_write(self, key, value_to_write);
                    break; }
                case ExecuteOp::ExprOperXCall: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto size = code_pos.decode<std::uint32_t>();
                    for(auto i = 0u; i < size; ++i) {
                        auto v = identifier_ptrs[code_pos.decode<VariableIndex>()];
                        current_unused_locals_ptr[i] = v;
                    }
                    target = OPER1_GET(call, current_unused_locals_ptr[0].kind())(size);
                    break; }
                case ExecuteOp::ExprOperXCallWithMember: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto member = code_pos.decode<IdentifierIndex>();
                    auto size = code_pos.decode<std::uint32_t>();
                    auto v = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    current_unused_locals_ptr[0] = current_vm().member(v, member);
                    for(auto i = 1u; i < size; ++i) {
                        auto v = identifier_ptrs[code_pos.decode<VariableIndex>()];
                        current_unused_locals_ptr[i] = v;
                    }
                    target = OPER1_GET(call, current_unused_locals_ptr[0].kind())(size);
                    break; }
                case ExecuteOp::ExprOperXCreateArray: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto size = code_pos.decode<std::uint32_t>();
                    auto arguments = std::deque<OwcaValue>{};
                    for(auto i = 0u; i < size; ++i) {
                        auto v = identifier_ptrs[code_pos.decode<VariableIndex>()];
                        arguments.push_back(v);
                    }
                    target = current_vm().create_array(std::move(arguments));
                    break; }
                case ExecuteOp::ExprOperXCreateTuple: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto size = code_pos.decode<std::uint32_t>();
                    auto arguments = std::vector<OwcaValue>{};
                    for(auto i = 0u; i < size; ++i) {
                        auto v = identifier_ptrs[code_pos.decode<VariableIndex>()];
                        arguments.push_back(v);
                    }
                    target = current_vm().create_tuple(std::move(arguments));
                    break; }
                case ExecuteOp::ExprOperXCreateSet: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto size = code_pos.decode<std::uint32_t>();
                    auto arguments = std::vector<OwcaValue>{};
                    for(auto i = 0u; i < size; ++i) {
                        auto v = identifier_ptrs[code_pos.decode<VariableIndex>()];
                        arguments.push_back(v);
                    }
                    target = current_vm().create_set(arguments);
                    break; }
                case ExecuteOp::ExprOperXCreateMap: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto size = code_pos.decode<std::uint32_t>();
                    auto arguments = std::vector<OwcaValue>{};
                    for(auto i = 0u; i < size; ++i) {
                        auto v = identifier_ptrs[code_pos.decode<VariableIndex>()];
                        arguments.push_back(v);
                    }
                    target = current_vm().create_map(arguments);
                    break; }
                case ExecuteOp::ExprIteratorNextAndJumpIfCompleted: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto iter = identifier_ptrs[code_pos.decode<VariableIndex>()].as_iterator_certainly();
                    auto end_position = code_pos.decode_jump();

                    if (iter.completed()) [[unlikely]] {
                        code_pos = end_position;
                        break;
                    }
                    auto val = continue_iterator(iter);
                    if (!val) [[unlikely]] {
                        code_pos = end_position;
                        break;
                    }
                    target = *val;
                    break; }
                case ExecuteOp::Function: {
                    auto &target = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    target = create_function(code_pos, identifier_ptrs);
                    break; }
                case ExecuteOp::If: {
                    auto condition = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto val = IS_TRUE(condition);
                    auto else_position = code_pos.decode_jump();
                    if (!val) {
                        code_pos = else_position;
                    }
                    break; }
                case ExecuteOp::IfAlmostAlwaysFalse: {
                    auto condition = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto val = IS_TRUE(condition);
                    auto else_position = code_pos.decode_jump();
                    if (!val){
                        code_pos = else_position;
                    }
                    break; }
                case ExecuteOp::IfAlmostAlwaysTrue: {
                    auto condition = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto val = IS_TRUE(condition);
                    auto else_position = code_pos.decode_jump();
                    if (!val){
                        code_pos = else_position;
                    }
                    break; }
                case ExecuteOp::ReturnCloseIterator: {
                    //complete_all(temporary_ptr);
                    return { OwcaCompleted{}, code_pos };
                    }
                case ExecuteOp::Return: {
                    //complete_all(temporary_ptr);
                    return { OwcaEmpty{}, code_pos };
                    }
                case ExecuteOp::ReturnValue: {
                    auto val = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    //complete_all(temporary_ptr);
                    return { val, code_pos };
                    }
                case ExecuteOp::Yield: {
                    auto val = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    return { val, code_pos };
                    }
                case ExecuteOp::TryCatchCompleted:
                    current_try_with_block_when_thrown = nullptr;
                    exception_being_thrown.reset();
                    exception_being_handled.reset();
                    [[fallthrough]];
                case ExecuteOp::Jump: {
                    auto dest = code_pos.decode_jump();
                    code_pos = dest;
                    break; }
                case ExecuteOp::Throw: {
                    auto val = identifier_ptrs[code_pos.decode<VariableIndex>()];
                    auto exc = val.as_exception_maybe();
                    if (!exc) {
                        throw_wrong_type(val.type(), "Exception");
                    }
                    throw *exc;
                    }
                case ExecuteOp::TryCatchType: {
                    auto block_pos = code_pos.decode_jump();
                    auto sz = code_pos.decode<std::uint32_t>();
                    auto dest_var = code_pos.decode<VariableIndex>();
                    if (sz == 0) {
                        if (dest_var) {
                            identifier_ptrs[dest_var] = *exception_being_thrown;
                        }
                        code_pos = block_pos;
                        break;
                    }
                    std::vector<OwcaClass> types;
                    types.reserve(sz);

                    for(auto i = 0u; i < sz; ++i) {
                        auto type = identifier_ptrs[code_pos.decode<VariableIndex>()];
                        types.push_back(type.as_class());
                    }

                    auto j = 0u;
                    for(; j < sz; ++j) {
                        auto &type = types[j];
                        if (exception_being_thrown->type().has_base_class(type)) {
                            if (dest_var) {
                                identifier_ptrs[dest_var] = *exception_being_thrown;
                            }
                            code_pos = block_pos;
                            break;
                        }
                    }
                    break; }
                case ExecuteOp::TryCatchTypeCompleted: {
                    auto next = current_try_with_block_when_thrown->next;
                    if (next == 0xffffffff) goto rethrow;
                    current_try_with_block_when_thrown = &sf->try_with_blocks[next];
                    code_pos = current_try_with_block_when_thrown->jump;
                    }                    

                // case ExecuteOp::WithInit: {
                //     PUSH_STATE(WithState{});
                //     auto &state = STATE(WithState);
                //     auto &obj = PEEK_VALUE(1);
                //     state.context = obj;
                //     obj = current_vm().member(obj, "$enter");
                //     obj = execute_call_from_values(temporary_ptr, 1);
                //     state.entered = true;
                //     auto index = code_pos.decode<std::uint32_t>();
                //     if (index != std::numeric_limits<std::uint32_t>::max()) {
                //         LOCAL_VAR(index) = obj;
                //     }
                //     POP_VALUES(1);
                //     break; }
                // case ExecuteOp::WithCompleted: {
                //     auto &state = STATE(WithState);
                //     complete(state, temporary_ptr);
                //     POP_STATE();
                //     break; }
                default:
                    assert(false);
                }
                assert(stacktrace_current == stacktrace_current_copy);
                stacktrace_current_copy->code_position = code_pos;
                //std::cout << "setting code position (" << (void*)&frame.code_position << ") to " << frame.code_position << std::endl;
            }
        }
        catch(OwcaException oe) {
            process_thrown_exception(&code_pos, oe);
            goto restart;
        }
    rethrow:
        throw *exception_being_thrown;
    }
    // void Executor::complete_all(TemporariesPtr temporary_ptr) {
    //     auto sc = stacktrace_current;
    //     while(HAS_STATE()) {
    //         if (auto s = TRY_STATE(WithState)) {
    //             complete(*s, temporary_ptr);
    //         }
    //         POP_STATE();
    //     }
    // }
    // void Executor::complete(WithState state, TemporariesPtr temporary_ptr) {
    //     if (state.entered) {
    //         state.entered = false;
    //         auto mbm = current_vm().member(state.context, "$exit");
    //         PUSH_VALUE(mbm);
    //         execute_call_from_values(temporary_ptr, 1);
    //         POP_VALUES(1);
    //     }
    // }
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

    OwcaValue Executor::run_script_code(RuntimeFunctionScriptFunction *function, LocalsPtr locals_ptr, unsigned int arg_count, bool clear_locals) {
        const auto max_values = function->max_values;

        assert(locals_ptr.local_values_ptr + max_values <= values_vector.data() + values_vector.size());
        if (clear_locals) [[likely]] {
            for(auto i = arg_count + 1; i < max_values; ++i) {
                locals_ptr[i] = OwcaEmpty{};
            }
        }

        //assert(function->copy_from_parents.size() == function->values_from_parents.size());

        auto est = StackTraceState{ *this, function, function->entry_point };
        auto [ retval, new_code_pos ] = run_opcodes(locals_ptr, function->entry_point);
        return retval;
    }
    Generator Executor::run_script_generator(Iterator *iter_object, RuntimeFunctionScriptGenerator *function, std::vector<OwcaValue> values_vec, CodePosition code_pos)
    {
        const auto locals_ptr = LocalsPtr{ values_vec.data() };
        while(true) {
            OwcaValue val;
            {
                auto est = StackTraceState{ *this, function, code_pos };
                auto tpk = Executor::TopPtrsKeeper{ *this, function->max_values };
                auto sc = stacktrace_current;
                auto [ retval, new_code_pos ] = run_opcodes(locals_ptr, code_pos);
                assert(sc == stacktrace_current);
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

	OwcaValue Executor::allocate_user_class_from_values(unsigned int arg_count) {
		OwcaValue obj;

        assert(arg_count > 0);

        auto cls = current_unused_locals_ptr[0].as_class_certainly().internal_value();

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

		auto ii_ptr = cls->values.find(current_vm().get_ii_init());
		if (!ii_ptr) {
			if (arg_count > 1) {
				throw_cant_call(std::format("type {} has no $init function defined - expected constructor's call with no parameters, instead got {} parameters", cls->full_name, arg_count - 1));
			}
            return obj;
		}
        if (auto state = std::get_if<RuntimeFunctions*>(ii_ptr)) [[likely]] {
            current_unused_locals_ptr[0] = obj;
            auto retval = execute_function_call_from_values(*state, arg_count);
            if (!cls->reload_self) [[likely]] {
                retval = obj;
            }
            return retval;
        }
        throw_cant_call(std::format("type {} has $init variable, not a function", std::get<Class*>(*ii_ptr)->full_name));
	}
    OwcaValue Executor::execute_call_from_values(unsigned int argument_count) {
        auto func = current_unused_locals_ptr[0];
        if (func.kind() == OwcaValueKind::Functions) [[likely]] {
            return execute_function_call_from_values(argument_count);
        }
        else {
            if (func.kind() == OwcaValueKind::Class) [[likely]] {
                return allocate_user_class_from_values(argument_count);
            }
            throw_cant_call(std::format("can't call {} with {} parameters", func.type(), argument_count - 1));
        }
    }
    OwcaValue Executor::execute_function_call_from_values(unsigned int argument_count) {
        auto func = current_unused_locals_ptr[0];
        auto f = func.as_functions_certainly();
        auto runtime_functions = f.internal_value();
        current_unused_locals_ptr[0] = f.self();

        return execute_function_call_from_values(runtime_functions, argument_count);
    }

    OwcaValue Executor::execute_function_call_from_values(RuntimeFunctions* runtime_functions, unsigned int arg_count) {
        assert(arg_count > 0);
        auto runtime_function = runtime_functions->functions[arg_count - 1];
        if (!runtime_function) [[unlikely]] {
            auto tmp = std::string{ "function " };
            tmp += runtime_functions->name;
            throw_not_callable_wrong_number_of_params(std::move(tmp), arg_count - 1);
        }
        return runtime_function->call(*this);
    }


	OwcaNamespace Executor::execute_code_block(const OwcaCodeBuffer &oc_buffer, std::shared_ptr<NativeCodeProvider> native_code_provider)
	{
        auto it = namespaces.find(oc_buffer.filename());
        if (it != namespaces.end()) {
            return OwcaNamespace{ it->second };
        }

#ifdef OWCA_SCRIPT_EXEC_LOG
        std::cout << "Executing code block from file " << oc_buffer.filename() << std::endl;
#endif
        auto oc = Internal::OwcaCode{ current_vm(), oc_buffer, std::move(native_code_provider) };
        auto ns = current_vm().create_namespace(std::move(oc));
        namespaces.insert({ ns.internal_value()->code.filename(), ns});
        if (namespaces.size() > 1) {
            auto ns_it = namespaces.at(current_vm().builtin_filename);
            for(auto it : ns_it.internal_value()->code.identifier_to_global_index()) {
                auto val = ns_it.internal_value()->globals[it.second];
                ns.try_member(it.first, val);
            }
        }

        auto globals_ptr = GlobalsPtr{ ns.internal_value()->globals.data() };
        auto tpk = TopPtrsKeeper{ *this, ns.internal_value()->code.max_values_count() };
        auto locals_ptr = tpk.current_unused_locals_ptr;

        auto mcb_index = current_vm().get_identifier_index("main-code-block");
        auto function = current_vm().allocate<RuntimeFunctionScriptFunction>(0, ns.internal_value()->code, ns, globals_ptr, mcb_index, std::string_view("main-code-block"), std::string_view("main-code-block"), false, ns.internal_value()->code.code_position());
        auto est = StackTraceState{ *this, function, function->entry_point };
        run_opcodes(locals_ptr, ns.internal_value()->code.code_position());
        return ns;
    }

    OwcaValue Executor::allocate_user_class(Class *cls, std::span<OwcaValue> arguments) {
        auto locals_ptr = current_unused_locals_ptr;
        locals_ptr[0] = OwcaClass{ cls };

        for(auto i = 0u; i < arguments.size(); ++i) {
            locals_ptr[i + 1] = arguments[i];
        }
        return allocate_user_class_from_values((unsigned int)arguments.size() + 1);
    }
    OwcaValue Executor::execute_call(OwcaValue func, std::span<OwcaValue> arguments) {
        auto locals_ptr = current_unused_locals_ptr;
        locals_ptr[0] = func;
        for(size_t i = 0; i < arguments.size(); ++i) {
            locals_ptr[i + 1] = arguments[i];
        }
        return execute_call_from_values((unsigned int)arguments.size() + 1);
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

    void gc_mark_value(GenerationGC ggc, const Executor &e) {
        for(auto it : e.namespaces) {
            gc_mark_value(ggc, it.second);
        }
        for(auto v = e.values_vector.data(); v < e.current_unused_locals_ptr.local_values_ptr; ++v) {
            gc_mark_value(ggc, *v);
        }
        for(auto sc = e.stacktrace_vector.data() + 1; sc <= e.stacktrace_current; ++sc) {
            gc_mark_value(ggc, sc->runtime_function);
        }
        if (e.exception_being_thrown) {
            gc_mark_value(ggc, *e.exception_being_thrown);
        }
        if (e.exception_being_handled) {
            gc_mark_value(ggc, *e.exception_being_handled);
        }
    }
}
