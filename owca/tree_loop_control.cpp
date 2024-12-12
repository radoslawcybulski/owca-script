#include "stdafx.h"
#include "base.h"
#include "tree_loop_control.h"
#include "op_base.h"
#include "tree_varspace.h"
#include "cmp_compiler.h"
#include "message.h"
#include "op_write.h"

namespace owca { namespace __owca__ {

	RCLMFUNCTION bool tree_loop_control::compile_names()
	{
		if (cmp->actual_scope->loopcount==0) {
            const char *txt = NULL;
            switch(type) {
            case BREAK: txt = "break"; break;
            case CONTINUE: txt = "continue"; break;
            case RESTART: txt = "restart"; break;
            case FINALLY: txt = "finally"; break;
            }
            throw error_information(owca::YERROR_UNEXPECTED_BREAK_CONTINUE_FINALLY_RESTART,location,txt);
        }
		if (!ident.empty()) {
			identloc=cmp->actual_scope->lookup_ident(ident,true);
			if (identloc==NULL) throw error_information(owca::YERROR_UNDEFINED,location,ident);
			if (identloc->type!=tree_varspace_location::LOOP) throw error_information(owca::YERROR_INVALID_IDENT,location,ident);
		}
		else identloc=NULL;

		return false;
	}

	RCLMFUNCTION bool tree_loop_control::compile_write(opcode_writer &dst)
	{
		opcode_writer_location owl(dst,get_first_location());
		RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
		switch(type) {
		case BREAK:
			dst << FLOW_BREAK << (identloc ? identloc->loc : exec_variable_location::invalid);
			break;
		case CONTINUE:
			dst << FLOW_CONTINUE << (identloc ? identloc->loc : exec_variable_location::invalid);
			break;
		case RESTART:
			dst << FLOW_RESTART << (identloc ? identloc->loc : exec_variable_location::invalid);
			break;
		case FINALLY:
			dst << FLOW_FINALLY << (identloc ? identloc->loc : exec_variable_location::invalid);
			break;
		default:
			RCASSERT(0);
			throw error_information(owca::YERROR_INTERNAL,location);
		}
		return false;
	}

} }













