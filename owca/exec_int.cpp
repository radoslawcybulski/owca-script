#include "stdafx.h"
#include "base.h"
#include "exec_string.h"
#include "exec_class_int.h"
#include "virtualmachine.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_external.h"

namespace owca { namespace __owca__ {

	D_FUNC2(int,new,const exec_variable*,owca_int)
		{
			if (p2<0 || p2==1 || p2>10+'z'-'a'+1) {
				vm->raise_invalid_integer(OWCA_ERROR_FORMAT1("%1 is an invalid base number",int_to_string(p2)));
				return executionstackreturnvalue::FUNCTION_CALL;
			}

			if (p2!=10 && p2!=0 && p1->mode()!=VAR_STRING) {
				vm->raise_invalid_param(OWCA_ERROR_FORMAT("unexpected parameter base"));
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			switch(p1->mode()) {
			case VAR_NO_PARAM_GIVEN: return_value->set_int(0); return executionstackreturnvalue::RETURN;
			case VAR_INT: return_value->set_int(p1->get_int()); return executionstackreturnvalue::RETURN;
			case VAR_REAL: return_value->set_int((owca_int)p1->get_real()); return executionstackreturnvalue::RETURN;
			case VAR_BOOL: return_value->set_int(p1->get_bool() ? 1 : 0); return executionstackreturnvalue::RETURN;
			case VAR_STRING: {
				const char *txt=p1->get_string()->data_pointer();
				unsigned int size=p1->get_string()->data_size();

				while(size>0 && isspace(txt[0])) {
					++txt;
					--size;
				}
				while(size>0 && isspace(txt[size-1])) --size;
				if (size==0) goto invalid_str;
				{
					bool neg=false;
					if (txt[0]=='-' || txt[0]=='+') {
						if (size==1 || isspace(txt[1])) goto invalid_str;
						if (txt[0]=='-') neg=true;
						++txt;
						--size;
					}

					if (p2==0) {
						if (size>2 && txt[0]=='0' && !isspace(txt[2])) {
							if (txt[1]=='x' || txt[1]=='X') p2=16;
							else if (txt[1]=='o' || txt[1]=='O') p2=8;
							else if (txt[1]=='b' || txt[1]=='B') p2=2;
						}
						if (p2) {
							txt+=2;
							size-=2;
						}
						else p2=10;
					}
					else if (size>2 && txt[0]=='0' && !isspace(txt[2])) {
						if ((p2==16 && (txt[1]=='x' || txt[1]=='X')) ||
							(p2== 8 && (txt[1]=='o' || txt[1]=='O')) ||
							(p2== 2 && (txt[1]=='b' || txt[1]=='B'))) {
							txt+=2;
							size-=2;
						}
					}
					owca_int self;
					if (!to_int(self,txt,size,(unsigned int)p2)) goto invalid_str;
					return_value->set_int(neg ? -self : self);
					return executionstackreturnvalue::RETURN;
				}
	invalid_str:
				vm->raise_invalid_param(OWCA_ERROR_FORMAT1("'%1' is not a valid string representation of an integer value",txt));
				return executionstackreturnvalue::FUNCTION_CALL; }
			default:
				vm->raise_invalid_param(OWCA_ERROR_FORMAT1("cant convert %1 to an integer",vm->to_stdstring_type(*p1)));
				return executionstackreturnvalue::FUNCTION_CALL;
			}
		}
	D_END

	D_SELF0(int,str,owca_int)
		{
			return_value->set_string(vm->allocate_string(int_to_string(self)));
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(int,bool,owca_int)
		{
			return_value->set_bool(self!=0);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(int,binnot,owca_int)
		{
			return_value->set_int(~self);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(int,sign,owca_int)
		{
			return_value->set_int(-self);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(int,add,owca_int,const exec_variable*)
		{
			switch(p1->mode()) {
			case VAR_INT: return_value->set_int(self+p1->get_int()); break;
			case VAR_REAL: return_value->set_real(self+p1->get_real()); break;
			default:
				return executionstackreturnvalue::RETURN_NO_VALUE;
			}
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(int,sub,owca_int,const exec_variable*)
		{
			switch(p1->mode()) {
			case VAR_INT: return_value->set_int(self-p1->get_int()); break;
			case VAR_REAL: return_value->set_real(self-p1->get_real()); break;
			default:
				return executionstackreturnvalue::RETURN_NO_VALUE;
			}
			return executionstackreturnvalue::RETURN;
		}
	D_END

	owca_internal_string *string_mul(virtual_machine *vm, owca_internal_string *s, unsigned int mul);

	D_SELF1(int,mul,owca_int,const exec_variable*)
		{
			switch(p1->mode()) {
			case VAR_INT: return_value->set_int(self*p1->get_int()); break;
			case VAR_REAL: return_value->set_real(self*p1->get_real()); break;
			case VAR_STRING:
				if (self<0) {
					vm->raise_invalid_integer(OWCA_ERROR_FORMAT("cant multiply by negative amount"));
					return executionstackreturnvalue::FUNCTION_CALL;
				}
				return_value->set_string(string_mul(vm,p1->get_string(),(unsigned int)self));
				break;
			default:
				return executionstackreturnvalue::RETURN_NO_VALUE;
			}
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(int,div,owca_int,const exec_variable*)
		{
			switch(p1->mode()) {
			case VAR_INT:
				if (p1->get_int()!=0) {
					if ((self % p1->get_int())==0) return_value->set_int(self/p1->get_int());
					else return_value->set_real((owca_real)self/p1->get_int());
				}
				else {
					vm->raise_division_by_zero();
					return executionstackreturnvalue::FUNCTION_CALL;
				}
				break;
			case VAR_REAL:
				if (p1->get_real()!=0) return_value->set_real(self/p1->get_real());
				else {
					vm->raise_division_by_zero();
					return executionstackreturnvalue::FUNCTION_CALL;
				}
				break;
			default:
				return executionstackreturnvalue::RETURN_NO_VALUE;
			}
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(int,lshift,owca_int,owca_int)
		{
			if (p1<0) {
				vm->raise_invalid_integer(OWCA_ERROR_FORMAT("cant left shift by a negative amount"));
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			return_value->set_int(self << p1);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(int,rshift,owca_int,owca_int)
		{
			if (p1<0) {
				vm->raise_invalid_integer(OWCA_ERROR_FORMAT("cant right shift by a negative amount"));
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			return_value->set_int(self >> p1);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(int,mod,owca_int,owca_int)
		{
			if (p1==0) {
				vm->raise_division_by_zero();
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			return_value->set_int(self % p1);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(int,binand,owca_int,owca_int)
		{
			return_value->set_int(self & p1);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(int,binor,owca_int,owca_int)
		{
			return_value->set_int(self | p1);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(int,binxor,owca_int,owca_int)
		{
			return_value->set_int(self ^ p1);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(int,eq,owca_int,const exec_variable*)
		{
			switch(p1->mode()) {
			case VAR_INT: return_value->set_bool(self==p1->get_int()); break;
			case VAR_REAL: return_value->set_bool(self==p1->get_real()); break;
			default:
				return executionstackreturnvalue::RETURN_NO_VALUE;
			}
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(int,noteq,owca_int,const exec_variable*)
		{
			switch(p1->mode()) {
			case VAR_INT: return_value->set_bool(self!=p1->get_int()); break;
			case VAR_REAL: return_value->set_bool(self!=p1->get_real()); break;
			default:
				return executionstackreturnvalue::RETURN_NO_VALUE;
			}
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(int,less,owca_int,const exec_variable*)
		{
			switch(p1->mode()) {
			case VAR_INT: return_value->set_bool(self<p1->get_int()); break;
			case VAR_REAL: return_value->set_bool(self<p1->get_real()); break;
			default:
				return executionstackreturnvalue::RETURN_NO_VALUE;
			}
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(int,more,owca_int,const exec_variable*)
		{
			switch(p1->mode()) {
			case VAR_INT: return_value->set_bool(self>p1->get_int()); break;
			case VAR_REAL: return_value->set_bool(self>p1->get_real()); break;
			default:
				return executionstackreturnvalue::RETURN_NO_VALUE;
			}
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(int,lesseq,owca_int,const exec_variable*)
		{
			switch(p1->mode()) {
			case VAR_INT: return_value->set_bool(self<=p1->get_int()); break;
			case VAR_REAL: return_value->set_bool(self<=p1->get_real()); break;
			default:
				return executionstackreturnvalue::RETURN_NO_VALUE;
			}
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(int,moreeq,owca_int,const exec_variable*)
		{
			switch(p1->mode()) {
			case VAR_INT: return_value->set_bool(self>=p1->get_int()); break;
			case VAR_REAL: return_value->set_bool(self>=p1->get_real()); break;
			default:
				return executionstackreturnvalue::RETURN_NO_VALUE;
			}
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(int,hash,owca_int)
		{
			return_value->set_int(self);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	void virtual_machine::initialize_int(internal_class &c)
	{
		c._setinheritable(false);

		M_OPER2(c,int,new,(*this,"$new","value","base",dv_no_param_given,0));
		M_OPER0(c,int,str,(*this,"$str"));
		M_OPER0(c,int,bool,(*this,"$bool"));
		M_OPER0(c,int,hash,(*this,"$hash"));
		M_OPER0(c,int,binnot,(*this,"$binnot"));
		M_OPER0(c,int,sign,(*this,"$sign"));

		M_OPER1(c,int,add,(*this,"$add","other"));
		M_OPER1(c,int,sub,(*this,"$sub","other"));
		M_OPER1(c,int,mul,(*this,"$mul","other"));
		M_OPER1(c,int,div,(*this,"$div","other"));

		M_OPER1(c,int,lshift,(*this,"$lshift","other"));
		M_OPER1(c,int,rshift,(*this,"$rshift","other"));
		M_OPER1(c,int,mod,(*this,"$mod","other"));
		M_OPER1(c,int,binand,(*this,"$binand","other"));
		M_OPER1(c,int,binor,(*this,"$binor","other"));
		M_OPER1(c,int,binxor,(*this,"$binxor","other"));

		M_OPER1(c,int,eq,(*this,"$eq","other"));
		M_OPER1(c,int,noteq,(*this,"$noteq","other"));
		M_OPER1(c,int,lesseq,(*this,"$lesseq","other"));
		M_OPER1(c,int,moreeq,(*this,"$moreeq","other"));
		M_OPER1(c,int,less,(*this,"$less","other"));
		M_OPER1(c,int,more,(*this,"$more","other"));
	}
} }












