#ifndef RC_OWCA_SCRIPT_OWCA_VALUE_H
#define RC_OWCA_SCRIPT_OWCA_VALUE_H

#include "stdafx.h"
#include "owca_string.h"
#include "owca_functions.h"
#include "owca_range.h"
#include "owca_map.h"
#include "owca_class.h"
#include "owca_object.h"
#include "owca_tuple.h"
#include "owca_array.h"
#include "owca_set.h"
#include "owca_exception.h"
#include "owca_iterator.h"
#include "owca_namespace.h"
#include <cstddef>
#include <string_view>

namespace OwcaScript {
	class OwcaVM;

	enum class OwcaValueKind : std::uint8_t {
		Empty,
		Completed,
		Range,
		Bool,
		Float,
		String,
		Functions,
		Map,
		Set,
		Class,
		Object,
		Tuple,
		Array,
		Iterator,
		Exception,
		Namespace,
		_Count,
	};

	namespace Internal {
		class VM;

		template <bool LittleEndian> struct ValuePtrs;
		template <> struct ValuePtrs<true> {
			struct PtrsValue {
				std::uintptr_t ptr1;
				void *ptr2;
			};
			static_assert(sizeof(PtrsValue) > sizeof(Number));
			struct NumberValue {
				OwcaValueKind kind;
				Number value;
			};
			static_assert(sizeof(PtrsValue) == sizeof(NumberValue));
			static constexpr const size_t kind_offset = offsetof(NumberValue, kind);
		};
		template <> struct ValuePtrs<false> {
			struct PtrsValue {
				void *ptr2;
				std::uintptr_t ptr1;
			};

			static_assert(sizeof(PtrsValue) > sizeof(Number));
			struct NumberValue {
				Number value;
				std::byte padding[sizeof(PtrsValue) - sizeof(Number) - 1];
				OwcaValueKind kind;
			};

			static_assert(sizeof(PtrsValue) == sizeof(NumberValue));
			static constexpr const size_t kind_offset = offsetof(NumberValue, kind);
		};
	}

	class OwcaValue {
		friend class Internal::VM;

		using PtrsValue = Internal::ValuePtrs<std::endian::native == std::endian::little>::PtrsValue;
		using NumberValue = Internal::ValuePtrs<std::endian::native == std::endian::little>::NumberValue;
		static constexpr const size_t kind_offset = Internal::ValuePtrs<std::endian::native == std::endian::little>::kind_offset;
		union {
			PtrsValue ptrs;
			NumberValue number;
		} value_encoded_;

		OwcaValue(OwcaValueKind kind, void *ptr1, void *ptr2) {
			std::uintptr_t k = (std::uintptr_t)ptr1;
			assert((k & 15) == 0);
			assert((int)kind < 16 && (int)kind >= 0);
			k |= (std::uintptr_t)kind;
			value_encoded_.ptrs.ptr1 = k;
			value_encoded_.ptrs.ptr2 = ptr2;
			assert(this->kind() == kind);
			assert(internal_ptr1() == ptr1);
			assert(internal_ptr2() == ptr2);
		}
		OwcaValue(OwcaValueKind kind, Number num) {
			assert((int)kind < 16 && (int)kind >= 0);
			value_encoded_.number.value = num;
			value_encoded_.number.kind = kind;
			assert(this->kind() == kind);
		}


		void *internal_ptr1() const {
			PtrsValue tmp;
			std::memcpy(&tmp, &value_encoded_, sizeof(PtrsValue));
			return (void*)(tmp.ptr1 & ~(std::uintptr_t)15);
		}
		void *internal_ptr2() const {
			PtrsValue tmp;
			std::memcpy(&tmp, &value_encoded_, sizeof(PtrsValue));
			return tmp.ptr2;
		}

