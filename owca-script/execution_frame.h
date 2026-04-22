#ifndef RC_OWCA_SCRIPT_EXECUTION_FRAME_H
#define RC_OWCA_SCRIPT_EXECUTION_FRAME_H

#include "owca_exception.h"
#include "stdafx.h"
#include "line.h"
#include "owca_iterator.h"
#include "owca_value.h"
#include "exec_buffer.h"

namespace OwcaScript {
	class OwcaVM;
    class OwcaValue;
	class OwcaCode;
	class OwcaMap;
    class GenerationGC;
    
	namespace Internal {
        class VM;
		enum class CompareKind : std::uint8_t;

        struct RuntimeFunctions;
        struct RuntimeFunction;
        struct Iterator;

		struct ExecutionFrame {
			struct ClassState {
				static constexpr const std::uint8_t Kind = 0;
				std::string_view name, full_name;
				Class *cls;

				friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ClassState &e);
			};
			struct ForState {
				static constexpr const std::uint8_t Kind = 1;
				std::uint64_t index = (std::uint64_t)-1;
				OwcaIterator iterator;
				std::uint32_t continue_position = 0, end_position = 0;
				std::uint32_t loop_index = 0;
				std::uint8_t loop_control_depth = 0;

				ForState(OwcaIterator iterator) : iterator(iterator) {}

				friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ForState &e);
			};
			struct WhileState {
				static constexpr const std::uint8_t Kind = 2;
				std::uint64_t index = (std::uint64_t)-1;
				std::uint32_t end_position = 0, continue_position = 0;
				std::uint32_t loop_index = 0, value_index = 0;
				std::uint8_t loop_control_depth = 0;

				friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const WhileState &e);
			};
			struct TryCatchState {
				static constexpr const std::uint8_t Kind = 3;
				std::uint32_t begin_pos = 0, end_pos = 0;
				std::uint32_t catches_pos = 0;
				OwcaValue *temporary_ptr = nullptr;
				std::optional<OwcaException> parent_exception;

				friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const TryCatchState &e);
			};
			struct WithState {
				static constexpr const std::uint8_t Kind = 4;
				OwcaValue context;
				bool entered = false;

				friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const WithState &e);
			};
			union StatesType {
				ClassState class_state;
				ForState for_state;
				WhileState while_state;
				TryCatchState try_catch_state;
				WithState with_state;
			};

			std::uint16_t max_values, max_temporaries, max_states, current_state = 0;
			OwcaValue *values_ = nullptr;
			OwcaValue *temporary_ = nullptr;
			std::uint8_t *state_kinds_ = nullptr;
			StatesType *state_ = nullptr;
			ExecutionFrame *previous_frame = nullptr, *next_frame = nullptr;

#ifdef DEBUG
			OwcaValue *temporaries_begin_, *temporaries_end_;
