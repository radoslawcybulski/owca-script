#include "stdafx.h"
#include "owca_error_message.h"

namespace OwcaScript {
    std::string_view to_string(OwcaErrorKind kind)
    {
        switch(kind) {
		case OwcaErrorKind::UnexpectedEndOfFile: return "UnexpectedEndOfFile";
		case OwcaErrorKind::ExpectedIdentifier: return "ExpectedIdentifier";
		case OwcaErrorKind::ExpectedValue: return "ExpectedValue";
		case OwcaErrorKind::IndexOutOfRange: return "IndexOutOfRange";
		case OwcaErrorKind::TooManyArgumentsForIndexing: return "TooManyArgumentsForIndexing";
		case OwcaErrorKind::InvalidNumber: return "InvalidNumber";
		case OwcaErrorKind::UndefinedIdentifier: return "UndefinedIdentifier";
		case OwcaErrorKind::VariableIsConstant: return "VariableIsConstant";
		case OwcaErrorKind::NotALValue: return "NotALValue";
		case OwcaErrorKind::SyntaxError: return "SyntaxError";
		case OwcaErrorKind::StringContainsEndOfLineCharacter: return "StringContainsEndOfLineCharacter";
		case OwcaErrorKind::InvalidIdentifier: return "InvalidIdentifier";
		case OwcaErrorKind::LoopControlError: return "LoopControlError";
 		case OwcaErrorKind::_Count: return "_Count";
        }
        assert(false);
        return "<?>";
    }
    std::string OwcaErrorMessage::to_string() const
    {
        return std::format("{}:{}: {}: {}", file_, line_, OwcaScript::to_string(kind_), message_);
    }
}