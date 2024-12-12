#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "tree_eq.h"
#include "op_write.h"
#include "tree_varspace.h"
#include "cmp_compiler.h"

namespace owca { namespace __owca__ {

	bool tree_eq::compile_names()
	{
		root->compile_names(tree_expression::AT_NONE);
		return true;
	}

	bool tree_eq::compile_write(opcode_writer &dst)
	{
		RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
		opcode_writer_location owl(dst,location);
		RCASSERT(root->compile_write(dst,tree_expression::ET_RVALUE));
		dst << FLOW_FIN_1_CLEAR;
		RCASSERT(cmp->actual_scope->count_temporary_variables()==1);
		cmp->actual_scope->pop_temporary_variables(1);
		return true;
	}

} }












