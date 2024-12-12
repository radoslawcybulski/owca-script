#include "stdafx.h"
#include "base.h"
#include "message.h"
#include "operatorcodes.h"
#include "op_base.h"

namespace owca {

	std::string owca_message_type_text(owca_message_type mt, const std::string &param)
	{
		switch(mt) {
		case YERROR_SYNTAX_ERROR: return "syntax error";
		case YERROR_NO_EXCEPT_CLAUSE: return "at least one except clause must be present";
        case YERROR_UNEXPECTED_INDENT: return "unexpected indent";
		case YERROR_UNEXPECTED_RETURN_YIELD: return "unexpected " + param + " statement";
		case YERROR_UNEXPECTED_RETURN_VALUE: return "unexpected return value";
		case YERROR_MISSING_VALUE_FOR_YIELD_RAISE: return param + " statement must have a value";
		case YERROR_INCORRECT_INDENT: return "depth of indent is mismatched";
		case YERROR_MIXED_CHARS_IN_INDENT: return "different kind of white space characters used for indent";
		case YERROR_UNFINISHED_STRING_CONSTANT: return "unfinished string constant";
        case YERROR_WRONG_FORMAT_OF_NUMBER: return "'" + param + "' is not a valid number";
        case YERROR_UNEXPECTED_CHARACTER: return "unexpected character '" + param + "'";
		case YERROR_UNEXPECTED_END_OF_FILE: return "unexpected end of file";
		case YERROR_NOT_LVALUE: return "not a l-value";
		case YERROR_NOT_RVALUE: return "not a r-value";
		case YERROR_ALREADY_DEFINED: return "'" + param +"' is already defined";
		case YERROR_UNDEFINED: return "'" + param + "' is not defined";
		case YERROR_INVALID_IDENT: return "'" + param + "' is not a valid identificator";
		case YERROR_UNMATCHED_BRACKET: return "bracket '" + param + "' is not matched";
		case YERROR_UNEXPECTED_BREAK_CONTINUE_FINALLY_RESTART: return "unexpected " + param + " statement";
		case YERROR_KEYWORD_PARAM_USED_TWICE: return "keyword argument '" + param + "' used more that once";
		case YERROR_VARIABLE_IS_LOCAL: return "variable '" + param + "' is local";
		case YERROR_MEANING_OF_VARIABLE_HAS_CHANGED: return "meaning of a variable '" + param + "' has changed";
	//#define Q(a) case a: return #a;
		//Q(YERROR_SYNTAX_ERROR)
		//Q(YERROR_NO_EXCEPT_CLAUSE)
		//Q(YERROR_UNEXPECTED_INDENT)
		//Q(YERROR_UNEXPECTED_RETURN_YIELD)
		//Q(YERROR_UNEXPECTED_RETURN_VALUE)
		//Q(YERROR_MISSING_VALUE_FOR_YIELD_RAISE)
		//Q(YERROR_INCORRECT_INDENT)
		//Q(YERROR_MIXED_CHARS_IN_INDENT)
		//Q(YERROR_UNFINISHED_STRING_CONSTANT)
		//Q(YERROR_WRONG_FORMAT_OF_NUMBER)
		//Q(YERROR_UNEXPECTED_CHARACTER)
		//Q(YERROR_UNEXPECTED_END_OF_FILE)
		//Q(YERROR_NOT_LVALUE)
		//Q(YERROR_NOT_RVALUE)
		//Q(YERROR_ALREADY_DEFINED)
		//Q(YERROR_UNDEFINED)
		//Q(YERROR_INVALID_IDENT)
		//Q(YERROR_UNMATCHED_BRACKET)
		//Q(YERROR_UNEXPECTED_BREAK_CONTINUE_FINALLY_RESTART)
		//Q(YERROR_KEYWORD_PARAM_USED_TWICE)
		//Q(YERROR_VARIABLE_IS_LOCAL)
		//Q(YERROR_MEANING_OF_VARIABLE_HAS_CHANGED)
		//Q(YERROR_INTERNAL)
	//#undef Q
		default:
			RCASSERT(0);
			return "";
		}
	}

	std::string owca_message::text() const
	{
        return owca_message_type_text(mt,param());
	}
}

