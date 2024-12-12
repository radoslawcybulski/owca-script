#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "tree_const.h"
#include "cmp_compiler.h"
#include "exec_string.h"
#include "tree_varspace.h"
#include "tree_function.h"
#include "message.h"
#include "op_write.h"

namespace owca { namespace __owca__ {
	void tree_expression_const::compile_names(assignment_type assigned)
	{
	}

	RCLMFUNCTION bool tree_expression_const::compile_write(opcode_writer &dst, expression_type et)
	{
		if (et!=ET_RVALUE) throw error_information(owca::YERROR_NOT_LVALUE,location);

		cmp->actual_scope->push_temporary_variable();
		switch(type) {
		case T_INT:
			dst << EXEC_CREATE_INT << cmp->get_int(location,value);
			break;
		case T_REAL:
			dst << EXEC_CREATE_REAL << cmp->get_real(location,value);
			break;
		case T_STRING: {
			std::string s;
			bool res = cmp->parse_string(s, location, value);
			if (!res)
				throw error_information(owca::YERROR_INVALID_STRING_SEQUENCE, location);
			dst << EXEC_CREATE_STRING << s;
			break; }
		case T_SPEC:
			dst << EXEC_CREATE_NULL;
			break;
		case T_NULL:
			dst << EXEC_CREATE_NULL;
			break;
		case T_FALSE:
			dst << EXEC_CREATE_FALSE;
			break;
		case T_TRUE:
			dst << EXEC_CREATE_TRUE;
			break;
		case T_CLASS:
		case T_SELF:
			dst << EXEC_IDENT_READ << dynamic_cast<tree_function*>(cmp->actual_scope->owner)->selfvar->loc;
			break;
		default:
			RCASSERT(0);
			throw error_information(owca::YERROR_INTERNAL,location);
		}
		return true;
	}

} }












