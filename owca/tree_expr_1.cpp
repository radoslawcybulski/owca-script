#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "tree_expr_1.h"
#include "cmp_base.h"
#include "message.h"
#include "op_write.h"
#include "cmp_compiler.h"
#include "tree_varspace.h"
#include "op_log_not.h"

namespace owca { namespace __owca__ {

	RCLMFUNCTION void tree_expression_1::compile_names(assignment_type assigned)
	{
		switch(opc) {
		case OPCODE_SIGN_CHANGE:
		case OPCODE_BIN_NOT:
			o1->compile_names(AT_NONE);
			return;
		case OPCODE_LOG_NOT:
			o1->compile_names(AT_NONE);
			cmp->actual_scope->add_stack_yield_size(sizeof(op_flow_log_not));
			cmp->actual_scope->strip_stack_yield_size(sizeof(op_flow_log_not));
			return;
		default:
			RCASSERT(0);
		}
		throw error_information(owca::YERROR_INTERNAL,location);
	}

	RCLMFUNCTION bool tree_expression_1::compile_write(opcode_writer &dst, expression_type et)
	{
		if (et!=ET_RVALUE) throw error_information(owca::YERROR_NOT_LVALUE,location);
		RCASSERT(o1->compile_write(dst,ET_RVALUE));
		RCASSERT(cmp->actual_scope->count_temporary_variables()>0);
		switch(opc) {
		case OPCODE_SIGN_CHANGE: dst << EXEC_SIGN_CHANGE; break;
		case OPCODE_BIN_NOT: dst << EXEC_BIN_NOT; break;
		case OPCODE_LOG_NOT: dst << EXEC_LOG_NOT; break;
		default:
			RCASSERT(0);
		}
		return true;
	}

} }












