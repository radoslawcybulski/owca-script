#ifndef RC_OWCA_SCRIPT_EXECUTION_FRAME_H
#define RC_OWCA_SCRIPT_EXECUTION_FRAME_H

#include "stdafx.h"
#include "line.h"
#include "owca_iterator.h"

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
			struct ForState {

				friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ForState &e);
			};
			struct WhileState {
				friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const WhileState &e);
			};
			struct TryCatchState {
				friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const TryCatchState &e);
			};
			using States = std::variant<ForState, WhileState, TryCatchState>;
			friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const States &e);
			
			RuntimeFunctions* runtime_functions = nullptr;
			RuntimeFunction* runtime_function = nullptr;
			std::vector<OwcaValue> values;
			std::vector<OwcaValue> temporaries;
			std::vector<States> states;
			const OwcaCode *code = nullptr;
			std::uint32_t code_position, prev_code_position;
			OwcaValue *return_value = nullptr;
			bool constructor_move_self_to_return_value = false;
			bool iterator_update_completed = false;

			ExecutionFrame();
			~ExecutionFrame();

			unsigned int current_line() const { return prev_code_position; }
			void clear();
			void initialize_code_block(OwcaValue &return_value, const OwcaCode &oc);
			void initialize_main_block_function(OwcaValue &return_value, VM *vm, RuntimeFunctions* runtime_functions, std::optional<OwcaMap> arguments);
			void initialize_execute_function(OwcaValue &return_value, VM *vm, RuntimeFunctions* runtime_functions, RuntimeFunction* runtime_function, std::optional<OwcaValue> self_value, std::span<OwcaValue> arguments);
			// void set_arguments(OwcaMap arguments);
			// void set_arguments(std::optional<OwcaValue> self, std::span<OwcaValue> arguments);

            friend void gc_mark_value(OwcaVM vm, GenerationGC gc, const ExecutionFrame &);
		};
	}
}

#endif
