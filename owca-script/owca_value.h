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
				std::uint8_t kind;
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
				std::uint8_t kind;
			};

			static_assert(sizeof(PtrsValue) == sizeof(NumberValue));
		};
	}

	class OwcaEmpty {};
	class OwcaCompleted {};

	class OwcaValue {
		friend class Internal::VM;

		using PtrsValue = Internal::ValuePtrs<std::endian::native == std::endian::little>::PtrsValue;
		using NumberValue = Internal::ValuePtrs<std::endian::native == std::endian::little>::NumberValue;
		union {
			PtrsValue ptrs;
			NumberValue number;
		} value_encoded_;

		OwcaValue(OwcaValueKind, void *ptr1, void *ptr2);
		OwcaValue(OwcaValueKind, Number num);

		void *internal_ptr1() const;
		void *internal_ptr2() const;
	public:
		OwcaValue() : OwcaValue(OwcaValueKind::Empty, nullptr, nullptr) {}
		template <typename T> OwcaValue(T value) requires(std::is_same_v<std::remove_cvref_t<T>, bool>) : OwcaValue(OwcaValueKind::Bool, (Number)(value ? 1 : 0)) {}
		template <typename T> OwcaValue(T value) requires(!std::is_same_v<std::remove_cvref_t<T>, bool> && std::is_arithmetic_v<T>) : OwcaValue(OwcaValueKind::Float, (Number)value) {}
		OwcaValue(OwcaEmpty value);
		OwcaValue(OwcaCompleted value);
		OwcaValue(OwcaRange value);
		OwcaValue(OwcaString value);
		OwcaValue(OwcaFunctions value);
		OwcaValue(OwcaMap value);
		OwcaValue(OwcaClass value);
		OwcaValue(OwcaObject value);
		OwcaValue(OwcaTuple value);
		OwcaValue(OwcaArray value);
		OwcaValue(OwcaSet value);
		OwcaValue(OwcaException value);
		OwcaValue(OwcaIterator value);

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

		std::string_view type() const;
		std::string to_string() const;

		OwcaValue member(OwcaVM vm, const std::string& key) const;
		void member(OwcaVM vm, const std::string& key, OwcaValue val);
		OwcaValue call(OwcaVM vm, std::span<OwcaValue> args) const;

		OwcaValue member(const std::string& key) const;
		void member(const std::string& key, OwcaValue val);
		OwcaValue call(std::span<OwcaValue> args) const;

		template <typename ... F> auto visit(F &&...fns) const {
			struct overloaded : F... {
				using F::operator()...;
			};
			auto tmp = overloaded{std::forward<F>(fns)...};
			switch(kind()) {
			case OwcaValueKind::Empty: return tmp(OwcaEmpty{});
			case OwcaValueKind::Completed: return tmp(OwcaCompleted{});
			case OwcaValueKind::Range: return tmp(as_range(nullptr));
			case OwcaValueKind::Bool: return tmp(as_bool(nullptr));
			case OwcaValueKind::Float: return tmp(as_float(nullptr));
			case OwcaValueKind::String: return tmp(as_string(nullptr));
			case OwcaValueKind::Functions: return tmp(as_functions(nullptr));
			case OwcaValueKind::Map: return tmp(as_map(nullptr));
			case OwcaValueKind::Class: return tmp(as_class(nullptr));
			case OwcaValueKind::Object: return tmp(as_object(nullptr));
			case OwcaValueKind::Tuple: return tmp(as_tuple(nullptr));
			case OwcaValueKind::Array: return tmp(as_array(nullptr));
			case OwcaValueKind::Set: return tmp(as_set(nullptr));
			case OwcaValueKind::Exception: return tmp(as_exception(nullptr));
			case OwcaValueKind::Iterator: return tmp(as_iterator(nullptr));
			case OwcaValueKind::_Count: break;
			}
			assert(false);
			throw std::logic_error("invalid OwcaValueKind");
		}
	};

    class Generator
    {
    public:
        struct promise_type;
        using handle_type = std::coroutine_handle<promise_type>;
     
        struct promise_type
        {
            OwcaValue value_;
            std::exception_ptr exception_;
			bool completed = false;

            Generator get_return_object()
            {
                return Generator(handle_type::from_promise(*this));
            }
            std::suspend_always initial_suspend() { return {}; }
            std::suspend_always final_suspend() noexcept { 
				completed = true;
				return {};
			}
            void unhandled_exception() { exception_ = std::current_exception(); }
     
            template<std::convertible_to<OwcaValue> From>
            std::suspend_always yield_value(From&& from)
            {
                value_ = std::forward<From>(from);
                return {};
            }
            void return_void() {}
        };
     
        Generator(handle_type h) : h_(std::move(h)) {
			if (h_.done()) completed = true;
		}
        ~Generator() {
			if (h_) h_.destroy();
		}
		Generator(const Generator &) = delete;
		Generator(Generator &&o) : h_(std::move(o.h_)), completed(o.completed) {
			o.h_ = {};
		}
		Generator &operator = (const Generator&) = delete;
		Generator &operator = (Generator &&o) {
			if (this != &o) {
				if (h_) h_.destroy();
				h_ = std::move(o.h_);
				completed = o.completed;
				o.h_ = {};
			}
			return *this;
		}

        std::optional<OwcaValue> next()
        {
            if (completed) return std::nullopt;
            h_();
            if (h_.promise().exception_)
                std::rethrow_exception(h_.promise().exception_);
            if (h_.done()) {
                completed = true;
                return std::nullopt;
            }
            return h_.promise().value_;
        }
     
    private:
        handle_type h_;
        bool completed = false;
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
		template <typename ... ARGS> struct FuncToTuple<Generator(OwcaVM, ARGS...)> {
			using type = std::tuple<std::remove_cvref_t<ARGS>...>;
			static constexpr bool is_generator = true;
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
	static auto adapt(F &&f) {
		if constexpr (Internal::FuncToTuple<std::remove_cvref_t<F>>::is_generator) {
			return [f = std::forward<F>(f)](OwcaVM vm, std::span<OwcaValue> args) -> Generator {
				using T = typename Internal::FuncToTuple<std::remove_cvref_t<F>>::type;
				auto dest_args = Internal::convert2(vm, args, (T*)nullptr);
				return std::apply(f, dest_args);
			};
		}
		else {
			return [f = std::forward<F>(f)](OwcaVM vm, std::span<OwcaValue> args) -> OwcaValue {
				using T = typename Internal::FuncToTuple<std::remove_cvref_t<F>>::type;
				auto dest_args = Internal::convert2(vm, args, (T*)nullptr);
				return std::apply(f, dest_args);
			};
		}
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
