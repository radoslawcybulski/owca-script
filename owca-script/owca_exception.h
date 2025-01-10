#ifndef RC_OWCA_SCRIPT_OWCA_EXCEPTION_H
#define RC_OWCA_SCRIPT_OWCA_EXCEPTION_H

#include "stdafx.h"
#include "owca_value.h"

namespace OwcaScript {
	class OwcaException : public std::exception {
		OwcaValue value_;
		std::string msg;
	public:
		OwcaException(std::string msg, OwcaValue value = {});

		const char* what() const noexcept override { return msg.c_str(); }
		const auto& value() const { return value_; }
	};
}

#endif
