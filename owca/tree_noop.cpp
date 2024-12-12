#include "stdafx.h"
#include "base.h"
#include "tree_noop.h"
#include "op_base.h"
#include "op_write.h"
#include "tree_varspace.h"
#include "cmp_compiler.h"

namespace owca { namespace __owca__ {

	bool tree_noop::compile_names()
	{
		return true;
	}

	bool tree_noop::compile_write(opcode_writer &dst)
	{
		opcode_writer_location owl(dst,get_first_location());
		RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
		dst << FLOW_NOOP;
		return true;
	}

} }













