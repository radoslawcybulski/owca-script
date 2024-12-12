#include "stdafx.h"
#include "base.h"
#include "virtualmachine.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "op_function.h"
#include "op_execute.h"
#include "exec_function_ptr.h"
#include "exec_stack.h"
#include "returnvalue.h"
#include "exec_variablelocation.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_create_function_validate(opcode_validator &oe)
	{
		unsigned int cnt,paramcnt,defvaluescnt;
		exec_variable_location vl;
		owca_internal_string *s;
		unsigned int md;
		unsigned int scopedepth,scopesize;
		store_uint_or_ptr tempcnt;

		//oe.push_local_stack_data_size(sizeof(op_flow_function));
		if (!oe.get(scopedepth) || !oe.get(scopesize)) return false; // stack depth and size
		if (!oe.get(tempcnt)) return false; // max amount of temporary variables
		if (!oe.get(cnt)) return false; // maximum stack/yield data size
		if (oe.stack_depth()!=scopedepth) return false;
		oe.push_stack_size(scopesize, tempcnt.get_uint(), cnt);
		if (!oe.get(vl) || (vl.valid() && !oe.check(vl))) return false;
		if (!oe.get(s)) return false; // function name
		if (!oe.get(md)) return false;
		if (!oe.get(paramcnt)) return false; // parameters
		if (!oe.get(defvaluescnt)) return false; // num of parameters counting from back with default value
		if (paramcnt>scopesize || defvaluescnt>paramcnt) return false;
		if (((md>>0)&3)>=3) return false; // selftype
		if (((md>>2)&3)>=3) return false; // functiontype
		// 4 bit - generator
		// 5 bit - has list param
		// 6 bit - has map param
		if (md>>7) return false; // garbage
		if (md&32) {
			if (!oe.get(s)) return false; // name
			if (!oe.get(vl) || !oe.check(vl)) return false; // location
		}
		if (md&64) {
			if (!oe.get(s)) return false; // name
			if (!oe.get(vl) || !oe.check(vl)) return false; // location
		}
		for(unsigned int i=0;i<scopesize;++i) {
			if (!oe.get(s)) return false; // name
			if (!oe.get(vl) || !oe.check(vl)) return false; // location
		}
		//for(unsigned int i=0;i<paramcnt;++i) {
		//	if (!oe.get(s)) return false; // name
		//	if (!oe.get(vl) || !oe.check(vl)) return false; // location
		//	bool b;
		//	if (!oe.peek_exec(b)) return false; // default var (or EXEC_NONE)
		//}
		opcode_executer_jump jmp;
		if (!oe.get(jmp)) return false;
		if (!oe.validate_flow()) return false;
		if (oe.compare(jmp)!=0) return false;
		if (!oe.pop_stack_size()) return false;
		oe.execution_can_continue=oe.continue_opcodes=true;
		if (!oe.pop_temporary_variables(defvaluescnt)) return false;
		if (!oe.push_temporary_variable()) return false;
		//oe.pop_local_stack_data_size(sizeof(op_flow_function));
		return true;
	}

	//unsigned int op_flow_function::size() const
	//{
	//	return sizeof(*this);
	//}

	//void op_flow_function::_mark_gc(const owca::gc_iteration &gc) const
	//{
	//	if (fnc) fnc->gc_mark(gc);
	//}

	//void op_flow_function::_release_resources(owca::__owca__::virtual_machine &vm)
	//{
	//	if (fnc) fnc->gc_release(vm);
	//}

	//bool op_flow_function::prepare(vm_execution_stack_elem_internal &oe)
	//{
	//	while(++index<count) {
	//		oe >> params[index].ident >> params[index].location;
	//		if (oe.peek_exec()) return true;
	//	}
	//	vm_execution_stack_elem_internal_jump jmp;
	//	oe >> jmp;
	//	*code=oe.actual_code_position();
	//	oe.set_code_position(jmp);

	//	exec_variable &retval=oe.temp(oe.tempstackactpos++);
	//	retval.set_function_fast(fnc,NULL);
	//	fnc=NULL;

	//	return false;
	//}

	//bool op_flow_function::resume_fin(vm_execution_stack_elem_internal &oe)
	//{
	//	oe.r=returnvalueflow::CONTINUE_OPCODES;
	//	RCASSERT(index>=0 && index<count);

	//	RCASSERT(oe.tempstackactpos>0);
	//	params[index].defaultvalue=oe.temp(--oe.tempstackactpos);

	//	return prepare(oe);
	//}

	RCLMFUNCTION bool op_create_function_flow(vm_execution_stack_elem_internal &oe)
	{
		//op_flow_function *ff;
		//oe.push(ff,0);

		unsigned int paramcnt, varcnt, defvaluescnt, maxdatayieldsize;
		owca_internal_string *s;
		unsigned int mode;
		exec_function_ptr::internal_param_info_nodef *map, *list;
		returnvalue r;
		exec_variable_location slfvar, vl;
		unsigned int scopedepth, scopesize;
		vm_execution_stack_elem_internal_jump *code;
		exec_function_ptr::internal_param_info *params;
		exec_function_ptr::internal_variable_info *variables;
		store_uint_or_ptr maxtempvars;

		oe >> scopedepth;
		oe >> scopesize;
		oe >> maxtempvars;
		oe >> maxdatayieldsize;
		oe >> slfvar;
		oe >> s;
		oe >> mode;
		oe >> paramcnt;
		oe >> defvaluescnt;

		varcnt = scopesize;

		exec_stack *st = oe.stack;
		RCASSERT(st->size() + 1 == scopedepth);

		exec_function_ptr *f = exec_function_ptr::internal_allocate(*oe.vm, s, (mode & 16) ? 1 : 0, oe.opcodes, slfvar,
			code, params, variables, list, map, paramcnt, varcnt, scopesize,
			(exec_function_ptr::selftype)(mode & 3), (exec_function_ptr::functiontype)((mode >> 2) & 3),
			maxdatayieldsize, maxtempvars.get_uint(), st, oe.fnc);

		for (unsigned int i = 0; i < paramcnt - defvaluescnt; ++i) {
			params[i].defaultvalue.setmode(VAR_NO_DEF_VALUE);
			params[i].var = variables + i;
		}
		for (unsigned int i = 0; i < defvaluescnt; ++i) {
			params[paramcnt - defvaluescnt + i].defaultvalue = oe.temp(oe.tempstackactpos - defvaluescnt + i);
			params[paramcnt - defvaluescnt + i].var = variables + paramcnt - defvaluescnt + i;
		}
		oe.tempstackactpos -= defvaluescnt;

		for (unsigned int i = 0; i < varcnt; ++i) {
			oe >> variables[i].ident >> variables[i].location;
		}

		if (mode & 32) {
			oe >> list->ident >> list->location;
		}
		else list->location = exec_variable_location();
		if (mode & 64) {
			oe >> map->ident >> map->location;
		}
		else map->location = exec_variable_location();

		for (unsigned int i = 0; i < paramcnt; ++i) { // all params must be in continuous block of memory
			RCASSERT(params[i].var->location.depth() == params[0].var->location.depth());
			RCASSERT(params[i].var->location.offset() == params[0].var->location.offset() + i);
		}

		vm_execution_stack_elem_internal_jump jmp;
		oe >> jmp;
		*code = oe.actual_code_position();
		oe.set_code_position(jmp);


		oe.temp(oe.tempstackactpos++).set_function_fast(f);
		//ff->index=-1;
		//ff->count=cnt;

		//if (!ff->prepare(oe)) oe.pop(ff);
		//else

		return true;
	}

} }












