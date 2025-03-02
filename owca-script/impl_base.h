#ifndef RC_OWCA_SCRIPT_IMPL_BASE_H
#define RC_OWCA_SCRIPT_IMPL_BASE_H

#include "stdafx.h"
#include "owca_value.h"
#include "line.h"

namespace OwcaScript {
	class OwcaVM;

	namespace Internal {
		struct ExecutionFrame;

		class ImplBase {
		public:
			const Line line;

			ImplBase(Line line) : line(line) {}

			virtual ~ImplBase() = default;
		};

		class ImplStat : public ImplBase {
		public:
			using ImplBase::ImplBase;
			
			// struct ResultNext {};
			// struct ResultBreak { unsigned int depth; };
			// struct ResultContinue { unsigned int depth; };
			// struct ResultException { OwcaException oe; };
			// struct ResultReturn { OwcaValue value; };
			// using Result = std::variant<ResultNext, ResultBreak, ResultContinue, ResultException, ResultReturn>;

			struct Task;
			struct State {
				std::vector<char> storage;
				const size_t size;
				std::exception_ptr exception;

				State(size_t size) : size(size) {
					storage.reserve(size);
				}
				~State() {
					assert(storage.capacity() == size);
				}
				Task *top();
			};
			struct Task
			{
				static constexpr const bool debug_print = false;
			
				struct promise_type;
				State &st;
				Task *prev = nullptr;
				std::coroutine_handle<promise_type> h;

				Task(State &st) : st(st) {
					if constexpr (debug_print) std::cout << "Task(" << (void*)this << ")\n";
				}

				struct Yield {
					State &st;

					Yield(OwcaVM vm, State &st, OwcaValue val);

					bool await_ready() {
						if constexpr (debug_print) std::cout << "Awaiter::await_ready(" << (void*)this << ")\n";
						return false;
					}
					auto await_resume() const
					{
						if (st.exception) {
							auto v = std::move(st.exception);
							st.exception = {};
							std::rethrow_exception(v);
						}
						if constexpr (debug_print) std::cout << "Awaiter::await_resume(" << (void*)this << ")\n";
					}
					bool await_suspend(std::coroutine_handle<promise_type> h) noexcept;
				};
				struct Awaiter {
					State &st;
					Awaiter(State &st) : st(st) {}
			
					bool await_ready() {
						if constexpr (debug_print) std::cout << "Awaiter::await_ready(" << (void*)this << ")\n";
						return false;
					}
					auto await_resume() const
					{
						if (st.exception) {
							auto v = std::move(st.exception);
							st.exception = {};
							std::rethrow_exception(v);
						}
						if constexpr (debug_print) std::cout << "Awaiter::await_resume(" << (void*)this << ")\n";
					}
					bool await_suspend(std::coroutine_handle<promise_type> h) noexcept
					{
						if constexpr (debug_print) std::cout << "Awaiter::await_suspend(" << (void*)this << ") storing to " << ((char*)st.top() - st.storage.data()) <<"\n";
						st.top()->h = h;
						return true;
					}
				};
			
				struct promise_type // required
				{
					State &st;
			
					template <typename T> promise_type(T&, OwcaVM, State &st) : st(st) {
						if constexpr (debug_print) std::cout << "promise_type(" << (void*)this << ")\n";
					}
					~promise_type() {
						if constexpr (debug_print) std::cout << "~promise_type(" << (void*)this << ")\n";
					}
			
					Task get_return_object()
					{
						return { st };
					}
					Awaiter initial_suspend() { return { st }; }
					std::suspend_never final_suspend() noexcept { return {}; }
					void unhandled_exception() { 
						st.exception = std::current_exception(); 
					}
			
					// std::suspend_always yield_value(OwcaValue v) {
					// 	if constexpr (debug_print) std::cout << "yield_value(" << (void*)this << ")\n";
					// 	assert(!Task::placeholder()->value);
					// 	Task::placeholder()->value = v;
					// 	return {};
					// }
					void return_void() {}
			
					template <typename T> void* operator new(std::size_t n, T&, OwcaVM, State &st)
					{
						auto &storage = st.storage;
						auto prev = storage.empty() ? (Task*)nullptr : (Task*)(storage.data() + storage.size() - sizeof(Task));
						auto p = storage.data() + storage.size() + sizeof(State*);
						assert(storage.size() + n + sizeof(Task) + sizeof(State*) <= storage.capacity());
						storage.resize(storage.size() + n + sizeof(Task) + sizeof(State*));
						auto p2 = p + n;
						if constexpr (debug_print) std::cout << "operator new (" << (void*)p << ", " << storage.size() << ") task at " << (p2 - storage.data()) << "\n";
						*(State**)((char*)p - sizeof(State*)) = &st;
						(new (p2) Task{ st })->prev = prev;
						return p;
					}        
			
					void operator delete(void *ptr) noexcept
					{
						auto st = *(State**)((char*)ptr - sizeof(State*));
						auto n = (char*)ptr - st->storage.data() - sizeof(State*);
						if constexpr (debug_print) std::cout << "operator delete (" << ptr << ", " << n << ") at " << ((char*)ptr - sizeof(State*) - st->storage.data()) << "\n";
						st->storage.resize(n);
					}        
				};
				
				~Task() {
					if constexpr (debug_print) std::cout << "~Task(" << (void*)this << ")\n";
					if (h) h.destroy();
				}
				Task(const Task &) = delete;
				Task(Task &&o) : st(o.st), h(std::move(o.h)), prev(o.prev) {
					if constexpr (debug_print) std::cout << "Task move " << (void*)&o << " to " << (void*)this << "\n";
					o.h = {};
				}
				Task &operator = (const Task&) = delete;
				Task &operator = (Task &&o) {
					if (this != &o) {
						if constexpr (debug_print) std::cout << "Task move " << (void*)&o << " to " << (void*)this << "\n";
						if (h) h.destroy();
						h = std::move(o.h);
						o.h = {};
					}
					return *this;
				}
				
				bool completed() const {
					return h && h.done();
				}
				void resume() {
					if constexpr (debug_print) std::cout << "Task::resume(" << (void*)this << ")\n";
					h.resume();
				}
				bool await_ready() {
					if constexpr (debug_print) std::cout << "Task::await_ready(" << (void*)this << ")\n";
					return false;
				}
				auto await_resume() const
				{
					if (st.exception) {
						auto v = std::move(st.exception);
						st.exception = {};
						std::rethrow_exception(v);
					}
					if constexpr (debug_print) std::cout << "Task::await_resume(" << (void*)this << ")\n";
				}
				bool await_suspend(std::coroutine_handle<promise_type> h) noexcept
				{
					if constexpr (debug_print) std::cout << "Task::await_suspend(" << (void*)this << ") storing to " << ((char*)st.top()->prev - st.storage.data()) <<"\n";
					st.top()->prev->h = h;
					return true;
				}
			};

			virtual void execute_statement_impl(OwcaVM ) const = 0;
			virtual Task execute_generator_statement_impl(OwcaVM, State &) const = 0;
			virtual size_t calculate_generator_allocation_size() const = 0;

			size_t calculate_generator_object_size_for_this() const;
			void execute_statement(OwcaVM ) const;
			Task execute_generator_statement(OwcaVM, State &) const;
		};

		class ImplExpr : public ImplBase {
		public:
			using ImplBase::ImplBase;

			virtual OwcaValue execute_expression_impl(OwcaVM ) const = 0;

			OwcaValue execute_expression(OwcaVM ) const;
		};
	}
}

#endif