		[[noreturn]] void throw_wrong_type(const OwcaVM &vm, std::string_view expected) const;
	public:
		OwcaValue() : OwcaValue(OwcaValueKind::Empty, nullptr, nullptr) {}
		template <typename T> OwcaValue(T value) requires(std::is_same_v<std::remove_cvref_t<T>, bool>) : OwcaValue(OwcaValueKind::Bool, (Number)(value ? 1 : 0)) {}
		template <typename T> OwcaValue(T value) requires(!std::is_same_v<std::remove_cvref_t<T>, bool> && std::is_arithmetic_v<T>) : OwcaValue(OwcaValueKind::Float, (Number)value) {}
		OwcaValue(OwcaEmpty value): OwcaValue(OwcaValueKind::Empty, nullptr, nullptr) {}
		OwcaValue(OwcaCompleted value): OwcaValue(OwcaValueKind::Completed, nullptr, nullptr) {}
		OwcaValue(OwcaRange value): OwcaValue(OwcaValueKind::Range, value.internal_object(), nullptr) {}
		OwcaValue(OwcaString value): OwcaValue(OwcaValueKind::String, value.internal_value(), nullptr) {}
		OwcaValue(OwcaFunctions value): OwcaValue(OwcaValueKind::Functions, value.internal_value(), value.internal_self_object()) {}
		OwcaValue(OwcaMap value): OwcaValue(OwcaValueKind::Map, value.internal_value(), nullptr) {}
		OwcaValue(OwcaClass value): OwcaValue(OwcaValueKind::Class, value.internal_value(), nullptr) {}
		OwcaValue(OwcaObject value): OwcaValue(OwcaValueKind::Object, value.internal_value(), nullptr) {}
		OwcaValue(OwcaTuple value): OwcaValue(OwcaValueKind::Tuple, value.internal_value(), nullptr) {}
		OwcaValue(OwcaArray value): OwcaValue(OwcaValueKind::Array, value.internal_value(), nullptr) {}
		OwcaValue(OwcaSet value): OwcaValue(OwcaValueKind::Set, value.internal_value(), nullptr) {}
		OwcaValue(OwcaException value): OwcaValue(OwcaValueKind::Exception, value.internal_owner(), value.internal_value()) {}	
		OwcaValue(OwcaIterator value): OwcaValue(OwcaValueKind::Iterator, value.internal_value(), nullptr) {}
		OwcaValue(OwcaNamespace value): OwcaValue(OwcaValueKind::Namespace, value.internal_value(), nullptr) {}

		OwcaValueKind kind() const {
			static_assert((int)OwcaValueKind::_Count <= 16, "OwcaValueKind must fit in 4 bits");
			auto kind = ((std::uint8_t*)this)[kind_offset] & 15;
#ifdef DEBUG			
			NumberValue tmp;
			std::memcpy(&tmp, &value_encoded_, sizeof(NumberValue));
			auto k = (std::uint8_t)tmp.kind & 15;
			assert(k == kind);
#endif
			return (OwcaValueKind)kind;

		}
		long long int as_int(const OwcaVM&) const;
		bool is_true() const;

		OwcaEmpty as_nul(const OwcaVM& vm) const
		{
			if (auto v = as_nul_maybe()) return *v;
			throw_wrong_type(vm, "Nul");
		}
		OwcaCompleted as_completed(const OwcaVM& vm) const
		{
			if (auto v = as_completed_maybe()) return *v;
			throw_wrong_type(vm, "Completed");
		}
		OwcaRange as_range(const OwcaVM& vm) const
		{
			if (auto v = as_range_maybe()) return *v;
			throw_wrong_type(vm, "Range");
		}
		bool as_bool(const OwcaVM& vm) const
		{
			if (auto v = as_bool_maybe()) return *v;
			throw_wrong_type(vm, "Bool");
		}
		Number as_float(const OwcaVM& vm) const
		{
			if (auto v = as_float_maybe()) return *v;
			throw_wrong_type(vm, "Float");
		}
		OwcaString as_string(const OwcaVM& vm) const
		{
			if (auto v = as_string_maybe()) return *v;
			throw_wrong_type(vm, "String");
		}
		OwcaFunctions as_functions(const OwcaVM& vm) const
		{
			if (auto v = as_functions_maybe()) return *v;
			throw_wrong_type(vm, "Function");
		}
		OwcaMap as_map(const OwcaVM& vm) const
		{
			if (auto v = as_map_maybe()) return *v;
			throw_wrong_type(vm, "Map");
		}
		OwcaClass as_class(const OwcaVM& vm) const
		{
			if (auto v = as_class_maybe()) return *v;
			throw_wrong_type(vm, "Class");
		}
		OwcaObject as_object(const OwcaVM& vm) const
		{
			if (auto v = as_object_maybe()) return *v;
			throw_wrong_type(vm, "Object");
		}
		OwcaArray as_array(const OwcaVM& vm) const
		{
			if (auto v = as_array_maybe()) return *v;
			throw_wrong_type(vm, "Array");
		}
		OwcaTuple as_tuple(const OwcaVM& vm) const
		{
			if (auto v = as_tuple_maybe()) return *v;
			throw_wrong_type(vm, "Tuple");
		}
		OwcaSet as_set(const OwcaVM& vm) const
		{
			if (auto v = as_set_maybe()) return *v;
			throw_wrong_type(vm, "Set");
		}
		OwcaNamespace as_namespace(const OwcaVM& vm) const
		{
			if (auto v = as_namespace_maybe()) return *v;
			throw_wrong_type(vm, "Namespace");
		}
		OwcaException as_exception(const OwcaVM& vm) const
		{
			if (auto v = as_exception_maybe()) return *v;
			throw_wrong_type(vm, "Exception");
		}
		OwcaIterator as_iterator(const OwcaVM& vm) const
		{
			if (auto v = as_iterator_maybe()) return *v;
			throw_wrong_type(vm, "Iterator");
		}

