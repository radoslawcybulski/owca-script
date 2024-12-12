#include "stdafx.h"
#include "base.h"
#include "tree_try.h"
#include "tree_varspace.h"
#include "op_base.h"
#include "op_try.h"
#include "cmp_compiler.h"
#include "message.h"
#include "op_write.h"

namespace owca { namespace __owca__ {

	RCLMFUNCTION bool tree_try::compile_names()
	{
		cmp->actual_scope->add_stack_yield_size(sizeof(op_flow_try));
		TRY_(mainblock->compile_names());
		if (elseblock) TRY_(elseblock->compile_names());
		if (finallyblock) TRY_(finallyblock->compile_names());

		for(std::list<pair>::iterator it=blocks.begin();it!=blocks.end();++it) {

			if (!it->asvar.empty()) {
				it->vi=cmp->actual_scope->create_ident(it->asvar,it->location,false,false);
				if (it->vi->type==tree_varspace_location::NONE)
					it->vi->type=tree_varspace_location::VARIABLE;
				else
					RCASSERT(it->vi->type==tree_varspace_location::VARIABLE);
			}
			else
				it->vi=NULL;
			for(unsigned int i=0;i<it->types.size();++i) TRY_(it->types[i]->compile_names(tree_expression::AT_NONE));
			TRY_(it->block->compile_names());
		}
		cmp->actual_scope->strip_stack_yield_size(sizeof(op_flow_try));

		return true;
	}

	RCLMFUNCTION bool tree_try::compile_write(opcode_writer &dst)
	{
		opcode_writer_location owl(dst,get_first_location());
		dst << FLOW_TRY;

		RCASSERT(cmp->actual_scope->count_temporary_variables()==0);

		opcode_writer_jump blocks_,elseblock_,finallyblock_,done;

		dst << blocks_ << elseblock_ << finallyblock_ << done;
		RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
		try {
			if (mainblock->compile_write(dst)) dst << FLOW_FIN;
			RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
		} catch(error_information &e) {
			cmp->error(e);
			cmp->actual_scope->clear_temporary_variables(0);
		}

		dst.finalize_jump(blocks_);
		dst << (unsigned int)blocks.size();

		for(std::list<pair>::iterator it=blocks.begin();it!=blocks.end();++it) {
			opcode_writer_jump jmp1,jmp2;
			dst << jmp1 << jmp2 << (unsigned int)it->types.size();

			for(unsigned int i=0;i<it->types.size();++i) {
				opcode_writer_location owl(dst,it->types[i]->get_first_location());
				RCASSERT(cmp->actual_scope->count_temporary_variables()==0);

				TRY1(RCASSERT(it->types[i]->compile_write(dst,tree_expression::ET_RVALUE)));
				dst << FLOW_FIN;
				cmp->actual_scope->pop_temporary_variables(1);
			}

			dst.finalize_jump(jmp1);
			dst << (it->vi ? it->vi->loc : exec_variable_location::invalid);
			RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
			try {
				if (it->block->compile_write(dst)) dst << FLOW_FIN;
				RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
			} catch(error_information &e) {
				cmp->error(e);
				cmp->actual_scope->clear_temporary_variables(0);
			}

			dst.finalize_jump(jmp2);
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
		dst.finalize_jump(done);
		if (elseblock == NULL) dst.finalize_jump(elseblock_);
		if (finallyblock == NULL) dst.finalize_jump(finallyblock_);
		return true;
	}

	unsigned int tree_try::get_first_location() const
	{
		return mainblock->get_first_location();
	}

	unsigned int tree_try::get_last_location() const
	{
		if (finallyblock) return finallyblock->get_last_location();
		if (elseblock) return elseblock->get_last_location();
		if (blocks.size()>0) return blocks.back().block->get_last_location();
		return mainblock->get_last_location();
	}

} }












