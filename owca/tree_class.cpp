#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "tree_class.h"
#include "tree_varspace.h"
#include "op_class.h"
#include "cmp_compiler.h"
#include "virtualmachine.h"

namespace owca { namespace __owca__ {

	RCLMFUNCTION void tree_class::compile_names(assignment_type assigned)
	{
		TRY_(body->compile_names(AT_NONE));

		for(std::list<tree_expression*>::iterator it=inherited.begin();it!=inherited.end();++it) {
			TRY_((*it)->compile_names(AT_NONE));
		}

		cmp->actual_scope->add_stack_yield_size((unsigned int)(sizeof(op_flow_class)));
		cmp->actual_scope->strip_stack_yield_size((unsigned int)(sizeof(op_flow_class)));
	}

	RCLMFUNCTION bool tree_class::compile_write(opcode_writer &dst, expression_type et)
	{
		RCASSERT(et==ET_RVALUE);

		try {
			RCASSERT(body->compile_write(dst,ET_RVALUE));
			//RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
		} catch(error_information &e) {
			cmp->error(e);
			cmp->actual_scope->clear_temporary_variables(0);
		}

		for(std::list<tree_expression*>::iterator it=inherited.begin();it!=inherited.end();++it) {
			TRY_(RCASSERT((*it)->compile_write(dst,ET_RVALUE)););
		}

		dst << EXEC_CREATE_CLASS;
		dst << (unsigned int)inherited.size() << name;

		cmp->actual_scope->pop_temporary_variables((unsigned int)inherited.size()+1);
		cmp->actual_scope->push_temporary_variable();

		return true;
	}

} }












