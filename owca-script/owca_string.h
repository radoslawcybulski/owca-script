#ifndef RC_OWCA_SCRIPT_OWCA_STRING_H
#define RC_OWCA_SCRIPT_OWCA_STRING_H

#include "stdafx.h"

namespace OwcaScript {
	using OwcaStringInternal = std::string;

	class OwcaString {
		OwcaStringInternal value;

	public:
		OwcaString() = default;
		OwcaString(OwcaStringInternal value) : value(std::move(value)) {}

		const auto &internal_value() const { return value; }
		explicit operator std::string_view() const { return value; }
	};
}

#endif