#endif

			std::optional<OwcaIterator> iterator_object;
			std::optional<OwcaException> exception_in_progress;
			RuntimeFunction* runtime_function = nullptr;
			std::uint32_t code_position = 0;
			OwcaValue *return_value = nullptr;
			OwcaMap *dict_output = nullptr;
			bool constructor_move_self_to_return_value = false;
			bool is_iterator = false;

			size_t temporary_count() const {
#ifdef DEBUG
				return temporary_ - temporaries_begin_;
#else
				return 0;
#endif
			}
			size_t state_count() const {
				return current_state;
			}
			std::string state_debug() const {
				std::string tmp;
				for(size_t i = 0; i < current_state; ++i) {
					switch(state_kinds_[i]) {
						case ClassState::Kind: tmp += "C"; break;
						case ForState::Kind: tmp += "F"; break;
						case WhileState::Kind: tmp += "W"; break;
						case TryCatchState::Kind: tmp += "T"; break;
						case WithState::Kind: tmp += "H"; break;
						default: tmp += "?"; break;
					}
				}
				return tmp;
			}
			void push_value(OwcaValue value) {
#ifdef DEBUG
				assert(temporary_ + 1 >= temporaries_begin_ && temporary_ + 1 <= temporaries_end_);
#endif
				*temporary_ = value;
				++temporary_;
			}
			void pop_values(size_t count) {
#ifdef DEBUG
				assert(temporary_ - count >= temporaries_begin_ && temporary_ - count <= temporaries_end_);
#endif
				temporary_ -= count;
			}
			OwcaValue &peek_value(size_t offset) const {
#ifdef DEBUG				
				assert(temporary_ - offset >= temporaries_begin_ && temporary_ - offset <= temporaries_end_);
#endif
				return *(temporary_ - offset);
			}
			OwcaValue &peek_value_and_make_top(size_t offset) {
#ifdef DEBUG				
				assert(temporary_ - offset >= temporaries_begin_ && temporary_ - offset <= temporaries_end_);
#endif
				auto ptr = temporary_ - offset;
				temporary_ -= (offset - 1);
				return *ptr;
			}
			std::span<OwcaValue> peek_values(size_t offset, size_t count) const {
				assert(offset >= count);
#ifdef DEBUG				
				assert(temporary_ - offset >= temporaries_begin_ && temporary_ - offset <= temporaries_end_);
#endif
				return std::span<OwcaValue>{ temporary_ - offset, count };
			}

			bool has_state() const {
				return current_state > 0;
			}
			template <typename T, typename ... ARGS> T &push_state(ARGS&& ... args) {
				assert(current_state < max_states);
				state_kinds_[current_state] = T::Kind;
				auto ptr = (char*)state_ + sizeof(StatesType) * current_state;
				auto t = new (ptr) T(std::forward<ARGS>(args)...);
				++current_state;
				return *t;
			}
			template <typename T> T &state() {
				assert(current_state > 0);
				assert(state_kinds_[current_state - 1] == T::Kind);
				auto ptr = (char*)state_ + sizeof(StatesType) * (current_state - 1);
				return *reinterpret_cast<T*>(ptr);
			}
			template <typename T> T *try_state() {
				assert(current_state > 0);
				if (state_kinds_[current_state - 1] == T::Kind) {
					auto ptr = (char*)state_ + sizeof(StatesType) * (current_state - 1);
					return reinterpret_cast<T*>(ptr);
				}
				return nullptr;
			}
			void pop_state() {
				assert(current_state > 0);
				--current_state;
			}

			void initialize_code_block(OwcaValue &return_value, VM *vm, const OwcaCode &oc);
			void initialize_main_block_function(OwcaValue &return_value, VM *vm, RuntimeFunctions* runtime_functions, std::optional<OwcaMap> arguments);
			void initialize_execute_function(OwcaValue &return_value, VM *vm, RuntimeFunction* runtime_function, std::optional<OwcaValue> self_value, std::span<OwcaValue> arguments);
			OwcaValue get_identifier(unsigned int index);
			void set_identifier(unsigned int index, OwcaValue value);
			void set_identifier_function(VM *vm, unsigned int index, OwcaValue value);

			// void set_arguments(OwcaMap arguments);
			// void set_arguments(std::optional<OwcaValue> self, std::span<OwcaValue> arguments);

            friend void gc_mark_value(OwcaVM vm, GenerationGC gc, const ExecutionFrame &);

			static ExecutionFrame *create(VM *vm, std::uint16_t values, std::uint16_t temporaries, std::uint16_t states);
			static ExecutionFrame *create(RuntimeFunction *runtime_function);
			std::unique_ptr<ExecutionFrame> clone_for_iterator();
			
			ExecutionFrame();
			~ExecutionFrame();

			void operator delete(void *ptr) {
				delete [] reinterpret_cast<char*>(ptr);
			}

			std::tuple<std::span<std::uint8_t>, std::span<StatesType>, std::span<OwcaValue>, std::span<OwcaValue>> calculate_blocks() const;
			void initialize(std::uint16_t values, std::uint16_t temporaries, std::uint16_t states);
			static size_t calculate_additional_size(std::uint16_t values, std::uint16_t temporaries, std::uint16_t states);
		};
	}
}

#endif
