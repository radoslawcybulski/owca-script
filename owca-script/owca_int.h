#ifndef RC_OWCA_SCRIPT_OWCA_INT_H
#define RC_OWCA_SCRIPT_OWCA_INT_H

#include "stdafx.h"

namespace OwcaScript {
	using OwcaIntInternal = std::int64_t;

	class OwcaInt {
		OwcaIntInternal value;

	public:
		OwcaInt() = default;
		OwcaInt(OwcaIntInternal value) : value(value) {}

		auto internal_value() const { return value; }
		explicit operator OwcaIntInternal() const { return value; }
	};
}

#endif
