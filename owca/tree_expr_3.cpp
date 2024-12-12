#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "tree_expr_3.h"
#include "cmp_base.h"
#include "message.h"
#include "op_write.h"
#include "tree_varspace.h"
#include "cmp_compiler.h"
#include "op_que.h"
#include "op_access_2.h"

namespace owca { namespace __owca__ {

	RCLMFUNCTION void tree_expression_3::compile_names(assignment_type assigned)
	{
		switch(opc) {
		case OPCODE_QUE:
			o1->compile_names(AT_NONE);
			cmp->actual_scope->add_stack_yield_size(sizeof(op_flow_que));
			o2->compile_names(assigned);
			o3->compile_names(assigned);
			cmp->actual_scope->strip_stack_yield_size(sizeof(op_flow_que));
			break;
		case OPCODE_ACCESS_2:
			if (assigned==AT_EXTERNAL) throw error_information(owca::YERROR_SYNTAX_ERROR,location);
			o1->compile_names(AT_NONE);
			o2->compile_names(AT_NONE);
			o3->compile_names(AT_NONE);
			if (assigned==AT_SELF) {
				cmp->actual_scope->add_stack_yield_size(sizeof(op_flow_access_2_oper));
				cmp->actual_scope->strip_stack_yield_size(sizeof(op_flow_access_2_oper));
			}
			break;
		default:
			RCASSERT(0);
		}
	}

	RCLMFUNCTION bool tree_expression_3::compile_write(opcode_writer &dst, expression_type et)
	{
		switch(opc) {
		case OPCODE_QUE: {
			opcode_writer_jump jmp1,jmp2;
			RCASSERT(o1->compile_write(dst,ET_RVALUE));
			dst << EXEC_QUE;
			dst << (unsigned int)(et==ET_RVALUE ? 1 : 0) << jmp1 << jmp2;
			cmp->actual_scope->pop_temporary_variables(1);
			RCASSERT(o2->compile_write(dst,et));
			dst << FLOW_FIN;
			dst.finalize_jump(jmp1);
			if (et==ET_RVALUE) cmp->actual_scope->pop_temporary_variables(1);
			RCASSERT(o3->compile_write(dst,et));
			dst << FLOW_FIN;
			dst.finalize_jump(jmp2);
			RCASSERT(cmp->actual_scope->count_temporary_variables()>0);
			break; }
		case OPCODE_ACCESS_2:
			if (et==ET_RVALUE) {
				RCASSERT(o1->compile_write(dst,ET_RVALUE));
				RCASSERT(o2->compile_write(dst,ET_RVALUE));
				RCASSERT(o3->compile_write(dst,ET_RVALUE));
				dst << EXEC_ACCESS_2_READ;
				cmp->actual_scope->pop_temporary_variables(3);
			}
			else if (et==ET_LVALUE) {
				RCASSERT(o1->compile_write(dst,ET_RVALUE));
				RCASSERT(o2->compile_write(dst,ET_RVALUE));
				RCASSERT(o3->compile_write(dst,ET_RVALUE));
				dst << EXEC_ACCESS_2_WRITE;
				cmp->actual_scope->pop_temporary_variables(4);
			}
			else if (et==ET_LVALUE_OPER) {
				RCASSERT(o1->compile_write(dst,ET_RVALUE));
				RCASSERT(o2->compile_write(dst,ET_RVALUE));
				RCASSERT(o3->compile_write(dst,ET_RVALUE));
				dst << EXEC_ACCESS_2_WRITE_OPER << (operatorcodes)et.opcode;
				cmp->actual_scope->pop_temporary_variables(4);
			}
			else RCASSERT(0);
			cmp->actual_scope->push_temporary_variable();
			RCASSERT(cmp->actual_scope->count_temporary_variables()>0);
			break;
		default:
			RCASSERT(0);
			throw error_information(owca::YERROR_INTERNAL,location);
		}
		return true;
	}

} }












