#ifndef RC_OWCA_SCRIPT_EXECUTION_FRAME_H
#define RC_OWCA_SCRIPT_EXECUTION_FRAME_H

#include "stdafx.h"
#include "line.h"
#include "owca_iterator.h"

namespace OwcaScript {
	class OwcaVM;
    class OwcaValue;
	class OwcaCode;
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
			
			RuntimeFunctions* runtime_functions;
			RuntimeFunction* runtime_function;
			Line line;
			std::vector<OwcaValue> values;
			std::vector<OwcaValue> temporaries;
			std::vector<States> states;
			OwcaCode *code = nullptr;
			std::uint32_t code_position;

			ExecutionFrame();
			~ExecutionFrame();

			void initialize(RuntimeFunction* runtime_function, RuntimeFunctions* runtime_functions);
			void initialize(OwcaCode &oc);

            friend void gc_mark_value(OwcaVM vm, GenerationGC gc, const ExecutionFrame &);
		};
	}
}

#endif
