#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "tree_expr_2.h"
#include "tree_ident.h"
#include "cmp_base.h"
#include "cmp_compiler.h"
#include "exec_string.h"
#include "message.h"
#include "op_write.h"
#include "operatorcodes.h"
#include "tree_varspace.h"
#include "op_log_or.h"
#include "op_log_and.h"
#include "op_lookup.h"
#include "op_access_1.h"

namespace owca { namespace __owca__ {
	RCLMFUNCTION void tree_expression_2::compile_names(assignment_type assigned)
	{
		switch(opc) {
		case OPCODE_LOOKUP:
			if (assigned!=AT_NONE && dynamic_cast<tree_expression_ident*>(o2)->ident[0]=='$') throw error_information(owca::YERROR_NOT_LVALUE,location);
			o1->compile_names(AT_NONE);
			o2->compile_names(AT_NONE);
			if (assigned==AT_SELF) {
				cmp->actual_scope->add_stack_yield_size(sizeof(op_flow_lookup_oper));
				cmp->actual_scope->strip_stack_yield_size(sizeof(op_flow_lookup_oper));
			}
			break;
		case OPCODE_ACCESS_1:
			if (assigned==AT_EXTERNAL) throw error_information(owca::YERROR_SYNTAX_ERROR,location);
			o1->compile_names(AT_NONE);
			o2->compile_names(AT_NONE);
			if (assigned==AT_SELF) {
				cmp->actual_scope->add_stack_yield_size(sizeof(op_flow_access_1_oper));
				cmp->actual_scope->strip_stack_yield_size(sizeof(op_flow_access_1_oper));
			}
			break;
		case OPCODE_ASSIGN_PROPERTY:
		case OPCODE_ASSIGN:
			o1->compile_names(AT_INTERNAL);
			o2->compile_names(AT_NONE);
			break;
		case OPCODE_ASSIGN_NON_LOCAL:
			o1->compile_names(AT_EXTERNAL);
			o2->compile_names(AT_NONE);
			break;
		case OPCODE_LOG_OR:
			o1->compile_names(AT_NONE);
			cmp->actual_scope->add_stack_yield_size(sizeof(op_flow_log_or));
			o2->compile_names(AT_NONE);
			cmp->actual_scope->strip_stack_yield_size(sizeof(op_flow_log_or));
			break;
		case OPCODE_LOG_AND:
			o1->compile_names(AT_NONE);
			cmp->actual_scope->add_stack_yield_size(sizeof(op_flow_log_and));
			o2->compile_names(AT_NONE);
			cmp->actual_scope->strip_stack_yield_size(sizeof(op_flow_log_and));
			break;
		case OPCODE_ASSIGN_ADD:
		case OPCODE_ASSIGN_SUB:
		case OPCODE_ASSIGN_MUL:
		case OPCODE_ASSIGN_DIV:
		case OPCODE_ASSIGN_MOD:
		case OPCODE_ASSIGN_LSHIFT:
		case OPCODE_ASSIGN_RSHIFT:
		case OPCODE_ASSIGN_BIN_AND:
		case OPCODE_ASSIGN_BIN_OR:
		case OPCODE_ASSIGN_BIN_XOR:
			o1->compile_names(AT_SELF);
			o2->compile_names(AT_NONE);
			break;
		default:
			o1->compile_names(AT_NONE);
			o2->compile_names(AT_NONE);
		}
	}

