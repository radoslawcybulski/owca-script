#ifndef RC_OWCA_SCRIPT_IMPL_BASE_H
#define RC_OWCA_SCRIPT_IMPL_BASE_H

#include "stdafx.h"
#include "owca_value.h"
#include "line.h"

#define IMPL_DEFINE_VARS_IMPL(name, ...) __VA_ARGS__ name;
#define IMPL_DEFINE_INIT_PARAMS_IMPL(name, ...) __VA_ARGS__ name,
#define IMPL_DEFINE_INIT_ASSIGN_IMPL(name, ...) this->name = name;
#define IMPL_DEFINE_INIT_STORE_IMPL(name, ...) ser.serialize(this->name);
#define IMPL_DEFINE_INIT_LOAD_IMPL(name, ...) ser.deserialize(this->name);

#define IMPL_DEFINE_VARS FIELDS(IMPL_DEFINE_VARS_IMPL);
#define IMPL_DEFINE_INIT \
	void init(FIELDS(IMPL_DEFINE_INIT_PARAMS_IMPL) void *ignore___ = nullptr) { \
		FIELDS(IMPL_DEFINE_INIT_ASSIGN_IMPL) \
	}
#define IMPL_DEFINE_STORE \
	void store_members(Serializer &ser) const override { \
		FIELDS(IMPL_DEFINE_INIT_STORE_IMPL) \
	}
#define IMPL_DEFINE_LOAD \
	void load_members(Deserializer &ser) override { \
		FIELDS(IMPL_DEFINE_INIT_LOAD_IMPL) \
	}
#define IMPL_DEFINE_COMPARE_IMPL(name, ...) if (!Comparer::compare(this->name, reinterpret_cast<decltype(this)>(r)->name)) return false;
#define IMPL_DEFINE_COMPARE_EXPR \
	bool compare(const ImplExpr *r) const { \
		if (kind() != r->kind()) return Comparer::debug_break(false); \
		FIELDS(IMPL_DEFINE_COMPARE_IMPL) \
		return true; \
	}
#define IMPL_DEFINE_COMPARE_STAT \
	bool compare(const ImplStat *r) const { \
		FIELDS(IMPL_DEFINE_COMPARE_IMPL) \
		return true; \
	}

#define IMPL_DEFINE_COMMON(a) \
	Kind kind() const override { return a; } \
	IMPL_DEFINE_VARS \
	IMPL_DEFINE_INIT \
	IMPL_DEFINE_STORE \
	IMPL_DEFINE_LOAD
#define IMPL_DEFINE_EXPR(a) IMPL_DEFINE_COMMON(a) \
	IMPL_DEFINE_COMPARE_EXPR
#define IMPL_DEFINE_STAT(a) IMPL_DEFINE_COMMON(a) \
	IMPL_DEFINE_COMPARE_STAT


namespace OwcaScript {
	class OwcaVM;

	namespace Internal {
		struct ExecutionFrame;
		class ImplStat;
		class ImplExpr;

		template <typename T, typename = void> struct HasSerializeObject {
			static constexpr const bool value = false;
		};
		template <typename T> struct HasSerializeObject<T, decltype(std::declval<T>().serialize_object(std::declval<Serializer&>()))> {
			static constexpr const bool value = true;
		};

		class Serializer {
		public:
			enum class SerializationOperation : unsigned char;
			static constexpr const std::string_view magic = "owca-script.";

		private:
			std::vector<unsigned char> output_storage;

			void write(SerializationOperation);
			void write(const void *, size_t);

			void serialize_array_header();
			void serialize_pair_header();
			void serialize_tuple_header();
			void serialize_enum_header();
			template <size_t I, typename ... ARGS> void serialize_tuple_impl(const std::tuple<ARGS...> &s) {
				if constexpr (I < sizeof...(ARGS)) {
					serialize(std::get<I>(s));
					serialize_tuple_impl<I + 1>(s);
				}
			}
			void serialize_impl(std::uint64_t);
			void serialize_impl(std::int64_t);
			
		public:
			enum class SerializationOperation : unsigned char;

			Serializer(size_t size);
			
