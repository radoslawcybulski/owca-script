#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "virtualmachine.h"
#include "exec_array_object.h"
#include "exec_tuple_object.h"
#include "exec_map_object.h"
#include "exec_object.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_internal.h"
#include "exec_callparams.h"
#include "exec_string.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_function_call_validate(opcode_validator &oe)
	{
		unsigned int cnt;
		if (!oe.get(cnt) || cnt==0) return false;
		if (!oe.get_operands(cnt)) return false;
		return true;
	}

	RCLMFUNCTION bool op_function_call_flow(vm_execution_stack_elem_internal &oe)
	{
		unsigned int count;
		oe >> count;
		oe.prepare_call_function_stack(count);
		oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
		return false;
	}

	opcode_validator::boolean_result op_function_call_list_map_validate(opcode_validator &oe)
	{
		//if (!oe.push_local_stack_data_size(sizeof(op_flow_function_call_lm))) return false;
		unsigned int cnt1,cnt2;
		unsigned int mode;
		if (!oe.get(cnt1) || !oe.get(cnt2) || !oe.get(mode)) return false;
		if (cnt1==0) return false;
		if ((mode|7)!=7) return false;
		unsigned int total=cnt1+cnt2+(mode&1 ? 1 : 0)+(mode&2 ? 1 : 0)+(mode&4 ? 1 : 0);
		for(unsigned int i=0;i<cnt2;++i) {
			owca_internal_string *s;
			if (!oe.get(s) || s->data_size()==0) return false;
			if (!s->is_ident()) return false;
		}
		if (!oe.get_operands(total)) return false;
		//if (!oe.pop_local_stack_data_size(sizeof(op_flow_function_call_lm))) return false;
		return true;
	}

	RCLMFUNCTION bool op_function_call_list_map_flow(vm_execution_stack_elem_internal &oe)
	{
		static unsigned char sizemode[]={0,1,1,2,1,2,2,3};
		unsigned int cnt1,cnt2,total;
		unsigned int mode;

		oe >> cnt1 >> cnt2 >> mode;
		total=cnt1+cnt2+sizemode[mode];

		RCASSERT(sizemode[mode]==(mode&1 ? 1 : 0)+(mode&2 ? 1 : 0)+(mode&4 ? 1 : 0));

		oe.prepare_call_function_stack(cnt1,cnt2,mode);
		oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
		return false;
	}

} }