		OwcaEmpty as_nul_certainly() const
		{
			return *as_nul_maybe();
		}
		OwcaCompleted as_completed_certainly() const
		{
			return *as_completed_maybe();
		}
		OwcaRange as_range_certainly() const
		{
			return *as_range_maybe();
		}
		bool as_bool_certainly() const
		{
			return *as_bool_maybe();
		}
		Number as_float_certainly() const
		{
			return *as_float_maybe();
		}
		OwcaString as_string_certainly() const
		{
			return *as_string_maybe();
		}
		OwcaFunctions as_functions_certainly() const
		{
			return *as_functions_maybe();
		}
		OwcaMap as_map_certainly() const
		{
			return *as_map_maybe();
		}
		OwcaClass as_class_certainly() const
		{
			return *as_class_maybe();
		}
		OwcaObject as_object_certainly() const
		{
			return *as_object_maybe();
		}
		OwcaArray as_array_certainly() const
		{
			return *as_array_maybe();
		}
		OwcaTuple as_tuple_certainly() const
		{
			return *as_tuple_maybe();
		}
		OwcaSet as_set_certainly() const
		{
			return *as_set_maybe();
		}
		OwcaNamespace as_namespace_certainly() const
		{
			return *as_namespace_maybe();
		}
		OwcaException as_exception_certainly() const
		{
			return *as_exception_maybe();
		}
		OwcaIterator as_iterator_certainly() const
		{
			return *as_iterator_maybe();
		}

		std::optional<OwcaEmpty> as_nul_maybe() const
		{
			if (kind() == OwcaValueKind::Empty) return OwcaEmpty{};
			return std::nullopt;
		}
		std::optional<OwcaCompleted> as_completed_maybe() const
		{
			if (kind() == OwcaValueKind::Completed) return OwcaCompleted{};
			return std::nullopt;
		}
		std::optional<OwcaRange> as_range_maybe() const
		{
			if (kind() == OwcaValueKind::Range) return OwcaRange{ (Internal::Range*)internal_ptr1() };
			return std::nullopt;
		}
		std::optional<bool> as_bool_maybe() const
		{
			if (kind() == OwcaValueKind::Bool) {
				NumberValue tmp;
				std::memcpy(&tmp, &value_encoded_, sizeof(NumberValue));
				return tmp.value != 0;			
			}
			return std::nullopt;
		}
		std::optional<Number> as_float_maybe() const
		{
			if (kind() == OwcaValueKind::Float) {
				NumberValue tmp;
				std::memcpy(&tmp, &value_encoded_, sizeof(NumberValue));
				return tmp.value;			
			}
			return std::nullopt;
		}
		std::optional<OwcaString> as_string_maybe() const
		{
			if (kind() == OwcaValueKind::String) return OwcaString{ (Internal::String*)internal_ptr1() };
			return std::nullopt;
		}
		std::optional<OwcaFunctions> as_functions_maybe() const
		{
			if (kind() == OwcaValueKind::Functions) return OwcaFunctions{ (Internal::RuntimeFunctions*)internal_ptr1(), (Internal::AllocationBase*)internal_ptr2() };
			return std::nullopt;
		}
		std::optional<OwcaMap> as_map_maybe() const
		{
			if (kind() == OwcaValueKind::Map) return OwcaMap{ (Internal::DictionaryShared*)internal_ptr1() };
			return std::nullopt;
		}
		std::optional<OwcaClass> as_class_maybe() const
		{
			if (kind() == OwcaValueKind::Class) return OwcaClass{ (Internal::Class*)internal_ptr1() };
			return std::nullopt;
		}
		std::optional<OwcaObject> as_object_maybe() const
		{
			if (kind() == OwcaValueKind::Object) return OwcaObject{ (Internal::Object*)internal_ptr1() };
			return std::nullopt;
		}
		std::optional<OwcaArray> as_array_maybe() const
		{
			if (kind() == OwcaValueKind::Array) return OwcaArray{ (Internal::Array*)internal_ptr1() };
			return std::nullopt;
		}
		std::optional<OwcaTuple> as_tuple_maybe() const
		{
			if (kind() == OwcaValueKind::Tuple) return OwcaTuple{ (Internal::Tuple*)internal_ptr1() };
			return std::nullopt;
		}
		std::optional<OwcaSet> as_set_maybe() const
		{
			if (kind() == OwcaValueKind::Set) return OwcaSet{ (Internal::SetShared*)internal_ptr1() };
			return std::nullopt;
		}
		std::optional<OwcaNamespace> as_namespace_maybe() const
		{
			if (kind() == OwcaValueKind::Namespace) return OwcaNamespace{ (Internal::Namespace*)internal_ptr1() };
			return std::nullopt;
		}
		std::optional<OwcaException> as_exception_maybe() const
		{
			if (kind() == OwcaValueKind::Exception) return OwcaException{ (Internal::Object*)internal_ptr1(), (Internal::Exception*)internal_ptr2() };
			return std::nullopt;
		}
		std::optional<OwcaIterator> as_iterator_maybe() const
		{
			if (kind() == OwcaValueKind::Iterator) return OwcaIterator{ (Internal::Iterator*)internal_ptr1() };
			return std::nullopt;
		}

