#ifndef RC_OWCA_SCRIPT_EXECUTION_FRAME_H
#define RC_OWCA_SCRIPT_EXECUTION_FRAME_H

#include "stdafx.h"
#include "line.h"
#include "owca_iterator.h"

namespace OwcaScript {
	class OwcaVM;
    class OwcaValue;
    class GenerationGC;
    
	namespace Internal {
        class VM;

        struct RuntimeFunctions;
        struct RuntimeFunction;
        struct Iterator;

		struct ExecutionFrame {
			RuntimeFunctions* runtime_functions;
			RuntimeFunction* runtime_function;
			Line line;
			std::vector<OwcaValue> values;

			ExecutionFrame(Line line);
            ~ExecutionFrame();

            friend void gc_mark_value(OwcaVM vm, GenerationGC gc, const ExecutionFrame &);
		};
	}
}

#endif
