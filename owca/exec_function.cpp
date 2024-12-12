#include "stdafx.h"
#include "base.h"
#include "exec_class_int.h"
#include "exec_function_ptr.h"
#include "virtualmachine.h"
#include "op_compare.h"
#include "exec_string.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_external.h"
#include "exec_object.h"

namespace owca { namespace __owca__ {

	std::string str_function(exec_function_ptr *f);

	D_SELF0(function,bool,unifunction)
		{
			return_value->set_bool(true);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(function,str,unifunction)
		{
			return_value->set_string(vm->allocate_string("function "+str_function(self.fnc())));
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(function,eq,unifunction,unifunction)
		{
			switch(mode) {
			CASE(0)
				if (self.fnc()==p1.fnc()) {
					params[0]=self.slf();
					params[1]=p1.slf();
					CALC(1,vm->calculate_eq(return_value,params));
				}
				return_value->set_bool(false);
				break;
			CASE(1)
				break;
			default:
				RCASSERT(0);
			}
			return executionstackreturnvalue::RETURN;
		}
		exec_variable tmp;
		exec_variable params[2];
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	D_SELF1(function,noteq,unifunction,unifunction)
		{
			switch(mode) {
			CASE(0)
				if (self.fnc()==p1.fnc()) {
					params[0]=self.slf();
					params[1]=p1.slf();
					CALC(1,vm->calculate_eq(return_value,params));
				}
				return_value->set_bool(true);
				break;
			CASE(1)
				return_value->set_bool(!return_value->get_bool());
				break;
			default:
				RCASSERT(0);
			}
			return executionstackreturnvalue::RETURN;
		}
		exec_variable tmp;
		exec_variable params[2];
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	D_SELF0(function,object,unifunction)
		{
			*return_value=self.slf();
			return_value->gc_acquire();
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(function,bind,unifunction,const exec_variable*)
		{
			return_value->set_function_s(*vm,*p1,self.fnc());
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(function,hash,unifunction)
		{
			switch(mode) {
			CASE(0)
				CALC(1,vm->calculate_hash(return_value,self.slf()));
			CASE(1)
				return_value->set_int(return_value->get_int() ^ reinterpret_cast<owca_int>(self.fnc()));
				break;
			default:
				RCASSERT(0);
			}
			return executionstackreturnvalue::RETURN;
		}
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	D_SELF0(function,name,unifunction)
		{
			return_value->set_string(self.fnc()->name());
			return_value->get_string()->gc_acquire();
			return executionstackreturnvalue::RETURN;
		}
	D_END
	D_SELF0(function,member_of,unifunction)
		{
			return_value->set_object_null(self.fnc()->classobject());
			if (return_value->mode()==VAR_OBJECT) return_value->get_object()->gc_acquire();
			return executionstackreturnvalue::RETURN;
		}
	D_END
	D_SELF0(function,filename,unifunction)
		{
			return_value->set_string(vm->allocate_string(self.fnc()->declared_filename()));
			return executionstackreturnvalue::RETURN;
		}
	D_END
 	D_SELF0(function,fileline,unifunction)
		{
			return_value->set_int(self.fnc()->declared_file_line());
			return executionstackreturnvalue::RETURN;
		}
	D_END

	void virtual_machine::initialize_function(internal_class &c)
	{
		c._setinheritable(false);
		c._setconstructable(false);

		M_OPER0(c,function,bool,(*this,"$bool"));
		M_OPER0(c,function,str,(*this,"$str"));
		M_OPER0(c,function,hash,(*this,"$hash"));
		M_OPER1(c,function,eq,(*this,"$eq","other"));
		M_OPER1(c,function,noteq,(*this,"$noteq","other"));

		M_FUNC0(c,function,object,(*this,"object"));
		M_FUNC1(c,function,bind,(*this,"bind","slf"));

		M_FUNC0(c,function,name,(*this,"name"));
		M_FUNC0(c,function,member_of,(*this,"member_of"));
		M_FUNC0(c,function,filename,(*this,"file_name"));
		M_FUNC0(c,function,fileline,(*this,"file_line"));
	}
} }












