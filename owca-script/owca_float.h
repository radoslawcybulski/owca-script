#ifndef RC_OWCA_SCRIPT_OWCA_FLOAT_H
#define RC_OWCA_SCRIPT_OWCA_FLOAT_H

#include "stdafx.h"

namespace OwcaScript {
	using OwcaFloatInternal = double;

	class OwcaFloat {
		OwcaFloatInternal value;

	public:
		OwcaFloat() = default;
		OwcaFloat(OwcaFloatInternal value) : value(value) {}

		auto internal_value() const { return value; }
		explicit operator OwcaFloatInternal() const { return value; }
	};
}

#endif
