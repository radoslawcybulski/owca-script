#ifndef RC_OWCA_SCRIPT_GENERATOR_H
#define RC_OWCA_SCRIPT_GENERATOR_H

#include "stdafx.h"
#include "owca_value.h"

namespace OwcaScript {
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

    namespace Internal {
		template <typename ... ARGS> struct FuncToTuple<Generator(OwcaVM, ARGS...)> {
			using type = std::tuple<std::remove_cvref_t<ARGS>...>;
			static constexpr bool is_generator = true;
		};

        template <typename F>
        static auto adapt(F &&f) requires (Internal::FuncToTuple<std::remove_cvref_t<F>>::is_generator) {
            return [f = std::forward<F>(f)](OwcaVM vm, std::span<OwcaValue> args) -> Generator {
                using T = typename Internal::FuncToTuple<std::remove_cvref_t<F>>::type;
                auto dest_args = Internal::convert2(vm, args, (T*)nullptr);
                return std::apply(f, dest_args);
            };
        }
    }
}

#endif
