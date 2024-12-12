#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "tree_with.h"
#include "op_with.h"
#include "cmp_compiler.h"
#include "tree_varspace.h"
#include "message.h"
#include "returnvalue.h"
#include "exec_variable.h"
#include "op_write.h"

namespace owca { namespace __owca__ {

	RCLMFUNCTION bool tree_with::compile_names()
	{
		cmp->actual_scope->add_stack_yield_size((unsigned int)(sizeof(op_flow_with)+sizeof(exec_variable)*exprs.size()));
		for(std::list<tt>::iterator it=exprs.begin();it!=exprs.end();++it) {
			if (!it->ident.empty()) {
				tree_varspace_location *v=cmp->actual_scope->create_ident(it->ident,location,false,false);
				RCASSERT(v->type==tree_varspace_location::NONE || v->type==tree_varspace_location::VARIABLE);
				if (v->type==tree_varspace_location::NONE) v->type=tree_varspace_location::VARIABLE;
			}
			TRY_(it->expr->compile_names(tree_expression::AT_NONE));
		}

		TRY_(mainblock->compile_names());
		cmp->actual_scope->strip_stack_yield_size((unsigned int)(sizeof(op_flow_with)+sizeof(exec_variable)*exprs.size()));

		return true;
	}

	RCLMFUNCTION bool tree_with::compile_write(opcode_writer &dst)
	{
		opcode_writer_location owl(dst,get_first_location());
		dst << FLOW_WITH;

		dst << (unsigned int)exprs.size();

		RCASSERT(cmp->actual_scope->count_temporary_variables()==0);

		for(std::list<tt>::iterator it=exprs.begin();it!=exprs.end();++it) {
			RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
			opcode_writer_location owl(dst,it->expr->get_first_location());
			TRY1(it->expr->compile_write(dst,tree_expression::ET_RVALUE));
			cmp->actual_scope->pop_temporary_variables(1);
			dst << FLOW_FIN;
			if (!it->ident.empty()) {
				dst << cmp->actual_scope->lookup_ident(it->ident,true)->loc;
			}
			else dst << exec_variable_location::invalid;
		}

		RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
		try {
			if (mainblock->compile_write(dst)) dst << FLOW_FIN;
			RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
		} catch(error_information &e) {
			cmp->error(e);
			cmp->actual_scope->clear_temporary_variables(0);
		}
		return true;
	}

	unsigned int tree_with::get_first_location() const
	{
		return location;
	}

	unsigned int tree_with::get_last_location() const
	{
		return mainblock->get_last_location();
	}

} }












