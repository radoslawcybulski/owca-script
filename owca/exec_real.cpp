#include "stdafx.h"
#include "base.h"
#include "virtualmachine.h"
#include "exec_string.h"
#include "exec_class_int.h"
#include "vm_execution_stack_elem_external.h"

namespace owca { namespace __owca__ {
	D_FUNC1(real,new,const exec_variable*)
		{
			switch(p1->mode()) {
			case VAR_NO_PARAM_GIVEN: return_value->set_real(0.0); break;
			case VAR_INT: return_value->set_real((owca_real)p1->get_int()); break;
			case VAR_REAL: return_value->set_real(p1->get_real()); break;
			case VAR_BOOL: return_value->set_real(p1->get_bool() ? 1.0 : 0); break;
			case VAR_STRING: {
				owca_real i;

				if (!to_real(i,p1->get_string()->data_pointer(),p1->get_string()->data_size())) {
					vm->raise_invalid_param(OWCA_ERROR_FORMAT1("'%1' is not a valid string representation of a real value",p1->get_string()->data_pointer()));
					return executionstackreturnvalue::FUNCTION_CALL;
				}
				else return_value->set_real(i);
				break; }
			default:
				vm->raise_invalid_param(*p1,vm->class_int);
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(real,sign,owca_real)
		{
			return_value->set_real(-self);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(real,hash,owca_real)
		{
			owca_int ii=(owca_int)self;
			if ((owca_real)ii==self) return_value->set_int(ii);
			else if (sizeof(owca_int)<=sizeof(owca_real)) return_value->set_int(*(owca_int*)&self);
			else {
				hashfunc f;
				f.process(&self,sizeof(owca_real));
				return_value->set_int(f.value());
			}
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(real,str,owca_real)
		{
			return_value->set_string(vm->allocate_string(real_to_string(self)));
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(real,bool,owca_real)
		{
			return_value->set_bool(self!=0.0);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(real,binnot,owca_int)
		{
			return_value->set_int(~self);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(real,add,owca_real,owca_real)
		{
			return_value->set_real(self+p1);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(real,sub,owca_real,owca_real)
		{
			return_value->set_real(self-p1);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	owca_internal_string *string_mul(virtual_machine *vm, owca_internal_string *s, unsigned int mul);

	D_SELF1(real,mul,owca_real,const exec_variable *)
		{
			switch(p1->mode()) {
			case VAR_INT:
				return_value->set_real(self*p1->get_int());
				break;
			case VAR_REAL:
				return_value->set_real(self*p1->get_real());
				break;
			case VAR_STRING: {
				owca_int i=(owca_int)self;
				if (i==self) {
					if (i<0) {
						vm->raise_invalid_integer(OWCA_ERROR_FORMAT("cant multiply by negative amount"));
						return executionstackreturnvalue::FUNCTION_CALL;
					}
					return_value->set_string(string_mul(vm,p1->get_string(),(unsigned int)i));
				}
				else {
					return_value->set_null(true);
					return executionstackreturnvalue::RETURN;
				}
				break; }
			default:
				return_value->set_null(true);
				return executionstackreturnvalue::RETURN;
			}
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(real,div,owca_real,owca_real)
		{
			if (p1==0.0) {
				vm->raise_division_by_zero();
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			return_value->set_real(self/p1);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(real,eq,owca_real,const exec_variable *)
		{
			switch(p1->mode()) {
			case VAR_INT: return_value->set_bool(self == (owca_real)p1->get_int()); return executionstackreturnvalue::RETURN;
			case VAR_REAL: return_value->set_bool(self == p1->get_real()); return executionstackreturnvalue::RETURN;
			}
			return_value->set_null(true);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(real,noteq,owca_real,const exec_variable *)
		{
			switch(p1->mode()) {
			case VAR_INT: return_value->set_bool(self != (owca_real)p1->get_int()); return executionstackreturnvalue::RETURN;
			case VAR_REAL: return_value->set_bool(self != p1->get_real()); return executionstackreturnvalue::RETURN;
			}
			return_value->set_null(true);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(real,less,owca_real,const exec_variable *)
		{
			switch(p1->mode()) {
			case VAR_INT: return_value->set_bool(self < (owca_real)p1->get_int()); return executionstackreturnvalue::RETURN;
			case VAR_REAL: return_value->set_bool(self < p1->get_real()); return executionstackreturnvalue::RETURN;
			}
			return_value->set_null(true);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(real,more,owca_real,const exec_variable *)
		{
			switch(p1->mode()) {
			case VAR_INT: return_value->set_bool(self > (owca_real)p1->get_int()); return executionstackreturnvalue::RETURN;
			case VAR_REAL: return_value->set_bool(self > p1->get_real()); return executionstackreturnvalue::RETURN;
			}
			return_value->set_null(true);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(real,lesseq,owca_real,const exec_variable *)
		{
			switch(p1->mode()) {
			case VAR_INT: return_value->set_bool(self <= (owca_real)p1->get_int()); return executionstackreturnvalue::RETURN;
			case VAR_REAL: return_value->set_bool(self <= p1->get_real()); return executionstackreturnvalue::RETURN;
			}
			return_value->set_null(true);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(real,moreeq,owca_real,const exec_variable *)
		{
			switch(p1->mode()) {
			case VAR_INT: return_value->set_bool(self >= (owca_real)p1->get_int()); return executionstackreturnvalue::RETURN;
			case VAR_REAL: return_value->set_bool(self >= p1->get_real()); return executionstackreturnvalue::RETURN;
			}
			return_value->set_null(true);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(real,lshift,owca_int,owca_int)
		{
			if (p1<0) {
				vm->raise_invalid_integer(OWCA_ERROR_FORMAT("cant left shift by a negative amount"));
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			return_value->set_int(self << p1);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(real,rshift,owca_int,owca_int)
		{
			if (p1<0) {
				vm->raise_invalid_integer(OWCA_ERROR_FORMAT("cant right shift by a negative amount"));
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			return_value->set_int(self >> p1);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(real,mod,owca_int,owca_int)
		{
			if (p1==0) {
				vm->raise_division_by_zero();
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			return_value->set_int(self % p1);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(real,binand,owca_int,owca_int)
		{
			return_value->set_int(self & p1);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(real,binor,owca_int,owca_int)
		{
			return_value->set_int(self | p1);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(real,binxor,owca_int,owca_int)
		{
			return_value->set_int(self ^ p1);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	void virtual_machine::initialize_real(internal_class &c)
	{
		c._setinheritable(false);

		M_OPER1(c,real,new,(*this,"$new","value",dv_no_param_given));
		M_OPER0(c,real,str,(*this,"$str"));
		M_OPER0(c,real,bool,(*this,"$bool"));
		M_OPER0(c,real,hash,(*this,"$hash"));
		M_OPER0(c,real,binnot,(*this,"$binnot"));
		M_OPER0(c,real,sign,(*this,"$sign"));

		M_OPER1(c,real,add,(*this,"$add","other"));
		M_OPER1(c,real,sub,(*this,"$sub","other"));
		M_OPER1(c,real,mul,(*this,"$mul","other"));
		M_OPER1(c,real,div,(*this,"$div","other"));

		M_OPER1(c,real,eq,(*this,"$eq","other"));
		M_OPER1(c,real,noteq,(*this,"$noteq","other"));
		M_OPER1(c,real,less,(*this,"$less","other"));
		M_OPER1(c,real,more,(*this,"$more","other"));
		M_OPER1(c,real,lesseq,(*this,"$lesseq","other"));
		M_OPER1(c,real,moreeq,(*this,"$moreeq","other"));

		M_OPER1(c,real,lshift,(*this,"$lshift","other"));
		M_OPER1(c,real,rshift,(*this,"$rshift","other"));
		M_OPER1(c,real,mod,(*this,"$mod","other"));
		M_OPER1(c,real,binand,(*this,"$binand","other"));
		M_OPER1(c,real,binor,(*this,"$binor","other"));
		M_OPER1(c,real,binxor,(*this,"$binxor","other"));
	}
} }












