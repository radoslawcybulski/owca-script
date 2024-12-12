#include "stdafx.h"
#include "base.h"
//#include "rc_y.h"
#include "tree_if.h"
#include "op_base.h"
#include "op_write.h"
#include "op_if.h"
#include "cmp_compiler.h"
#include "tree_varspace.h"

namespace owca { namespace __owca__ {

	RCLMFUNCTION bool tree_if::compile_names()
	{
		bool b1,b2,b3;

		b1=false;
		b2=b3=true;
		cmp->actual_scope->add_stack_yield_size(sizeof(op_flow_if));
		for(std::list<pair>::iterator it=blocks.begin();it!=blocks.end();++it) {
			TRY_(it->cond->compile_names(tree_expression::AT_NONE));
			try {
				if (it->block->compile_names()) b1=true;
			} catch(error_information &e) {
				cmp->error(e);
			}
		}
		if (elseblock) TRY_(b2=elseblock->compile_names());
		if (finallyblock) TRY_(b3=finallyblock->compile_names());
		cmp->actual_scope->strip_stack_yield_size(sizeof(op_flow_if));

		if (!b2 && !b3) return false;
		if (!b1 && !b2) return false;
		return true;
	}

	RCLMFUNCTION bool tree_if::compile_write(opcode_writer &dst)
	{
		opcode_writer_jump elseblock_,finallyblock_,done;
		bool b1=false,b2=false,b3=false;

		opcode_writer_location owl(dst,get_first_location());
		dst << FLOW_IF << elseblock_ << finallyblock_ << done;

		RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
		dst << (unsigned int)blocks.size();
		for(std::list<pair>::iterator it=blocks.begin();it!=blocks.end();++it) {
			opcode_writer_jump jmp1;
			dst << jmp1;
			{
				opcode_writer_location owl(dst,it->cond->get_first_location());
				RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
				TRY1(RCASSERT(it->cond->compile_write(dst,tree_expression::ET_RVALUE)));
				dst << EXEC_BOOL << FLOW_FIN;
				cmp->actual_scope->pop_temporary_variables(1);
			}
			RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
			try {
				if (it->block->compile_write(dst)) {
					dst << FLOW_FIN;
					b1=true;
				}
				RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
			} catch(error_information &e) {
				cmp->error(e);
				cmp->actual_scope->clear_temporary_variables(0);
			}
			dst.finalize_jump(jmp1);
		}
		if (elseblock) {
			dst.finalize_jump(elseblock_);
			try {
				if (elseblock->compile_write(dst)) {
					dst << FLOW_FIN;
					b2=true;
				}
				RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
			} catch(error_information &e) {
				cmp->error(e);
				cmp->actual_scope->clear_temporary_variables(0);
			}
		}
		else b2=true;
		if (finallyblock) {
			dst.finalize_jump(finallyblock_);
			try {
				if (finallyblock->compile_write(dst)) {
					dst << FLOW_FIN;
					b3=true;
				}
				RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
			} catch(error_information &e) {
				cmp->error(e);
				cmp->actual_scope->clear_temporary_variables(0);
			}
		}
		else b3=true;
		if (elseblock == NULL) dst.finalize_jump(elseblock_);
		if (finallyblock == NULL) dst.finalize_jump(finallyblock_);
		dst.finalize_jump(done);

		if (!b2 && !b3) return false;
		if (!b1 && !b2) return false;
		return true;
	}

	unsigned int tree_if::get_first_location() const
	{
		return location;
	}

	unsigned int tree_if::get_last_location() const
	{
		if (finallyblock) return finallyblock->get_last_location();
		if (elseblock) return elseblock->get_last_location();
		return blocks.back().block->get_last_location();
	}

} }













