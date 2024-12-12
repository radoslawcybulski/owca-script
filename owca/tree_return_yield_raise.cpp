#include "stdafx.h"
#include "base.h"
#include "tree_return_yield_raise.h"
#include "op_base.h"
#include "op_write.h"
#include "cmp_compiler.h"
#include "tree_varspace.h"
#include "tree_function.h"
#include "message.h"

namespace owca { namespace __owca__ {

	bool tree_return_yield_raise::compile_names()
	{
		if (value) {
			TRY_(value->compile_names(tree_expression::AT_NONE));
		}
		switch(type) {
		case YIELD: return true;
		case RAISE:
		case RETURN: break;
		default:
			RCASSERT(0);
		}
		return false;
	}

	RCLMFUNCTION bool tree_return_yield_raise::compile_write(opcode_writer &dst)
	{
		opcode_writer_location owl(dst,get_first_location());
		RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
		switch(type) {
		case YIELD: {
			opcode_writer_location owl(dst,value->get_first_location());
			TRY1(RCASSERT(value->compile_write(dst,tree_expression::ET_RVALUE)));
			cmp->actual_scope->pop_temporary_variables(1);
			dst << FLOW_YIELD;
			return true; }
		case RAISE:
			if (value) {
				opcode_writer_location owl(dst,value->get_first_location());
				TRY1(RCASSERT(value->compile_write(dst,tree_expression::ET_RVALUE)));
				cmp->actual_scope->pop_temporary_variables(1);
				dst << FLOW_RAISE;
			}
			else dst << FLOW_RAISE_NO_VALUE;
			break;
		case RETURN:
			if (value) {
				if (dynamic_cast<tree_function*>(cmp->actual_scope->owner)->generator) {
					throw error_information(owca::YERROR_UNEXPECTED_RETURN_VALUE,location);
				}
				opcode_writer_location owl(dst,value->get_first_location());
				TRY1(RCASSERT(value->compile_write(dst,tree_expression::ET_RVALUE)));
				dst << FLOW_RETURN;
				cmp->actual_scope->pop_temporary_variables(1);
			}
			else dst << FLOW_RETURN_NO_VALUE;
			break;
		default:
			RCASSERT(0);
			throw error_information(owca::YERROR_INTERNAL,location);
		}
		return false;
	}

} }