namespace owca {
	namespace __owca__ {
		const char *operatorcodes_name(operatorcodes opc)
		{
			switch(opc) {
			case E_WITH_EXIT: return "E_WITH_EXIT";
			case E_WITH_ENTER: return "E_WITH_ENTER";
			case E_SUB_SELF: return "E_SUB_SELF";
			case E_SUB_R: return "E_SUB_R";
			case E_SUB: return "E_SUB";
			case E_STR: return "E_STR";
			case E_SIGN_CHANGE: return "E_SIGN_CHANGE";
			case E_RSHIFT_SELF: return "E_RSHIFT_SELF";
			case E_RSHIFT_R: return "E_RSHIFT_R";
			case E_RSHIFT: return "E_RSHIFT";
			case E_NOTEQ_R: return "E_NOTEQ_R";
			case E_NOTEQ: return "E_NOTEQ";
			case E_NEW: return "E_NEW";
			case E_MUL_SELF: return "E_MUL_SELF";
			case E_MUL_R: return "E_MUL_R";
			case E_MUL: return "E_MUL";
			case E_MORE_R: return "E_MORE_R";
			case E_MOREEQ_R: return "E_MOREEQ_R";
			case E_MOREEQ: return "E_MOREEQ";
			case E_MORE: return "E_MORE";
			case E_MOD_SELF: return "E_MOD_SELF";
			case E_MOD_R: return "E_MOD_R";
			case E_MOD: return "E_MOD";
			case E_LSHIFT_SELF: return "E_LSHIFT_SELF";
			case E_LSHIFT_R: return "E_LSHIFT_R";
			case E_LSHIFT: return "E_LSHIFT";
			case E_LESS_R: return "E_LESS_R";
			case E_LESSEQ_R: return "E_LESSEQ_R";
			case E_LESSEQ: return "E_LESSEQ";
			case E_LESS: return "E_LESS";
			case E_INIT: return "E_INIT";
			case E_IN: return "E_IN";
			case E_HASH: return "E_HASH";
			case E_GENERATOR: return "E_GENERATOR";
			case E_EQ_R: return "E_EQ_R";
			case E_EQ: return "E_EQ";
			case E_DIV_SELF: return "E_DIV_SELF";
			case E_DIV_R: return "E_DIV_R";
			case E_DIV: return "E_DIV";
			case E_COUNT: return "E_COUNT";
			case E_CALL: return "E_CALL";
			case E_BOOL: return "E_BOOL";
			case E_BIN_XOR_SELF: return "E_BIN_XOR_SELF";
			case E_BIN_XOR_R: return "E_BIN_XOR_R";
			case E_BIN_XOR: return "E_BIN_XOR";
			case E_BIN_OR_SELF: return "E_BIN_OR_SELF";
			case E_BIN_OR_R: return "E_BIN_OR_R";
			case E_BIN_OR: return "E_BIN_OR";
			case E_BIN_NOT: return "E_BIN_NOT";
			case E_BIN_AND_SELF: return "E_BIN_AND_SELF";
			case E_BIN_AND_R: return "E_BIN_AND_R";
			case E_BIN_AND: return "E_BIN_AND";
			case E_ADD_SELF: return "E_ADD_SELF";
			case E_ADD_R: return "E_ADD_R";
			case E_ADD: return "E_ADD";
			case E_ACCESS_2_WRITE: return "E_ACCESS_2_WRITE";
			case E_ACCESS_2_READ: return "E_ACCESS_2_READ";
			case E_ACCESS_1_WRITE: return "E_ACCESS_1_WRITE";
			case E_ACCESS_1_READ: return "E_ACCESS_1_READ";
			}
			RCASSERT(0);
			return "";
		}

