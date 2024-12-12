#include "stdafx.h"
#include "base.h"
//#include "op_block.h"
#include "tree_block.h"
#include "cmp_compiler.h"
#include "tree_varspace.h"
#include "op_write.h"

namespace owca { namespace __owca__ {

	RCLMFUNCTION bool tree_block::compile_names()
	{
		bool lastb=true;

		for(std::list<tree_flow*>::iterator it=elements.begin();it!=elements.end() && lastb;++it) {
			TRY_(lastb=(*it)->compile_names());
		}
		return lastb;
	}

	RCLMFUNCTION bool tree_block::compile_write(opcode_writer &dst)
	{
		RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
		if (elements.empty()) {
			RCASSERT(0);
			dst << FLOW_NOOP;
			return true;
		}
		else if (elements.size()==1) {
			bool b=true;

			try {
				b=elements.front()->compile_write(dst);
				RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
			} catch(error_information &e) {
				cmp->error(e);
				cmp->actual_scope->clear_temporary_variables(0);
			}

			return b;
		}
		else {
			bool lastb=true;
			for(std::list<tree_flow*>::iterator it=elements.begin();it!=elements.end() && lastb;++it) {
				try {
					lastb=(*it)->compile_write(dst);
					RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
				} catch(error_information &e) {
					cmp->error(e);
					cmp->actual_scope->clear_temporary_variables(0);
				}
			}
			return lastb;
		}
	}

} }