			std::vector<unsigned char> take_result() {
				return std::move(output_storage);
			}
			void serialize(std::uint8_t v) { serialize_impl((std::uint64_t)v); }
			void serialize(std::uint16_t v) { serialize_impl((std::uint64_t)v); }
			void serialize(std::uint32_t v) { serialize_impl((std::uint64_t)v); }
			void serialize(std::uint64_t v) { serialize_impl((std::uint64_t)v); }
			void serialize(std::int8_t v) { serialize_impl((std::int64_t)v); }
			void serialize(std::int16_t v) { serialize_impl((std::int64_t)v); }
			void serialize(std::int32_t v) { serialize_impl((std::int64_t)v); }
			void serialize(std::int64_t v) { serialize_impl((std::int64_t)v); }
			void serialize(float);
			void serialize(double);
			void serialize(bool);
			void serialize(std::string_view);
			void serialize(const ImplExpr &);
			void serialize(const ImplStat &);
			void serialize(const ImplExpr *s);
			void serialize(const ImplStat *s);
			template <typename T> void serialize(const T &t) requires(HasSerializeObject<T>::value) {
				t.serialize_object(*this);
			}
			template <typename T> void serialize(const T &t) requires(std::is_enum_v<T>) {
				serialize_enum_header();
				serialize_impl((std::uint64_t)t);
			}
			template <typename A, typename B> void serialize(const std::pair<A, B> &s) {
				serialize_pair_header();
				serialize(s.first);
				serialize(s.second);
			}
			template <typename ... ARGS> void serialize(const std::tuple<ARGS...> &s) {
				serialize_tuple_header();
				serialize_tuple_impl<0>(s);
			}
			template <typename T> void serialize(std::span<T> s) {
				serialize_array_header();
				serialize(s.size());
				for(auto &q : s) serialize(q);
			}
		};
		class Deserializer {
			using SerializationOperation = Serializer::SerializationOperation;

			std::span<unsigned char> data;
			std::vector<char> loaded_data;
			std::span<std::function<ImplStat*(Deserializer&, Line)>> stat_constructors;
			std::span<std::function<ImplExpr*(Deserializer&, Line)>> expr_constructors;
			std::string_view fname;
			size_t position = 0, loaded_data_size = 0;

			SerializationOperation peek();
			void require(SerializationOperation);
			bool try_require(SerializationOperation);
			void read(void *dst, size_t s);
			void *allocate_raw(size_t size, size_t align);
			ImplStat *allocate_stat(unsigned int, Line line);
			ImplExpr *allocate_expr(unsigned int, Line line);

			void deserialize_array_header();
			void deserialize_pair_header();
			void deserialize_tuple_header();
			void deserialize_enum_header();
			void deserialize_impl(std::uint8_t&);
			void deserialize_impl(std::uint16_t&);
			void deserialize_impl(std::uint32_t&);
			void deserialize_impl(std::uint64_t&);
		public:
			Deserializer(std::string_view fname, std::span<unsigned char> data, std::span<std::function<ImplStat*(Deserializer&, Line)>> stat_constructors, std::span<std::function<ImplExpr*(Deserializer&, Line)>> expr_constructors);

			std::vector<char> take_loaded_data();
			void deserialize(std::uint8_t &v);
			void deserialize(std::uint16_t &v);
			void deserialize(std::uint32_t &v);
			void deserialize(std::uint64_t &v);
			void deserialize(std::int8_t &v);
			void deserialize(std::int16_t &v);
			void deserialize(std::int32_t &v);
			void deserialize(std::int64_t &v);
			void deserialize(float&);
			void deserialize(double&);
			void deserialize(bool&);
			void deserialize(std::string_view&);
			void deserialize(ImplExpr *&);
			void deserialize(ImplStat *&);
			template <typename T> void deserialize(T &t) requires(HasSerializeObject<T>::value) {
				t.deserialize_object(*this);
			}
			template <typename T> void deserialize(T &t) requires(std::is_enum_v<T>) {
				deserialize_enum_header();
				std::uint64_t temp;
				deserialize(temp);
				t = (T)temp;
			}
			template <typename A, typename B> void deserialize(std::pair<A, B> &s) {
				deserialize_pair_header();
				deserialize(s.first);
				deserialize(s.second);
			}
			template <size_t I, typename ... ARGS> void deserialize_tuple_impl(std::tuple<ARGS...> &s) {
				if constexpr (I < sizeof...(ARGS)) {
					deserialize(std::get<I>(s));
					deserialize_tuple_impl<I + 1>(s);
				}
			}
			template <typename ... ARGS> void deserialize(std::tuple<ARGS...> &s) {
				deserialize_tuple_header();
				deserialize_tuple_impl<0>(s);
			}
			template <typename T> void deserialize(std::span<T> &s) {
				deserialize_array_header();
				size_t size;
				deserialize(size);
				s = allocate_array<T>(size);
				for(size_t i = 0u; i < size; ++i) {
					deserialize(s[i]);
				}
			}
			template <typename T> T *allocate_object(Line line) {
				auto ptr = allocate_raw(sizeof(T), alignof(T));
				return new (ptr) T{ line };
			}
			template <typename T> std::span<T> allocate_array(size_t size) {
				if (size == 0) return {};
				auto ptr = allocate_raw(sizeof(T) * size, alignof(T));
				auto ptr2 = (T*)ptr;
				for(size_t i = 0u; i < size; ++i) {
					new (ptr2 + i) T();
				}
				return std::span<T>((T*)ptr, size);
			}
		};

