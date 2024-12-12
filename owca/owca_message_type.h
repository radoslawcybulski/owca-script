#ifndef _RC_Y_MESSAGE_TYPE_H
#define _RC_Y_MESSAGE_TYPE_H

namespace owca {
	enum owca_message_type {
		YMESSAGE_ERROR_BEGIN,
		YERROR_SYNTAX_ERROR,
		YERROR_NO_EXCEPT_CLAUSE,
		YERROR_UNEXPECTED_INDENT,
		YERROR_UNEXPECTED_RETURN_YIELD,
		YERROR_UNEXPECTED_RETURN_VALUE,
		YERROR_MISSING_VALUE_FOR_YIELD_RAISE,
		YERROR_INCORRECT_INDENT,
		YERROR_MIXED_CHARS_IN_INDENT,
		YERROR_UNFINISHED_STRING_CONSTANT,
		YERROR_WRONG_FORMAT_OF_NUMBER,
		YERROR_UNEXPECTED_CHARACTER,
		YERROR_UNEXPECTED_END_OF_FILE,
		YERROR_NOT_LVALUE,
		YERROR_NOT_RVALUE,
		YERROR_ALREADY_DEFINED,
		YERROR_UNDEFINED,
		YERROR_INVALID_IDENT,
		YERROR_UNMATCHED_BRACKET,
		YERROR_UNEXPECTED_BREAK_CONTINUE_FINALLY_RESTART,
		YERROR_KEYWORD_PARAM_USED_TWICE,
		YERROR_VARIABLE_IS_LOCAL,
		YERROR_MEANING_OF_VARIABLE_HAS_CHANGED,
		YERROR_INVALID_STRING_SEQUENCE,
		YERROR_INTERNAL,
		YMESSAGE_ERROR_END,
		YMESSAGE_WARNING_BEGIN,
		YMESSAGE_WARNING_END,
	};
}

#endif