		std::string_view type() const;
		std::string to_string() const;

		OwcaValue member(const OwcaVM&, const std::string& key) const;
		void member(const OwcaVM&, const std::string& key, OwcaValue val);
		OwcaValue call(const OwcaVM&, std::span<OwcaValue> args) const;

		OwcaValue member(const std::string& key) const;
		void member(const std::string& key, OwcaValue val);
		OwcaValue call_with_args(std::span<OwcaValue> args) const;
		template <typename ... ARGS> OwcaValue call(ARGS&&... args) const {
			std::array<OwcaValue, sizeof...(ARGS)> arr{ OwcaValue(std::forward<ARGS>(args))... };
			return call_with_args(arr);
		}

		template <typename ... F> auto visit(F &&...fns) const {
			struct overloaded : F... {
				using F::operator()...;
			};
			auto tmp = overloaded{std::forward<F>(fns)...};
			switch(kind()) {
			case OwcaValueKind::Empty: return tmp(OwcaEmpty{});
			case OwcaValueKind::Completed: return tmp(OwcaCompleted{});
			case OwcaValueKind::Range: return tmp(as_range_certainly());
			case OwcaValueKind::Bool: return tmp(as_bool_certainly());
			case OwcaValueKind::Float: return tmp(as_float_certainly());
			case OwcaValueKind::String: return tmp(as_string_certainly());
			case OwcaValueKind::Functions: return tmp(as_functions_certainly());
			case OwcaValueKind::Map: return tmp(as_map_certainly());
			case OwcaValueKind::Class: return tmp(as_class_certainly());
			case OwcaValueKind::Object: return tmp(as_object_certainly());
			case OwcaValueKind::Tuple: return tmp(as_tuple_certainly());
			case OwcaValueKind::Array: return tmp(as_array_certainly());
			case OwcaValueKind::Set: return tmp(as_set_certainly());
			case OwcaValueKind::Exception: return tmp(as_exception_certainly());
			case OwcaValueKind::Iterator: return tmp(as_iterator_certainly());
			case OwcaValueKind::Namespace: return tmp(as_namespace_certainly());
			case OwcaValueKind::_Count: break;
			}
			assert(false);
			throw std::logic_error("invalid OwcaValueKind");
		}
	};

	void gc_mark_value(const OwcaVM&, GenerationGC gc, OwcaValue);
	inline void gc_mark_value(const OwcaVM&, GenerationGC gc, OwcaEmpty) {}
	inline void gc_mark_value(const OwcaVM&, GenerationGC gc, OwcaCompleted) {}		
	template <typename T> inline void gc_mark_value(const OwcaVM&, GenerationGC gc, T) requires(std::is_arithmetic_v<T>) {}		

	namespace Internal {
		class VM;

