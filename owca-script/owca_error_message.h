#ifndef RC_OWCA_SCRIPT_OWCA_ERROR_MESSAGE_H
#define RC_OWCA_SCRIPT_OWCA_ERROR_MESSAGE_H

#include "stdafx.h"

namespace OwcaScript {
	enum class OwcaErrorKind {
		UnexpectedEndOfFile,
		ExpectedIdentifier,
		ExpectedValue,
		IndexOutOfRange,
		TooManyArgumentsForIndexing,
		InvalidNumber,
		UndefinedIdentifier,
		VariableIsConstant,
		NotALValue,
		SyntaxError,
		StringContainsEndOfLineCharacter,
		MissingNativeFunction,
		_Count
	};
	class OwcaErrorMessage {
		OwcaErrorKind kind_;
		std::string file_;
		std::string message_;
		unsigned int line_;
	public:
		OwcaErrorMessage(OwcaErrorKind kind_, std::string file_, unsigned int line_, std::string message_) :
			kind_(kind_), file_(std::move(file_)), message_(std::move(message_)), line_(line_) {}

		auto kind() const { return kind_; }
		const auto& file() const { return file_; }
		const auto& message() const { return message_; }
		auto line() const { return line_; }
	};
}

#endif