		class ImplBase {
		public:
			const Line line;

			ImplBase(Line line) : line(line) {}

			class Comparer {
				template <size_t I, typename ... ARGS> static bool compare_impl(const std::tuple<ARGS...> &l, const std::tuple<ARGS...> &r) {
					if constexpr ( I < sizeof...(ARGS)) {
						if (!compare(std::get<I>(l), std::get<I>(r))) return false;
						return compare_impl<I + 1>(l, r);
					}
					return true;
				}
			public:
				static bool debug_break(bool);
				static bool compare(ImplBase *, ImplBase *);
				template <typename T> static bool compare(T l, T r) requires(std::is_integral_v<T>) {
					return debug_break(l == r);
				}
				template <typename T> static bool compare(T l, T r) requires(std::is_floating_point_v<T>) {
					return debug_break(l == r);
				}
				static bool compare(bool l, bool r) { return debug_break(l == r); }
				static bool compare(std::string_view l, std::string_view r) { return debug_break(l == r); }
				template <typename T> static bool compare(std::span<T> l, std::span<T> r) {
					if (l.size() != r.size()) return debug_break(false);
					for(size_t i = 0; i < l.size(); ++i) {
						if (!compare(l[i], r[i])) return false;
					}
					return true;
				}
				template <typename A, typename B> static bool compare(const std::pair<A, B> &l, const std::pair<A, B> &r) {
					return compare(l.first, r.first) && compare(l.second, r.second);
				}
				template <typename ... ARGS> static bool compare(const std::tuple<ARGS...> &l, const std::tuple<ARGS...> &r) {
					return compare_impl<0>(l, r);
				}
				static bool compare(const ImplExpr *l, const ImplExpr *r);
				static bool compare(const ImplStat *l, const ImplStat *r);
				template <typename T> static bool compare(const T &l, const T &r) requires(HasSerializeObject<T>::value) {
					return debug_break(l.compare(r));
				}
				template <typename T> static bool compare(T l, T r) requires(std::is_enum_v<T>) {
					return debug_break(l == r);
				}
			};

			virtual ~ImplBase() = default;
		};

		class ImplStat : public ImplBase {
		public:
			enum class Kind : unsigned char {
				Block,
				ExprAsStat,
				For,
				If,
				Break,
				Continue,
				Return,
				Throw,
				Try,
				While,
				With,
				Yield,
				_Count
			};

			using ImplBase::ImplBase;

			virtual Kind kind() const = 0;
			virtual void store_members(Serializer &) const = 0;
			virtual void load_members(Deserializer &) = 0;

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

			virtual bool compare(const ImplStat *) const = 0;
			virtual void execute_statement_impl(OwcaVM ) const = 0;
			virtual Task execute_generator_statement_impl(OwcaVM, State &) const = 0;
			virtual size_t calculate_generator_allocation_size() const = 0;

			size_t calculate_generator_object_size_for_this() const;
			void execute_statement(OwcaVM ) const;
			Task execute_generator_statement(OwcaVM, State &) const;
		};

		class ImplExpr : public ImplBase {
		public:
			enum class Kind : unsigned char {
				NativeClass,
				ScriptClass,
				NativeFunction,
				ScriptFunction,				
				Compare,
				ConstantEmpty,
				ConstantBool,
				ConstantInt,
				ConstantFloat,
				ConstantString,
				Ident,
				MemberRead,
				MemberWrite,
				BinNeg,
				LogNot,
				Negate,
				LogOr,
				LogAnd,
				BinOr,
				BinAnd,
				BinXor,
				BinLShift,
				BinRShift,
				MakeRange,
				Add,
				Sub,
				Mul,
				Div,
				Mod,
				Call,
				CreateArray,
				CreateTuple,
				CreateSet,
				CreateMap,
				_Count
			};

			using ImplBase::ImplBase;

			virtual Kind kind() const = 0;
			virtual void store_members(Serializer &) const = 0;
			virtual void load_members(Deserializer &) = 0;

			virtual OwcaValue execute_expression_impl(OwcaVM ) const = 0;
			virtual bool compare(const ImplExpr *) const = 0;

			OwcaValue execute_expression(OwcaVM ) const;
		};
	}
}

#endif
