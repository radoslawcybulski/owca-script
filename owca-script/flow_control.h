#ifndef RC_OWCA_SCRIPT_FLOW_CONTROLH
#define RC_OWCA_SCRIPT_FLOW_CONTROLH

#include "stdafx.h"
#include "owca_value.h"

namespace OwcaScript {
	namespace Internal {
		struct FlowControlReturn {
			OwcaValue value;
		};
		struct FlowControlContinue {
			unsigned int depth;
		};
		struct FlowControlBreak {
			unsigned int depth;
		};
	}
}

#endif
