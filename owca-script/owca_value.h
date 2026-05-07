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
		Class,
		Object,
		Tuple,
		Array,
		Set,
		Iterator,
		Exception,
		Namespace,
		_Count,
	};

	namespace Internal {
		class VM;
		class Class;
		

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
		};
	}

	class OwcaValue {
		friend class Internal::VM;

		using PtrsValue = Internal::ValuePtrs<std::endian::native == std::endian::little>::PtrsValue;
		using NumberValue = Internal::ValuePtrs<std::endian::native == std::endian::little>::NumberValue;
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

		OwcaValueKind kind() const;
		long long int as_int(OwcaVM ) const;
		bool is_true() const;

		OwcaEmpty as_nul(OwcaVM ) const;
		OwcaCompleted as_completed(OwcaVM ) const;
		OwcaRange as_range(OwcaVM ) const;
		bool as_bool(OwcaVM ) const;
		Number as_float(OwcaVM ) const;
		OwcaString as_string(OwcaVM ) const;
		OwcaFunctions as_functions(OwcaVM ) const;
		OwcaMap as_map(OwcaVM ) const;
		OwcaClass as_class(OwcaVM ) const;
		OwcaObject as_object(OwcaVM ) const;
		OwcaTuple as_tuple(OwcaVM ) const;
		OwcaArray as_array(OwcaVM ) const;
		OwcaSet as_set(OwcaVM ) const;
		OwcaException as_exception(OwcaVM) const;
		OwcaIterator as_iterator(OwcaVM) const;
		OwcaNamespace as_namespace(OwcaVM) const;

		OwcaEmpty as_nul_certainly() const { assert(kind() == OwcaValueKind::Empty); return {}; }
		OwcaCompleted as_completed_certainly() const { assert(kind() == OwcaValueKind::Completed); return {}; }
		OwcaRange as_range_certainly() const { assert(kind() == OwcaValueKind::Range); return OwcaRange{ (Internal::Range*)internal_ptr1() }; }
		bool as_bool_certainly() const { assert(kind() == OwcaValueKind::Bool); NumberValue tmp; std::memcpy(&tmp, &value_encoded_, sizeof(NumberValue)); return tmp.value != 0; }
		Number as_float_certainly() const { assert(kind() == OwcaValueKind::Float); NumberValue tmp; std::memcpy(&tmp, &value_encoded_, sizeof(NumberValue)); return tmp.value; }
		OwcaString as_string_certainly() const { assert(kind() == OwcaValueKind::String); return { (Internal::String*)internal_ptr1() }; }
		OwcaFunctions as_functions_certainly() const { assert(kind() == OwcaValueKind::Functions); return { (Internal::RuntimeFunctions*)internal_ptr1(), (Internal::AllocationBase*)internal_ptr2() }; }
		OwcaMap as_map_certainly() const { assert(kind() == OwcaValueKind::Map); return OwcaMap{ (Internal::DictionaryShared*)internal_ptr1() }; }
		OwcaClass as_class_certainly() const { assert(kind() == OwcaValueKind::Class); return OwcaClass{ (Internal::Class*)internal_ptr1() }; }
		OwcaObject as_object_certainly() const { assert(kind() == OwcaValueKind::Object); return OwcaObject{ (Internal::Object*)internal_ptr1() }; }
		OwcaTuple as_tuple_certainly() const { assert(kind() == OwcaValueKind::Tuple); return OwcaTuple{ (Internal::Tuple*)internal_ptr1() }; }
		OwcaArray as_array_certainly() const { assert(kind() == OwcaValueKind::Array); return OwcaArray{ (Internal::Array*)internal_ptr1() }; }
		OwcaSet as_set_certainly() const { assert(kind() == OwcaValueKind::Set); return OwcaSet{ (Internal::SetShared*)internal_ptr1() }; }
		OwcaException as_exception_certainly() const { 
			assert(kind() == OwcaValueKind::Exception); 
			return OwcaException{ (Internal::Object*)internal_ptr1(), (Internal::Exception*)internal_ptr2() };
		}
		OwcaIterator as_iterator_certainly() const { assert(kind() == OwcaValueKind::Iterator); return OwcaIterator{ (Internal::Iterator*)internal_ptr1() }; }
		OwcaNamespace as_namespace_certainly() const { assert(kind() == OwcaValueKind::Namespace); return OwcaNamespace{ (Internal::Namespace*)internal_ptr1() }; }

		std::string_view type() const;
		std::string to_string() const;

		OwcaValue member(OwcaVM vm, const std::string& key) const;
		void member(OwcaVM vm, const std::string& key, OwcaValue val);
		OwcaValue call(OwcaVM vm, std::span<OwcaValue> args) const;

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

	void gc_mark_value(OwcaVM vm, GenerationGC gc, OwcaValue);
	inline void gc_mark_value(OwcaVM vm, GenerationGC gc, OwcaEmpty) {}
	inline void gc_mark_value(OwcaVM vm, GenerationGC gc, OwcaCompleted) {}		
	template <typename T> inline void gc_mark_value(OwcaVM vm, GenerationGC gc, T) requires(std::is_arithmetic_v<T>) {}		

	namespace Internal {
		class VM;

		void throw_cant_convert_to_number(OwcaVM vm, size_t I, OwcaValue v);
		template <std::integral T>
		static T convert_impl2(OwcaVM vm, size_t I, T *, OwcaValue v) {
			if (v.kind() != OwcaValueKind::Float) 
				throw_cant_convert_to_number(vm, I, v);
			return (T)v.as_float(vm);
		}
		template <std::floating_point T>
		static auto convert_impl2(OwcaVM vm, size_t I, T *, OwcaValue v) {
			if (v.kind() != OwcaValueKind::Float) 
				throw_cant_convert_to_number(vm, I, v);
			return (T)v.as_float(vm);
		}
		bool convert_impl2(OwcaVM vm, size_t I, bool *b, OwcaValue v);
		std::string convert_impl2(OwcaVM vm, size_t I, std::string *b, OwcaValue v);
		std::string_view convert_impl2(OwcaVM vm, size_t I, std::string_view *b, OwcaValue v);
		OwcaEmpty convert_impl2(OwcaVM vm, size_t I, OwcaEmpty *b, OwcaValue v);
		OwcaRange convert_impl2(OwcaVM vm, size_t I, OwcaRange *b, OwcaValue v);
		Number convert_impl2(OwcaVM vm, size_t I, Number *b, OwcaValue v);
		OwcaString convert_impl2(OwcaVM vm, size_t I, OwcaString *b, OwcaValue v);
		OwcaFunctions convert_impl2(OwcaVM vm, size_t I, OwcaFunctions *b, OwcaValue v);
		OwcaMap convert_impl2(OwcaVM vm, size_t I, OwcaMap *b, OwcaValue v);
		OwcaClass convert_impl2(OwcaVM vm, size_t I, OwcaClass *b, OwcaValue v);
		OwcaObject convert_impl2(OwcaVM vm, size_t I, OwcaObject *b, OwcaValue v);
		OwcaIterator convert_impl2(OwcaVM vm, size_t I, OwcaIterator *b, OwcaValue v);
		OwcaArray convert_impl2(OwcaVM vm, size_t I, OwcaArray *b, OwcaValue v);
		OwcaTuple convert_impl2(OwcaVM vm, size_t I, OwcaTuple *b, OwcaValue v);
		OwcaSet convert_impl2(OwcaVM vm, size_t I, OwcaSet *b, OwcaValue v);
		OwcaException convert_impl2(OwcaVM vm, size_t I, OwcaException *b, OwcaValue v);
		OwcaValue convert_impl2(OwcaVM vm, size_t I, OwcaValue *b, OwcaValue v);

		template <typename T> struct FuncToTuple {
		};
		template <typename ... ARGS> struct FuncToTuple<OwcaValue(OwcaVM, ARGS...)> {
			using type = std::tuple<std::remove_cvref_t<ARGS>...>;
			static constexpr bool is_generator = false;
		};

		template <size_t I, typename ... ARGS> static auto convert_impl(OwcaVM vm, std::span<OwcaValue> args) {
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
		template <typename ... ARGS> static std::tuple<OwcaVM, ARGS...> convert2(OwcaVM vm, std::span<OwcaValue> args, std::tuple<ARGS...> *) {
			assert(sizeof...(ARGS) == args.size());
			std::tuple<ARGS...> dst_args = convert_impl<0, ARGS...>(vm, args);
			return std::tuple_cat(std::tuple<OwcaVM>(vm), std::move(dst_args));
		}

	}

	template <typename F>
	static auto adapt(F &&f) requires (!Internal::FuncToTuple<std::remove_cvref_t<F>>::is_generator) {
		return [f = std::forward<F>(f)](OwcaVM vm, std::span<OwcaValue> args) -> OwcaValue {
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
