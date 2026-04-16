#ifndef RC_OWCA_SCRIPT_EXECUTION_FRAME_H
#define RC_OWCA_SCRIPT_EXECUTION_FRAME_H

#include "stdafx.h"
#include "line.h"
#include "owca_iterator.h"
#include "owca_value.h"

namespace OwcaScript {
	class OwcaVM;
    class OwcaValue;
	class OwcaCode;
	class OwcaMap;
    class GenerationGC;
    
	namespace Internal {
        class VM;

        struct RuntimeFunctions;
        struct RuntimeFunction;
        struct Iterator;

		struct ExecutionFrame {
			struct ClassState {
				std::string_view name, full_name;
				Class *cls;

				friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ClassState &e);
			};
			struct ForState {
				std::uint64_t index = (std::uint64_t)-1;
				OwcaIterator iterator;
				std::uint32_t continue_position = 0, end_position = 0;
				std::vector<std::uint32_t> value_indexes;
				std::uint32_t loop_index = 0;
				std::uint8_t loop_control_depth = 0;

				ForState(OwcaIterator iterator) : iterator(iterator) {}

				friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ForState &e);
			};
			struct WhileState {
				std::uint64_t index = (std::uint64_t)-1;
				std::uint32_t end_position = 0, continue_position = 0;
				std::uint32_t loop_index = 0, value_index = 0;
				std::uint8_t loop_control_depth = 0;

				friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const WhileState &e);
			};
			struct TryCatchState {
				std::uint32_t begin_pos = 0, end_pos = 0;
				std::uint32_t catches_pos = 0;
				size_t temporaries_size = 0;

				friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const TryCatchState &e);
			};
			struct WithState {
				OwcaValue context;

				friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const WithState &e);
			};
			using States = std::variant<ForState, WhileState, TryCatchState, WithState, ClassState>;
			friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const States &e);
			
			std::vector<OwcaValue> values;
			std::vector<OwcaValue> temporaries;
			std::vector<States> states;
			std::optional<OwcaIterator> iterator_object;
			RuntimeFunctions* runtime_functions = nullptr;
			RuntimeFunction* runtime_function = nullptr;
			std::uint32_t code_position;
			OwcaValue *return_value = nullptr;
			bool constructor_move_self_to_return_value = false;
			bool is_iterator = false;
			bool throw_return_value = false;

			ExecutionFrame();
			~ExecutionFrame();

			void clear();
			void initialize_code_block(OwcaValue &return_value, VM *vm, const OwcaCode &oc);
			void initialize_main_block_function(OwcaValue &return_value, VM *vm, RuntimeFunctions* runtime_functions, std::optional<OwcaMap> arguments);
			void initialize_execute_function(OwcaValue &return_value, VM *vm, RuntimeFunctions* runtime_functions, RuntimeFunction* runtime_function, std::optional<OwcaValue> self_value, std::span<OwcaValue> arguments);
			// void set_arguments(OwcaMap arguments);
			// void set_arguments(std::optional<OwcaValue> self, std::span<OwcaValue> arguments);

            friend void gc_mark_value(OwcaVM vm, GenerationGC gc, const ExecutionFrame &);
		};
	}
}

#endif
