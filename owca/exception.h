#ifndef _RC_Y_EXCEPTION_H
#define _RC_Y_EXCEPTION_H

#include "global.h"
#include <exception>

#ifdef OVERFLOW
#undef OVERFLOW
#endif

namespace owca {
	enum class ExceptionCode {
		NONE=0,
		CLASS_CREATION,
		PARAM_ASSIGNED_TWICE,
		PARAM_NOT_SET,
		UNUSED_KEYWORD_PARAM,
		KEYWORD_PARAM_NOT_STRING,
		MISSING_KEY_PARAMETER,
		MISSING_VALUE_PARAMETER,
		NO_CONSTRUCTOR,
		TOO_MANY_PARAMETERS,
		NOT_ENOUGH_PARAMETERS,
		INVALID_LIST_PARAM,
		INVALID_MAP_PARAM,
		INVALID_PARAM_TYPE,
		INTEGER_OUT_OF_BOUNDS, // integer value is out of bounds for given operation (ie too large index for array access)
		UNSUPPORTED_KEYWORD_PARAMETERS,
		INVALID_VM,
		DIVISION_BY_ZERO,
		OVERFLOW,
		INVALID_IDENT,
		NOT_LVALUE,
		NOT_RVALUE,
		MISSING_MEMBER,
		TOO_MANY_ITEMS_IN_ITER,
		TOO_LITTLE_ITEMS_IN_ITER,
		INVALID_RETURN_TYPE,
		INVALID_OPERATOR_FUNCTION,
		MISSING_RETURN_VALUE,
		STACK_OVERFLOW,
		CANT_INSERT,
		KEY_NOT_FOUND,
		NO_COROUTINE_TO_STOP,
		CANT_STOP_FROM_WITHIN_USER_FUNCTION,
		CANT_RESUME_FROM_COROUTINE,
		CANT_RESUME_NORMAL_FUNCTION,
		CANT_RESUME_FINISHED_COROUTINE,
		CANT_STOP_COROUTINE_FROM_USER_FUNCTION,
		CANT_CREATE_GENERATOR_FROM_USER_FUNCTION,
		LIST_MODIFED_WHILE_BEING_SORTED,
		MAP_MODIFED_WHILE_BEING_USED,
		SET_MODIFED_WHILE_BEING_USED,
		NOT_A_TYPE,
		INVALID_UTF8_STRING,
		EMPTY_LIST,
		CODE_FAILED_TO_VALIDATE,
		CPP_EXCEPTION_IN_USER_CODE,
		USER=1024,
	};
	DLLEXPORT std::string to_string(ExceptionCode);

	class owca_exception : public std::exception {
		owca_global _exception_object;
		std::string _message;
		ExceptionCode _code;
	public:
		owca_exception(ExceptionCode code, const std::string& txt) : _message(to_string(code) + ": " + txt), _code(code) { }
		explicit owca_exception(owca_global exception_object);

		struct StacktraceElem {
			std::string function, filename;
			owca_int line;
		};
		const std::string &message() const { return _message; }
		const char* what() const { return _message.c_str(); }
		auto code() const { return _code; }
		const auto& exception_object() const { return _exception_object; }
		auto take_exception_object() { return std::move(_exception_object); }
		bool has_exception_object() const;
		std::vector<StacktraceElem> stacktrace() const;

	};

}

#endif
