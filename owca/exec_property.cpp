#include "stdafx.h"
#include "base.h"
#include "exec_property.h"
#include "exec_class_int.h"
#include "exec_function_ptr.h"
#include "virtualmachine.h"
#include "op_compare.h"
#include "vm_execution_stack_elem_external.h"

namespace owca { namespace __owca__ {

	void exec_property::_mark_gc(const gc_iteration &gc) const
	{
		if (read) {
			gc_iteration::debug_info _d("exec_property: read function");
			read->gc_mark(gc);
		}
		if (write) {
			gc_iteration::debug_info _d("exec_property: write function");
			write->gc_mark(gc);
		}
	}

	void exec_property::_release_resources(virtual_machine &vm)
	{
		if (read) read->gc_release(vm);
		if (write) write->gc_release(vm);
	}

	static bool get_function(exec_function_ptr *&dst, virtual_machine *vm, const std::string &name, const exec_variable *v)
	{
		switch(v->mode()) {
		case VAR_FUNCTION:
			if (dst) dst->gc_release(*vm);
			dst=v->get_function()->function();
			dst->gc_acquire();
			break;
		case VAR_FUNCTION_FAST:
			if (dst) dst->gc_release(*vm);
			dst=v->get_function_fast().fnc;
			dst->gc_acquire();
			break;
		case VAR_NULL:
		case VAR_NO_PARAM_GIVEN:
			if (dst) dst->gc_release(*vm);
			dst=NULL;
			break;
		default:
			vm->raise_invalid_param(name,vm->class_function);
			return false;
		}
		return true;
	}

	D_FUNC2(property,new,const exec_variable *,const exec_variable *)
		{
			exec_property *p=vm->allocate_property();
			if (!get_function(p->read,vm,"read",p1)) {
				p->read=p->write=NULL;
				p->gc_release(*vm);
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			if (!get_function(p->write,vm,"write",p2)) {
				p->write=NULL;
				p->gc_release(*vm);
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			return_value->set_property(p);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(property,str,exec_property*)
		{
			return_value->set_string(vm->allocate_string("property"));
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(property,bool,exec_property*)
		{
			return_value->set_bool(true);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(property,read_read,exec_property*)
		{
			if (self->read) {
				return_value->set_function_fast(self->read);
				return_value->gc_acquire();
			}
			else return_value->set_null();
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(property,read_write,exec_property*,const exec_variable*)
		{
			if (!get_function(self->read,vm,"value",p1)) return executionstackreturnvalue::FUNCTION_CALL;
			*return_value=*p1;
			p1->gc_acquire();
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(property,write_read,exec_property*)
		{
			if (self->write) {
				return_value->set_function_fast(self->write);
				return_value->gc_acquire();
			}
			else return_value->set_null();
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(property,write_write,exec_property*,const exec_variable*)
		{
			if (!get_function(self->write,vm,"value",p1)) return executionstackreturnvalue::FUNCTION_CALL;
			*return_value=*p1;
			p1->gc_acquire();
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(property,eq,exec_property*,exec_property*)
		{
			return_value->set_bool(self->read==p1->read && self->write==p1->write);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(property,noteq,exec_property*,exec_property*)
		{
			return_value->set_bool(self->read!=p1->read || self->write!=p1->write);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	void virtual_machine::initialize_property(internal_class &c)
	{
		c._setinheritable(false);

		M_OPER2(c,property,new,(*this,"$new","read","write",dv_no_param_given,dv_no_param_given));
		M_OPER0(c,property,str,(*this,"$str"));
		M_OPER0(c,property,bool,(*this,"$bool"));
		M_OPER1(c,property,eq,(*this,"$eq","other"));
		M_OPER1(c,property,noteq,(*this,"$noteq","other"));

		M_PROP_RW(c,property,read,(*this));
		M_PROP_RW(c,property,write,(*this));
	}
} }












