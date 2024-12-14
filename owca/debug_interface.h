#ifndef _RC_Y_DEBUG_INTERFACE_H
#define _RC_Y_DEBUG_INTERFACE_H

namespace owca {
	struct DebugInterface {
		virtual ~DebugInterface() = default;

		virtual void debug_break() = 0;
	};
}

#endif