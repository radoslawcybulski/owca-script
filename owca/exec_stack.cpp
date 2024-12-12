#include "stdafx.h"
#include "base.h"
#include "exec_class_int.h"
#include "exec_stack.h"
#include "exec_string.h"
#include "virtualmachine.h"
#include "exec_function_ptr.h"
#include "namespace.h"
#include "exec_namespace.h"
#include "vm_execution_stack_elem_internal.h"
#include "exec_string_compare.h"
#include "exec_array_object.h"

namespace owca { namespace __owca__ {

	exec_stack::exec_stack(exec_namespace *ns_, unsigned int *level0map_): size_(0),ns(ns_),level0map(level0map_)
	{
		RCASSERT(level0map[0]==1);
#ifdef RCDEBUG
		stackvariables=NULL;
#endif
	}

	unsigned int exec_stack::calculate_size(exec_function_ptr *fnc)
	{
		return sizeof(exec_stack)+sizeof(exec_stack_variables*)*(fnc->internal_stack_variables_count()+1);
	}

	unsigned int exec_stack::calculate_size(exec_stack *prev)
	{
		return sizeof(exec_stack)+sizeof(exec_stack_variables*)*(prev->size()+1);
	}

	exec_stack::exec_stack(exec_function_ptr *fnc) : size_(fnc->internal_stack_variables_count()+1),ns(fnc->ynamespace()),level0map(fnc->internal_stack_variables_level0Map())
	{
		level0map[0]++;
#ifdef RCDEBUG
		stackvariables=((exec_stack_variables**)((char*)this+sizeof(*this)));
#endif
		for(unsigned int i=1;i<size_;++i) {
			exec_stack_variables *q=fnc->internal_stack_variables(i-1);
			q->gc_acquire();
			set_variables(i,q);
		}
	}

	exec_stack::exec_stack(exec_stack *prev) : size_(prev->size()+1),ns(prev->ns),level0map(prev->level0map)
	{
		level0map[0]++;
#ifdef RCDEBUG
		stackvariables=((exec_stack_variables**)((char*)this+sizeof(*this)));
#endif
		for(unsigned int i=1;i<size_;++i) {
			exec_stack_variables *q=prev->get_variables(i);
			q->gc_acquire();
			set_variables(i,q);
		}
	}

	exec_variable &exec_stack::get_0(unsigned int index)
	{
		return ns->variables->get(level0map[index+1]);
	}

	unsigned int exec_stack_variables::calculate_size(unsigned int size)
	{
		return sizeof(exec_stack_variables)+sizeof(exec_variable)*size;
	}

	exec_stack_variables::exec_stack_variables(unsigned int size, exec_function_ptr *own) : size_(size),owner_(own)
	{
		if (owner_) owner_->gc_acquire();
	}

	void exec_stack_variables::_mark_gc(const gc_iteration &gc) const
	{
		if (owner_) {
			gc_iteration::debug_info _d("exec_stack_variables %x: owner object",this);
			owner_->gc_mark(gc);
		}
		for(unsigned int i=0;i<size_;++i) {
			gc_iteration::debug_info _d("exec_stack_variables %x: variable %d",this,i);
			get(i).gc_mark(gc);
		}
	}

	void exec_stack_variables::_release_resources(virtual_machine &vm)
	{
		exec_variable *dbg=size_>0 ? &get(0) : NULL;

		if (owner_) owner_->gc_release(vm);
		for(unsigned int i=0;i<size_;++i) {
			get(i).gc_release(vm);
		}
	}

	void exec_stack::_mark_gc(const gc_iteration &gc) const
	{
		for(unsigned int i=1;i<=size_;++i) {
			gc_iteration::debug_info _d("exec_stack %x: depth %d",this,i);
			get_variables(i)->gc_mark(gc);
		}
	}

	void exec_stack::_release_resources(virtual_machine &vm)
	{
		for(unsigned int i=1;i<=size_;++i) {
			get_variables(i)->gc_release(vm);
		}

		if (--level0map[0]==0) delete [] level0map;
	}