		void throw_cant_convert_to_number(const OwcaVM&, size_t I, OwcaValue v);
		template <std::integral T>
		static T convert_impl2(const OwcaVM& vm, size_t I, T *, OwcaValue v) {
			if (v.kind() != OwcaValueKind::Float) 
				throw_cant_convert_to_number(vm, I, v);
			return (T)v.as_float_certainly();
		}
		template <std::floating_point T>
		static auto convert_impl2(const OwcaVM& vm, size_t I, T *, OwcaValue v) {
			if (v.kind() != OwcaValueKind::Float) 
				throw_cant_convert_to_number(vm, I, v);
			return (T)v.as_float_certainly();
		}
		bool convert_impl2(const OwcaVM& vm, size_t I, bool *b, OwcaValue v);
		std::string convert_impl2(const OwcaVM& vm, size_t I, std::string *b, OwcaValue v);
		std::string_view convert_impl2(const OwcaVM& vm, size_t I, std::string_view *b, OwcaValue v);
		OwcaEmpty convert_impl2(const OwcaVM& vm, size_t I, OwcaEmpty *b, OwcaValue v);
		OwcaRange convert_impl2(const OwcaVM& vm, size_t I, OwcaRange *b, OwcaValue v);
		Number convert_impl2(const OwcaVM& vm, size_t I, Number *b, OwcaValue v);
		OwcaString convert_impl2(const OwcaVM& vm, size_t I, OwcaString *b, OwcaValue v);
		OwcaFunctions convert_impl2(const OwcaVM& vm, size_t I, OwcaFunctions *b, OwcaValue v);
		OwcaMap convert_impl2(const OwcaVM& vm, size_t I, OwcaMap *b, OwcaValue v);
		OwcaClass convert_impl2(const OwcaVM& vm, size_t I, OwcaClass *b, OwcaValue v);
		OwcaObject convert_impl2(const OwcaVM& vm, size_t I, OwcaObject *b, OwcaValue v);
		OwcaIterator convert_impl2(const OwcaVM& vm, size_t I, OwcaIterator *b, OwcaValue v);
		OwcaArray convert_impl2(const OwcaVM& vm, size_t I, OwcaArray *b, OwcaValue v);
		OwcaTuple convert_impl2(const OwcaVM& vm, size_t I, OwcaTuple *b, OwcaValue v);
		OwcaSet convert_impl2(const OwcaVM& vm, size_t I, OwcaSet *b, OwcaValue v);
		OwcaException convert_impl2(const OwcaVM& vm, size_t I, OwcaException *b, OwcaValue v);
		OwcaValue convert_impl2(const OwcaVM& vm, size_t I, OwcaValue *b, OwcaValue v);

		template <typename T> struct FuncToTuple {
		};
		template <typename ... ARGS> struct FuncToTuple<OwcaValue(const OwcaVM &, ARGS...)> {
			using type = std::tuple<std::remove_cvref_t<ARGS>...>;
			static constexpr bool is_generator = false;
		};

		template <size_t I, typename ... ARGS> static auto convert_impl(const OwcaVM& vm, std::span<OwcaValue> args) {
			if constexpr(I < sizeof...(ARGS)) {
				using T = std::remove_cvref_t<std::tuple_element_t<I, std::tuple<ARGS...>>>;
				std::tuple<T> tmp = { convert_impl2(vm, I, (T*)nullptr, args[I]) };
				auto res = convert_impl<I + 1, ARGS...>(vm, args);
				auto res2 = std::tuple_cat(std::move(tmp), std::move(res));
				return res2;
			}
			else {
				return std::tuple<>{};
			}
		}
		template <typename ... ARGS> static std::tuple<const OwcaVM&, ARGS...> convert2(const OwcaVM& vm, std::span<OwcaValue> args, std::tuple<ARGS...> *) {
			assert(sizeof...(ARGS) == args.size());
			std::tuple<ARGS...> dst_args = convert_impl<0, ARGS...>(vm, args);
			return std::tuple_cat(std::tuple<const OwcaVM&>(vm), std::move(dst_args));
		}

	}

	template <typename F>
	static auto adapt(F &&f) requires (!Internal::FuncToTuple<std::remove_cvref_t<F>>::is_generator) {
		return [f = std::forward<F>(f)](const OwcaVM &vm, std::span<OwcaValue> args) -> OwcaValue {
			using T = typename Internal::FuncToTuple<std::remove_cvref_t<F>>::type;
			auto dest_args = Internal::convert2(vm, args, (T*)nullptr);
			return std::apply(f, dest_args);
		};
	}
}

namespace std {
	template <>
	struct formatter<OwcaScript::OwcaValue>
	{
		template <typename FormatContext>
		auto format(OwcaScript::OwcaValue v, FormatContext& ctx) const
		{
			return format_to(ctx.out(), "{}", v.to_string());  
		}
		template<class ParseContext>
		constexpr ParseContext::iterator parse(ParseContext& ctx)
		{
			return ctx.begin();
		}
	};
}

#endif