		const char *operatorcodes_ident(operatorcodes opc)
		{
			switch(opc) {
			case E_COUNT:
			case E_WITH_EXIT: return NULL;
			case E_WITH_ENTER: return NULL;
			case E_SUB_SELF: return "-=";
			case E_SUB_R: return "-";
			case E_SUB: return "-";
			case E_STR: return NULL;
			case E_SIGN_CHANGE: return "-";
			case E_RSHIFT_SELF: return ">>=";
			case E_RSHIFT_R: return ">>=";
			case E_RSHIFT: return ">>";
			case E_NOTEQ_R: return "!=";
			case E_NOTEQ: return "!=";
			case E_NEW: return NULL;
			case E_MUL_SELF: return "*=";
			case E_MUL_R: return "*";
			case E_MUL: return "*";
			case E_MORE_R: return ">";
			case E_MOREEQ_R: return ">=";
			case E_MOREEQ: return ">=";
			case E_MORE: return ">";
			case E_MOD_SELF: return "%=";
			case E_MOD_R: return "%";
			case E_MOD: return "%";
			case E_LSHIFT_SELF: return "<<=";
			case E_LSHIFT_R: return "<<";
			case E_LSHIFT: return "<<";
			case E_LESS_R: return "<";
			case E_LESSEQ_R: return "<=";
			case E_LESSEQ: return "<=";
			case E_LESS: return "<";
			case E_INIT: return NULL;
			case E_HASH: return NULL;
			case E_GENERATOR: return NULL;
			case E_EQ_R: return "==";
			case E_EQ: return "==";
			case E_DIV_SELF: return "%=";
			case E_DIV_R: return "%";
			case E_DIV: return "%";
			case E_CALL: return NULL;
			case E_BOOL: return NULL;
			case E_BIN_XOR_SELF: return "^=";
			case E_BIN_XOR_R: return "^";
			case E_BIN_XOR: return "^";
			case E_BIN_OR_SELF: return "|=";
			case E_BIN_OR_R: return "|";
			case E_BIN_OR: return "|";
			case E_BIN_NOT: return "~";
			case E_BIN_AND_SELF: return "&=";
			case E_BIN_AND_R: return "&";
			case E_BIN_AND: return "&";
			case E_ADD_SELF: return "+=";
			case E_ADD_R: return "+";
			case E_ADD: return "+";
			case E_IN: return "'in'";
			case E_ACCESS_2_WRITE: return NULL;
			case E_ACCESS_2_READ: return NULL;
			case E_ACCESS_1_WRITE: return NULL;
			case E_ACCESS_1_READ: return NULL;
			}
			RCASSERT(0);
			return "";
		}
	}
}

