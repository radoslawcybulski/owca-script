#include "stdafx.h"
#include "base.h"
#include "tree_ident.h"
#include "tree_varspace.h"
#include "op_base.h"
#include "cmp_compiler.h"
#include "message.h"
#include "op_write.h"
#include "op_ident.h"

namespace owca { namespace __owca__ {

	RCLMFUNCTION void tree_expression_ident::compile_names(assignment_type assigned)
	{
		assigntype=assigned;
		switch(assigned) {
		case AT_NONE:
			RCASSERT(!function_class);
			break;
		case AT_EXTERNAL: {
			RCASSERT(!function_class);
			var=cmp->actual_scope->lookup_ident(ident);
			if (var) {
				if (var->owner==cmp->actual_scope) throw error_information(owca::YERROR_VARIABLE_IS_LOCAL,location,ident);
				else if (var->readonly) {
					var->readonly=false;
					throw error_information(owca::YERROR_NOT_LVALUE,location);
				}
			}
			break; }
		case AT_SELF:
			cmp->actual_scope->add_stack_yield_size(sizeof(op_flow_ident_oper));
		case AT_INTERNAL: {
			var=cmp->actual_scope->create_ident(ident,location,function_class,false);
			if (var->type==tree_varspace_location::NONE) {
				var->type=tree_varspace_location::VARIABLE;
			}
			else if (function_class) {
				if (var->type!=tree_varspace_location::VARIABLE) {
					throw error_information(owca::YERROR_ALREADY_DEFINED,location,ident);
				}
			}
			else RCASSERT(!var->readonly);
			if (assigned==AT_SELF) cmp->actual_scope->strip_stack_yield_size(sizeof(op_flow_ident_oper));
			break; }
		default:
			RCASSERT(0);
		}
	}

	RCLMFUNCTION bool tree_expression_ident::compile_write(opcode_writer &dst, expression_type et)
	{
		tree_varspace_location *v=cmp->actual_scope->lookup_ident(ident);
		if (v==NULL) {
			cmp->actual_scope->create_ident(ident,location,function_class,true);
			throw error_information(owca::YERROR_UNDEFINED,location,ident);
		}
		else if (var!=NULL && v!=var) {
			throw error_information(owca::YERROR_MEANING_OF_VARIABLE_HAS_CHANGED,location,ident);
		}
		else if (v->owner==cmp->actual_scope && assigntype==AT_EXTERNAL) {
			RCASSERT(var==NULL);
			throw error_information(owca::YERROR_VARIABLE_IS_LOCAL,location,ident);
		}
		else if (v->owner!=cmp->actual_scope && (assigntype==AT_INTERNAL || assigntype==AT_SELF)) {
			RCASSERT(0);
			throw error_information(owca::YERROR_INVALID_IDENT,location,ident); // failed to create??
		}
		else {
			if (et==ET_RVALUE) {
				RCASSERT(assigntype!=AT_SELF);
				dst << EXEC_IDENT_READ << v->loc;
				cmp->actual_scope->push_temporary_variable();
			}
			else if (et==ET_LVALUE) {
				RCASSERT(assigntype!=AT_SELF);
				dst << EXEC_IDENT_WRITE << v->loc;
				RCASSERT(cmp->actual_scope->count_temporary_variables()>0);
			}
			else if (et==ET_LVALUE_OPER) {
				RCASSERT(assigntype==AT_SELF);
				dst << EXEC_IDENT_WRITE_OPER << v->loc << (operatorcodes)et.opcode;
				RCASSERT(cmp->actual_scope->count_temporary_variables()>0);
			}
			else RCASSERT(0);
			return true;
		}
	}

} }