	void exec_stack_element_object::init(void)
	{
		if (!inited) {
			inited=true;
			std::map<owca_internal_string*,exec_variable*,string_compare> mp;

			switch(elem->fnc->mode()) {
			case exec_function_ptr::F_INTERNAL: {
				vm_execution_stack_elem_internal *el=dynamic_cast<vm_execution_stack_elem_internal*>(elem);

				for(exec_function_ptr *f=elem->fnc;f;f=f->internal_parent_function()) {
					for(unsigned int i=0;i<f->internal_param_count();++i) {
						const exec_function_ptr::internal_variable_info &var=f->internal_variable(i);

						mp.insert(std::pair<owca_internal_string*,exec_variable*>(var.ident,&el->stack->get_variable(var.location)));
					}
				}
				break; }
			case exec_function_ptr::F_SELF:
			case exec_function_ptr::F_SELF_0:
			case exec_function_ptr::F_SELF_1:
			case exec_function_ptr::F_SELF_2:
			case exec_function_ptr::F_SELF_3:
			case exec_function_ptr::F_FAST:
			case exec_function_ptr::F_FAST_0:
			case exec_function_ptr::F_FAST_1:
			case exec_function_ptr::F_FAST_2:
			case exec_function_ptr::F_FAST_3: {
				break; }
			default:
				RCASSERT(0);
			}
			RCASSERT(elements.empty());
			elements.reserve(mp.size());
			for(std::map<owca_internal_string*,exec_variable*,string_compare>::iterator it=mp.begin();it!=mp.end();++it) {
				elements.push_back(valueelem());
				elements.back().ident=it->first;
				elements.back().value=it->second;
			}
		}
	}

	void exec_stack_element_object::set_element(vm_execution_stack_elem_base *e)
	{
		RCASSERT(elem==NULL);
		e->gc_acquire();
		elem=e;
		elements.clear();
		inited=false;
	}

	//void exec_stack_element_object::_create(virtual_machine &, unsigned int oversize) { }
	void exec_stack_element_object::_marker(const gc_iteration &gc) const
	{
		gc_iteration::debug_info _d("exec_stack_element_object %x: elem object",this);
		elem->gc_mark(gc);
	}
	void exec_stack_element_object::_destroy(virtual_machine &vm) { elem->gc_release(vm); }

	D_SELF0(stack_element,str,exec_stack_element_object*)
		{
			return_value->set_string(vm->allocate_string("stack element"));
			return executionstackreturnvalue::RETURN;
		}
	D_END
	D_SELF0(stack_element,function,exec_stack_element_object*)
		{
			if (self->element()->fnc) {
				return_value->set_function_fast(self->element()->fnc);
				return_value->get_function_fast().fnc->gc_acquire();
			}
			else return_value->set_null();
			return executionstackreturnvalue::RETURN;
		}
	D_END
	D_SELF0(stack_element,file_name,exec_stack_element_object*)
		{
			vm_execution_stack_elem_internal *s=dynamic_cast<vm_execution_stack_elem_internal*>(self->element());
			if (s) {
				owca_location l=s->actual_location();
				return_value->set_string(vm->allocate_string(l.filename()));
			}
			else 
                return_value->set_string(vm->allocate_string(NULL,0,0));
			return executionstackreturnvalue::RETURN;
		}
	D_END
	D_SELF0(stack_element,file_line,exec_stack_element_object*)
		{
			vm_execution_stack_elem_internal *s=dynamic_cast<vm_execution_stack_elem_internal*>(self->element());
			if (s) {
				owca_location l=s->actual_location();
				return_value->set_int(l.line());
			}
			else return_value->set_null();
			return executionstackreturnvalue::RETURN;
		}
	D_END


	D_SELF0(stack_element,identificators,exec_stack_element_object*)
		{
			exec_array_object *oo;
			return_value->set_object(vm->allocate_array(oo));

			init();
			oo->resize(*vm, (unsigned int)self->elements.size());

			for(unsigned int i=0;i<(unsigned int)self->elements.size();++i) {
				oo->get(i).set_string(self->elements[i].ident);
				self->elements[i].ident->gc_acquire();
			}

			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(stack_element,values,exec_stack_element_object*)
		{
			exec_array_object *oo;
			return_value->set_object(vm->allocate_array(oo));

			init();
			oo->resize(*vm, (unsigned int)self->elements.size());

			for(unsigned int i=0;i<(unsigned int)self->elements.size();++i) {
				oo->get(i)=*self->elements[i].value;
				oo->get(i).gc_acquire();
			}

			return executionstackreturnvalue::RETURN;
		}
	D_END

	void virtual_machine::initialize_stack_element(internal_class &c)
	{
		c._setstructure<exec_stack_element_object>();
		c._setconstructable(false);

		M_OPER0(c,stack_element,str,(*this,"$str"));
		M_FUNC0(c,stack_element,function,(*this,"function"));
		M_FUNC0(c,stack_element,file_name,(*this,"file_name"));
		M_FUNC0(c,stack_element,file_line,(*this,"file_line"));
		M_FUNC0(c,stack_element,identificators,(*this,"identificators"));
		M_FUNC0(c,stack_element,values,(*this,"values"));
		//M_FUNC0(c,stack_element,get_value,(*this,"get_value","identificator"));
		//M_FUNC0(c,stack_element,set_value,(*this,"set_value","identificator","value"));
	}

} }