	RCLMFUNCTION bool tree_expression_2::compile_write(opcode_writer &dst, expression_type et)
	{
		switch(opc) {
		case OPCODE_LOOKUP:
		case OPCODE_ACCESS_1:
			break;
		case OPCODE_ASSIGN_PROPERTY:
		case OPCODE_ASSIGN:
		case OPCODE_ASSIGN_NON_LOCAL:
		case OPCODE_ASSIGN_ADD:
		case OPCODE_ASSIGN_SUB:
		case OPCODE_ASSIGN_MUL:
		case OPCODE_ASSIGN_DIV:
		case OPCODE_ASSIGN_MOD:
		case OPCODE_ASSIGN_LSHIFT:
		case OPCODE_ASSIGN_RSHIFT:
		case OPCODE_ASSIGN_BIN_AND:
		case OPCODE_ASSIGN_BIN_OR:
		case OPCODE_ASSIGN_BIN_XOR:
		case OPCODE_MUL:
		case OPCODE_DIV:
		case OPCODE_MOD:
		case OPCODE_ADD:
		case OPCODE_SUB:
		case OPCODE_LSHIFT:
		case OPCODE_RSHIFT:
		case OPCODE_BIN_AND:
		case OPCODE_BIN_OR:
		case OPCODE_BIN_XOR:
		case OPCODE_IS:
		case OPCODE_IN:
		case OPCODE_LOG_AND:
		case OPCODE_LOG_OR:
			if (et!=ET_RVALUE) throw error_information(owca::YERROR_NOT_LVALUE,location);
			break;
		default:
			RCASSERT(0);
			throw error_information(owca::YERROR_INTERNAL,location);
		}
		switch(opc) {
		case OPCODE_LOOKUP:
			if (et==ET_RVALUE) {
				RCASSERT(o1->compile_write(dst,ET_RVALUE));
				dst << EXEC_LOOKUP_READ << dynamic_cast<tree_expression_ident*>(o2)->ident;
			}
			else if (et==ET_LVALUE) {
				RCASSERT(o1->compile_write(dst,ET_RVALUE));
				dst << EXEC_LOOKUP_WRITE << dynamic_cast<tree_expression_ident*>(o2)->ident;
				cmp->actual_scope->pop_temporary_variables(1);
			}
			else if (et==ET_LVALUE_OPER) {
				RCASSERT(o1->compile_write(dst,ET_RVALUE));
				dst << EXEC_LOOKUP_WRITE_OPER << (operatorcodes)et.opcode << dynamic_cast<tree_expression_ident*>(o2)->ident;
				cmp->actual_scope->pop_temporary_variables(1);
			}
			else RCASSERT(0);
			return true;
		case OPCODE_ACCESS_1:
			RCASSERT(o1->compile_write(dst,ET_RVALUE));
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			if (et==ET_RVALUE) {
				dst << EXEC_ACCESS_1_READ;
			}
			else if (et==ET_LVALUE) {
				dst << EXEC_ACCESS_1_WRITE;
				cmp->actual_scope->pop_temporary_variables(1);
			}
			else if (et==ET_LVALUE_OPER) {
				dst << EXEC_ACCESS_1_WRITE_OPER << (operatorcodes)et.opcode;
				cmp->actual_scope->pop_temporary_variables(1);
			}
			else RCASSERT(0);
			break;
		case OPCODE_ASSIGN_PROPERTY: {
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			tree_expression_ident *ident=dynamic_cast<tree_expression_ident*>(o1);
			RCASSERT(ident);
			tree_varspace_location *v=cmp->actual_scope->lookup_ident(ident->ident);
			RCASSERT(v);
			RCASSERT(v->owner==cmp->actual_scope);
			dst << EXEC_IDENT_WRITE_PROPERTY << v->loc;
			return true; }
		case OPCODE_ASSIGN_NON_LOCAL: {
			tree_expression_ident *ident=dynamic_cast<tree_expression_ident*>(o1);
			if (ident==NULL) throw error_information(owca::YERROR_SYNTAX_ERROR,location);
			tree_varspace_location *v=cmp->actual_scope->lookup_ident(ident->ident);
            if (v==NULL) throw error_information(owca::YERROR_UNDEFINED,location,ident->ident);
            if (v->owner==cmp->actual_scope) throw error_information(owca::YERROR_VARIABLE_IS_LOCAL,location,ident->ident);
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			RCASSERT(o1->compile_write(dst,ET_LVALUE));
			return true; }
		case OPCODE_ASSIGN:
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			RCASSERT(o1->compile_write(dst,ET_LVALUE));
			return true;
		case OPCODE_ASSIGN_ADD:
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			RCASSERT(o1->compile_write(dst,tree_expression::expression_type(ET_LVALUE_OPER,E_ADD_SELF)));
			return true;
		case OPCODE_ASSIGN_SUB:
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			RCASSERT(o1->compile_write(dst,tree_expression::expression_type(ET_LVALUE_OPER,E_SUB_SELF)));
			return true;
		case OPCODE_ASSIGN_MUL:
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			RCASSERT(o1->compile_write(dst,tree_expression::expression_type(ET_LVALUE_OPER,E_MUL_SELF)));
			return true;
		case OPCODE_ASSIGN_DIV:
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			RCASSERT(o1->compile_write(dst,tree_expression::expression_type(ET_LVALUE_OPER,E_DIV_SELF)));
			return true;
		case OPCODE_ASSIGN_MOD:
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			RCASSERT(o1->compile_write(dst,tree_expression::expression_type(ET_LVALUE_OPER,E_MOD_SELF)));
			return true;
		case OPCODE_ASSIGN_LSHIFT:
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			RCASSERT(o1->compile_write(dst,tree_expression::expression_type(ET_LVALUE_OPER,E_LSHIFT_SELF)));
			return true;
		case OPCODE_ASSIGN_RSHIFT:
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			RCASSERT(o1->compile_write(dst,tree_expression::expression_type(ET_LVALUE_OPER,E_RSHIFT_SELF)));
			return true;
		case OPCODE_ASSIGN_BIN_AND:
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			RCASSERT(o1->compile_write(dst,tree_expression::expression_type(ET_LVALUE_OPER,E_BIN_AND_SELF)));
			return true;
		case OPCODE_ASSIGN_BIN_OR:
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			RCASSERT(o1->compile_write(dst,tree_expression::expression_type(ET_LVALUE_OPER,E_BIN_OR_SELF)));
			return true;
		case OPCODE_ASSIGN_BIN_XOR:
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			RCASSERT(o1->compile_write(dst,tree_expression::expression_type(ET_LVALUE_OPER,E_BIN_XOR_SELF)));
			return true;
		case OPCODE_MUL:
			RCASSERT(o1->compile_write(dst,ET_RVALUE));
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			dst << EXEC_MUL;
			break;
		case OPCODE_DIV:
			RCASSERT(o1->compile_write(dst,ET_RVALUE));
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			dst << EXEC_DIV;
			break;
		case OPCODE_MOD:
			RCASSERT(o1->compile_write(dst,ET_RVALUE));
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			dst << EXEC_MOD;
			break;
		case OPCODE_ADD:
			RCASSERT(o1->compile_write(dst,ET_RVALUE));
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			dst << EXEC_ADD;
			break;
		case OPCODE_SUB:
			RCASSERT(o1->compile_write(dst,ET_RVALUE));
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			dst << EXEC_SUB;
			break;
		case OPCODE_LSHIFT:
			RCASSERT(o1->compile_write(dst,ET_RVALUE));
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			dst << EXEC_LSHIFT;
			break;
		case OPCODE_RSHIFT:
			RCASSERT(o1->compile_write(dst,ET_RVALUE));
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			dst << EXEC_RSHIFT;
			break;
		case OPCODE_BIN_AND:
			RCASSERT(o1->compile_write(dst,ET_RVALUE));
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			dst << EXEC_BIN_AND;
			break;
		case OPCODE_BIN_OR:
			RCASSERT(o1->compile_write(dst,ET_RVALUE));
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			dst << EXEC_BIN_OR;
			break;
		case OPCODE_BIN_XOR:
			RCASSERT(o1->compile_write(dst,ET_RVALUE));
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			dst << EXEC_BIN_XOR;
			break;
		case OPCODE_IS:
			RCASSERT(o1->compile_write(dst,ET_RVALUE));
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			dst << EXEC_IS;
			break;
		case OPCODE_IN:
			RCASSERT(o1->compile_write(dst,ET_RVALUE));
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			dst << EXEC_IN;
			break;
		case OPCODE_LOG_AND: {
			RCASSERT(o1->compile_write(dst,ET_RVALUE));
			opcode_writer_jump jmp;
			cmp->actual_scope->pop_temporary_variables(1);
			dst << EXEC_LOG_AND << jmp;
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			dst << FLOW_FIN;
			dst.finalize_jump(jmp);
			RCASSERT(cmp->actual_scope->count_temporary_variables()>0);
			return true; }
		case OPCODE_LOG_OR: {
			RCASSERT(o1->compile_write(dst,ET_RVALUE));
			opcode_writer_jump jmp;
			cmp->actual_scope->pop_temporary_variables(1);
			dst << EXEC_LOG_OR << jmp;
			RCASSERT(o2->compile_write(dst,ET_RVALUE));
			dst << FLOW_FIN;
			dst.finalize_jump(jmp);
			RCASSERT(cmp->actual_scope->count_temporary_variables()>0);
			return true; }
		default:
			RCASSERT(0);
			throw error_information(owca::YERROR_INTERNAL,location);
		}
		cmp->actual_scope->pop_temporary_variables(1);
		return true;
	}
} }












