#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_for.h"
#include "tree_for.h"
#include "cmp_base.h"
#include "cmp_compiler.h"
#include "tree_varspace.h"
#include "message.h"
#include "op_write.h"

namespace owca { namespace __owca__ {

	RCLMFUNCTION bool tree_for::compile_names()
	{
		cmp->actual_scope->add_stack_yield_size(sizeof(op_flow_for));
		tree_varspace_location *v=NULL;
		TRY_(loop->compile_names(tree_expression::AT_NONE));
		TRY_(vars->compile_names(tree_expression::AT_INTERNAL));
		cmp->actual_scope->loopcount++;
		if (!name.empty()) {
			v=cmp->actual_scope->create_ident(name,location,false,false);
			RCASSERT(v->type!=tree_varspace_location::LOOP);
			v->type=tree_varspace_location::LOOP;
			v->readonly=true;
		}
		TRY_(mainblock->compile_names());
		if (!name.empty()) {
			v->type=tree_varspace_location::VARIABLE;
			v->readonly=false;
		}
		cmp->actual_scope->loopcount--;
		if (elseblock) TRY_(elseblock->compile_names());
		if (finallyblock) TRY_(finallyblock->compile_names());
		cmp->actual_scope->strip_stack_yield_size(sizeof(op_flow_for));

		return true;
	}

	RCLMFUNCTION bool tree_for::compile_write(opcode_writer &dst)
	{
		opcode_writer_location owl(dst,get_first_location());
		dst << FLOW_FOR;


		RCASSERT(cmp->actual_scope->count_temporary_variables()==0);

		opcode_writer_jump mainblock_,elseblock_,finallyblock_,done_;

		dst << mainblock_ << elseblock_ << finallyblock_ << done_;
		dst << (!name.empty() ? cmp->actual_scope->lookup_ident(name)->loc : exec_variable_location::invalid);

		{
			opcode_writer_location owl(dst,loop->get_first_location());
			RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
			TRY1(RCASSERT(loop->compile_write(dst,tree_expression::ET_RVALUE)));
			dst << EXEC_GENERATOR << FLOW_FIN;
			cmp->actual_scope->pop_temporary_variables(1);
		}
		{
			opcode_writer_location owl(dst,vars->get_first_location());
			RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
			cmp->actual_scope->push_temporary_variable();
			TRY1(RCASSERT(vars->compile_write(dst,tree_expression::ET_LVALUE)));
			dst << FLOW_FIN_1_CLEAR_FOR;
			cmp->actual_scope->pop_temporary_variables(1);
		}

		dst.finalize_jump(mainblock_);
		RCASSERT(cmp->actual_scope->count_temporary_variables()==0);

		try {
			if (mainblock->compile_write(dst)) dst << FLOW_FIN;
			RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
		} catch(error_information &e) {
			cmp->error(e);
			cmp->actual_scope->clear_temporary_variables(0);
		}

		if (elseblock) {
			dst.finalize_jump(elseblock_);
			try {
				if (elseblock->compile_write(dst)) dst << FLOW_FIN;
				RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
			} catch(error_information &e) {
				cmp->error(e);
				cmp->actual_scope->clear_temporary_variables(0);
			}
		}
		if (finallyblock) {
			dst.finalize_jump(finallyblock_);
			try {
				if (finallyblock->compile_write(dst)) dst << FLOW_FIN;
				RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
			} catch(error_information &e) {
				cmp->error(e);
				cmp->actual_scope->clear_temporary_variables(0);
			}
		}
		if (elseblock == NULL) dst.finalize_jump(elseblock_);
		if (finallyblock == NULL) dst.finalize_jump(finallyblock_);
		dst.finalize_jump(done_);

		return true;
	}

	unsigned int tree_for::get_first_location() const
	{
		return mainblock->get_first_location();
	}

	unsigned int tree_for::get_last_location() const
	{
		if (finallyblock) return finallyblock->get_last_location();
		if (elseblock) return elseblock->get_last_location();
		return mainblock->get_last_location();
	}

} }












