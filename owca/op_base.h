#ifndef _RC_Y_OP_BASE_H
#define _RC_Y_OP_BASE_H

#include "location.h"
#include "operatorcodes.h"
#include "op_flow_data_object.h"

#ifdef RCDEBUG_GC_TEST
#define OPGC oe.vm->run_gc()
#else
#define OPGC
#endif

namespace owca {
	class owca_location;
	class gc_iteration;
	namespace __owca__ {
		struct vm_execution_stack_elem_internal;
		class opcode_executer;
		class opcode_executer_jump;
		class opcode_validator;
		class opcode_writer;
		class opcode_writer_jump;
		class returnvalueflow;
		class exec_function_ptr;
		class exec_generator_function;
		class exec_function_stack_data;
		class exec_variable;
		class exec_variable_location;
		template <class A> class op_flow_data_pointer;
		class virtual_machine;
		class owca_internal_string;
		class owca_internal_string_nongc;
		class exec_function_stack_data;
	}
}

namespace owca { namespace __owca__ {
	enum execopcode {
		EXEC_COMPARE_SIMPLE,
		EXEC_COMPARE,
		EXEC_ADD,
		EXEC_SUB,
		EXEC_MUL,
		EXEC_DIV,
		EXEC_MOD,
		EXEC_LSHIFT,
		EXEC_RSHIFT,
		EXEC_BIN_AND,
		EXEC_BIN_OR,
		EXEC_BIN_XOR,
		EXEC_ACCESS_1_READ,
		EXEC_ACCESS_1_WRITE,
		EXEC_ACCESS_1_WRITE_OPER,
		EXEC_ACCESS_2_READ,
		EXEC_ACCESS_2_WRITE,
		EXEC_ACCESS_2_WRITE_OPER,
		EXEC_BIN_NOT,
		EXEC_SIGN_CHANGE,
		EXEC_ASSIGN_TUPLE,
		EXEC_CREATE_TUPLE,
		EXEC_CREATE_ARRAY,
		EXEC_CREATE_MAP,
		EXEC_CREATE_SET,
		EXEC_CREATE_TUPLE_COMPREHENSION,
		EXEC_CREATE_ARRAY_COMPREHENSION,
		EXEC_CREATE_MAP_COMPREHENSION,
		EXEC_CREATE_SET_COMPREHENSION,
		EXEC_CREATE_INT,
		EXEC_CREATE_REAL,
		EXEC_CREATE_STRING,
		EXEC_CREATE_TRUE,
		EXEC_CREATE_FALSE,
		EXEC_CREATE_NULL,
		EXEC_CREATE_FUNCTION,
		EXEC_CREATE_CLASS,
		EXEC_LOOKUP_READ,
		EXEC_LOOKUP_WRITE,
		EXEC_LOOKUP_WRITE_OPER,
		EXEC_IDENT_READ,
		EXEC_IDENT_WRITE,
		EXEC_IDENT_WRITE_OPER,
		EXEC_IDENT_WRITE_PROPERTY,
		EXEC_FUNCTION_CALL,
		EXEC_FUNCTION_CALL_LIST_MAP,
		EXEC_LOG_AND,
		EXEC_LOG_OR,
		EXEC_LOG_NOT,
		EXEC_IS,
		EXEC_IN,
		EXEC_BOOL,
		EXEC_GENERATOR,
		EXEC_QUE,
		EXEC_COUNT,
		FLOW_FOR,
		FLOW_IF,
		FLOW_NOOP,
		FLOW_RETURN,
		FLOW_RETURN_NO_VALUE,
		FLOW_TRY,
		FLOW_WHILE,
		FLOW_WITH,
		FLOW_BREAK,
		FLOW_CONTINUE,
		FLOW_RESTART,
		FLOW_FINALLY,
		FLOW_YIELD,
		FLOW_RAISE,
		FLOW_RAISE_NO_VALUE,
		FLOW_FIN,
		FLOW_FIN_1_CLEAR,
		FLOW_FIN_1_CLEAR_FOR,
		FLOW_COUNT,
	};
	const char *execopcode_name(execopcode opc);
	enum opcodestreamtype {
		OST_JUMP=0xf0,
		OST_UINT32,
		OST_UINT32_OR_PTR,
		//OST_UINT16,
		OST_EXECOPCODE,
		OST_OPERATOROPCODE,
		OST_VARIABLELOCATION,
		OST_REAL,
		OST_INT,
		OST_STRING,
		OST_STRING_BODY,
		//OST_UCHAR8,
	};

} }
#endif
