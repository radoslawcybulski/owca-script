#ifndef RC_OWCA_SCRIPT_OWCA_BOOL_H
#define RC_OWCA_SCRIPT_OWCA_BOOL_H

#include "stdafx.h"

namespace OwcaScript {
	class OwcaBool {
		bool value;

	public:
		explicit OwcaBool(bool value) : value(value) {}

		auto internal_value() const { return value; }
		explicit operator bool() const { return value; }
	};
}

#endif