namespace owca {
	namespace __owca__ {
		const char *execopcode_name(execopcode opc)
		{
			switch(opc) {
			case FLOW_YIELD: return "FLOW_YIELD";
			case FLOW_WITH: return "FLOW_WITH";
			case FLOW_WHILE: return "FLOW_WHILE";
			case FLOW_TRY: return "FLOW_TRY";
			case FLOW_RETURN_NO_VALUE: return "FLOW_RETURN_NO_VALUE";
			case FLOW_RETURN: return "FLOW_RETURN";
			case FLOW_RESTART: return "FLOW_RESTART";
			case FLOW_RAISE_NO_VALUE: return "FLOW_RAISE_NO_VALUE";
			case FLOW_RAISE: return "FLOW_RAISE";
			case FLOW_NOOP: return "FLOW_NOOP";
			case FLOW_IF: return "FLOW_IF";
			case FLOW_FOR: return "FLOW_FOR";
			case FLOW_FIN_1_CLEAR_FOR: return "FLOW_FIN_1_CLEAR_FOR";
			case FLOW_FIN_1_CLEAR: return "FLOW_FIN_1_CLEAR";
			case FLOW_FINALLY: return "FLOW_FINALLY";
			case FLOW_FIN: return "FLOW_FIN";
			case FLOW_COUNT: return "FLOW_COUNT";
			case FLOW_CONTINUE: return "FLOW_CONTINUE";
			case FLOW_BREAK: return "FLOW_BREAK";
			case EXEC_SUB: return "EXEC_SUB";
			case EXEC_SIGN_CHANGE: return "EXEC_SIGN_CHANGE";
			case EXEC_RSHIFT: return "EXEC_RSHIFT";
			case EXEC_QUE: return "EXEC_QUE";
			case EXEC_MUL: return "EXEC_MUL";
			case EXEC_MOD: return "EXEC_MOD";
			case EXEC_LSHIFT: return "EXEC_LSHIFT";
			case EXEC_LOOKUP_WRITE_OPER: return "EXEC_LOOKUP_WRITE_OPER";
			case EXEC_LOOKUP_WRITE: return "EXEC_LOOKUP_WRITE";
			case EXEC_LOOKUP_READ: return "EXEC_LOOKUP_READ";
			case EXEC_LOG_OR: return "EXEC_LOG_OR";
			case EXEC_LOG_NOT: return "EXEC_LOG_NOT";
			case EXEC_LOG_AND: return "EXEC_LOG_AND";
			case EXEC_IS: return "EXEC_IS";
			case EXEC_IN: return "EXEC_IN";
			case EXEC_IDENT_WRITE_PROPERTY: return "EXEC_IDENT_WRITE_PROPERTY";
			case EXEC_IDENT_WRITE_OPER: return "EXEC_IDENT_WRITE_OPER";
			case EXEC_IDENT_WRITE: return "EXEC_IDENT_WRITE";
			case EXEC_IDENT_READ: return "EXEC_IDENT_READ";
			case EXEC_GENERATOR: return "EXEC_GENERATOR";
			case EXEC_FUNCTION_CALL_LIST_MAP: return "EXEC_FUNCTION_CALL_LIST_MAP";
			case EXEC_FUNCTION_CALL: return "EXEC_FUNCTION_CALL";
			case EXEC_DIV: return "EXEC_DIV";
			case EXEC_CREATE_TUPLE_COMPREHENSION: return "EXEC_CREATE_TUPLE_COMPREHENSION";
			case EXEC_CREATE_TUPLE: return "EXEC_CREATE_TUPLE";
			case EXEC_CREATE_TRUE: return "EXEC_CREATE_TRUE";
			case EXEC_CREATE_STRING: return "EXEC_CREATE_STRING";
			case EXEC_CREATE_SET_COMPREHENSION: return "EXEC_CREATE_SET_COMPREHENSION";
			case EXEC_CREATE_SET: return "EXEC_CREATE_SET";
			case EXEC_CREATE_REAL: return "EXEC_CREATE_REAL";
			case EXEC_CREATE_NULL: return "EXEC_CREATE_NULL";
			case EXEC_CREATE_MAP_COMPREHENSION: return "EXEC_CREATE_MAP_COMPREHENSION";
			case EXEC_CREATE_MAP: return "EXEC_CREATE_MAP";
			case EXEC_CREATE_INT: return "EXEC_CREATE_INT";
			case EXEC_CREATE_FUNCTION: return "EXEC_CREATE_FUNCTION";
			case EXEC_CREATE_FALSE: return "EXEC_CREATE_FALSE";
			case EXEC_CREATE_CLASS: return "EXEC_CREATE_CLASS";
			case EXEC_CREATE_ARRAY_COMPREHENSION: return "EXEC_CREATE_ARRAY_COMPREHENSION";
			case EXEC_CREATE_ARRAY: return "EXEC_CREATE_ARRAY";
			case EXEC_COUNT: return "EXEC_COUNT";
			case EXEC_COMPARE_SIMPLE: return "EXEC_COMPARE_SIMPLE";
			case EXEC_COMPARE: return "EXEC_COMPARE";
			case EXEC_BOOL: return "EXEC_BOOL";
			case EXEC_BIN_XOR: return "EXEC_BIN_XOR";
			case EXEC_BIN_OR: return "EXEC_BIN_OR";
			case EXEC_BIN_NOT: return "EXEC_BIN_NOT";
			case EXEC_BIN_AND: return "EXEC_BIN_AND";
			case EXEC_ASSIGN_TUPLE: return "EXEC_ASSIGN_TUPLE";
			case EXEC_ADD: return "EXEC_ADD";
			case EXEC_ACCESS_2_WRITE_OPER: return "EXEC_ACCESS_2_WRITE_OPER";
			case EXEC_ACCESS_2_WRITE: return "EXEC_ACCESS_2_WRITE";
			case EXEC_ACCESS_2_READ: return "EXEC_ACCESS_2_READ";
			case EXEC_ACCESS_1_WRITE_OPER: return "EXEC_ACCESS_1_WRITE_OPER";
			case EXEC_ACCESS_1_WRITE: return "EXEC_ACCESS_1_WRITE";
			case EXEC_ACCESS_1_READ: return "EXEC_ACCESS_1_READ";
			}
			RCASSERT(0);
			return "";
		}
	}
}


