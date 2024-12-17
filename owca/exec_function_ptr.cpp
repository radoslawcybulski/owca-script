#include "stdafx.h"
#include "base.h"
#include "virtualmachine.h"
#include "exec_function_ptr.h"
#include "exec_string.h"
#include "exec_stack.h"
#include "returnvalue.h"
#include "exec_variable.h"
#include "op_execute.h"
#include "defval.h"
#include "vm_execution_stack_elem_internal.h"
#include "exec_stack.h"
#include "exec_object.h"
#include "exec_function_ptr.h"
#include "exec_property.h"
#include "exec_namespace.h"
#include "namespace.h"
#include "op_validate.h"

namespace owca { namespace __owca__ {

	struct function_internal {
		exec_function_ptr::internal_param_info *params;
		exec_function_ptr::internal_variable_info *variables;
		exec_stack_variables **stackvariables;

		exec_function_ptr::internal_param_info_nodef list,map;
		exec_variable_location selflocation;
		opcode_data *opcodes;
		vm_execution_stack_elem_internal_jump codebegin;
		unsigned int paramcount,variablecount;
		unsigned int maxyielddatasize;
		unsigned int temporaryvariablescount;
		unsigned int localstacksize;
		unsigned int stack_variables_count;
		exec_function_ptr::selftype stype;
		exec_function_ptr::functiontype ftype;
		unsigned int *level0map;
		unsigned char is_generator;
		exec_function_ptr *parent;
	};

	void exec_function_ptr::set_namespace(exec_namespace *n)
	{
		RCASSERT(nspace_ == NULL);
		nspace_ = n;
		n->gc_acquire();
	}

	exec_function_ptr *exec_function_ptr::internal_allocate(virtual_machine &vm, owca_internal_string *name, unsigned char is_generator, opcode_data *opcodes,
		const exec_variable_location &selflocation,
		vm_execution_stack_elem_internal_jump *&begin, internal_param_info *&params, internal_variable_info *&variables,
		internal_param_info_nodef *&list_param, internal_param_info_nodef *&map_param,
		unsigned int paramcount, unsigned int varcount, unsigned int localstacksize, selftype stype, functiontype ftype,
		unsigned int maxyielddatasize, unsigned int temporaryvariablescount, exec_stack *st, exec_function_ptr *parentfnc)
	{
		unsigned int size = sizeof(exec_function_ptr)+sizeof(function_internal)+sizeof(internal_variable_info)*varcount + sizeof(internal_param_info)*paramcount + sizeof(exec_stack_variables*)*st->size();
		exec_function_ptr *p = vm.allocate_function_ptr(size);

		p->set_namespace(st->ns);
		p->name_ = name;
		p->mode_ = F_INTERNAL;
		function_internal *fi = (function_internal*)(((char*)p) + sizeof(*p));

		fi->parent = parentfnc;
		if (parentfnc) parentfnc->gc_acquire();
		fi->params = params = (internal_param_info*)(((char*)fi) + sizeof(*fi));
		fi->paramcount = paramcount;
		fi->variables = variables = (internal_variable_info*)(((char*)params) + sizeof(internal_param_info)*paramcount);
		fi->variablecount = varcount;
		fi->stackvariables = (exec_stack_variables**)(((char*)variables) + sizeof(internal_variable_info)*varcount);

		fi->opcodes = opcodes;

		begin = &fi->codebegin;
		fi->is_generator = is_generator;
		fi->list.ident = NULL;
		fi->map.ident = NULL;
		list_param = &fi->list;
		map_param = &fi->map;
		fi->paramcount = paramcount;
		fi->localstacksize = localstacksize;
		fi->maxyielddatasize = maxyielddatasize;
		fi->temporaryvariablescount = temporaryvariablescount;
		fi->selflocation = selflocation;
		fi->stype = stype;
		fi->ftype = ftype;

		fi->stack_variables_count = st->size();
		//exec_stack_variables **pp=(exec_stack_variables **)(((char*)p)+sizeof(*p)+sizeof(function_internal)+sizeof(internal_param_info)*paramcount);
		for (unsigned int i = 1; i <= st->size(); ++i) (fi->stackvariables[i - 1] = st->get_variables(i))->gc_acquire();
		fi->level0map = st->level0map;
		++fi->level0map[0];

#ifdef RCDEBUG
		p->_external_param = NULL;
		p->_external = NULL;
		p->_internal = fi;
#endif
		return p;
	}

	exec_function_ptr *exec_function_ptr::internal_parent_function() const
	{
		function_internal *fi=(function_internal*)(((char*)this)+sizeof(*this));
		return fi->parent;
	}

	exec_function_ptr::selftype exec_function_ptr::internal_self_type() const
	{
		function_internal *fi=(function_internal*)(((char*)this)+sizeof(*this));
		return fi->stype;
	}

	exec_function_ptr::functiontype exec_function_ptr::internal_function_type() const
	{
		function_internal *fi=(function_internal*)(((char*)this)+sizeof(*this));
		return fi->ftype;
	}

	const exec_variable_location &exec_function_ptr::internal_self_location() const
	{
		function_internal *fi=(function_internal*)(((char*)this)+sizeof(*this));
		return fi->selflocation;
	}

	const exec_function_ptr::internal_param_info_nodef &exec_function_ptr::internal_map_param() const
	{
		function_internal *fi=(function_internal*)(((char*)this)+sizeof(*this));
		return fi->map;
	}

	const exec_function_ptr::internal_param_info_nodef &exec_function_ptr::internal_list_param() const
	{
		function_internal *fi=(function_internal*)(((char*)this)+sizeof(*this));
		return fi->list;
	}

	unsigned int exec_function_ptr::internal_param_count() const
	{
		function_internal *fi=(function_internal*)(((char*)this)+sizeof(*this));
		return fi->paramcount;
	}

	const exec_function_ptr::internal_param_info &exec_function_ptr::internal_param(unsigned int index) const
	{
		RCASSERT(index<internal_param_count());
		function_internal *fi=(function_internal*)(((char*)this)+sizeof(*this));
		return fi->params[index];
	}

	unsigned int exec_function_ptr::internal_variable_count() const
	{
		function_internal *fi=(function_internal*)(((char*)this)+sizeof(*this));
		return fi->variablecount;
	}
	const exec_function_ptr::internal_variable_info &exec_function_ptr::internal_variable(unsigned int index) const
	{
		function_internal *fi=(function_internal*)(((char*)this)+sizeof(*this));
		RCASSERT(index<fi->variablecount);
		return fi->variables[index];
	}

	unsigned int exec_function_ptr::internal_temporary_variables_count() const
	{
		function_internal *fi=(function_internal*)(((char*)this)+sizeof(*this));
		return fi->temporaryvariablescount;
	}

	unsigned int exec_function_ptr::internal_stack_yield_size() const
	{
		function_internal *fi=(function_internal*)(((char*)this)+sizeof(*this));
		return fi->maxyielddatasize;
	}

	unsigned char exec_function_ptr::internal_is_generator() const
	{
		function_internal *fi=(function_internal*)(((char*)this)+sizeof(*this));
		return fi->is_generator;
	}

	opcode_data *exec_function_ptr::internal_opcodes() const
	{
		function_internal *fi=(function_internal*)(((char*)this)+sizeof(*this));
		return fi->opcodes;
	}

	vm_execution_stack_elem_internal_jump exec_function_ptr::internal_jump_start() const
	{
		function_internal *fi=(function_internal*)(((char*)this)+sizeof(*this));
		return fi->codebegin;
	}

	unsigned int exec_function_ptr::internal_local_stack_size() const
	{
		function_internal *fi=(function_internal*)(((char*)this)+sizeof(*this));
		return fi->localstacksize;
	}

	unsigned int exec_function_ptr::internal_stack_variables_count() const
	{
		function_internal *fi=(function_internal*)(((char*)this)+sizeof(*this));
		return fi->stack_variables_count;
	}

	unsigned int *exec_function_ptr::internal_stack_variables_level0Map() const
	{
		function_internal *fi=(function_internal*)(((char*)this)+sizeof(*this));
		return fi->level0map;
	}

	exec_stack_variables *exec_function_ptr::internal_stack_variables(unsigned int index) const
	{
		function_internal *fi=(function_internal*)(((char*)this)+sizeof(*this));
		RCASSERT(index<fi->stack_variables_count);
		return fi->stackvariables[index];
		//exec_stack_variables **pp=(exec_stack_variables **)(((char*)this)+sizeof(*this)+sizeof(function_internal)+sizeof(internal_param_info)*fi->paramcount);
		//return pp[index];
	}

	//exec_function_ptr::external_constructor exec_function_ptr::external_constructor_function() const
	//{
	//	function_external *fe=(function_external*)(((char*)this)+sizeof(*this));
	//	return fe->func;
	//}

	//const void *exec_function_ptr::external_data() const
	//{
	//	function_external *fe=(function_external*)(((char*)this)+sizeof(*this));
	//	return fe->fptr;
	//}





	struct function_external_param {
		owca_internal_string *ident;
		exec_variable value;
	};
	struct function_external {
		exec_function_ptr::functionptr fptr;
		const void *userptr;
	};

	exec_function_ptr::functionptr exec_function_ptr::external_function() const
	{
		function_external *fe=(function_external*)(((char*)this)+sizeof(*this));
		return fe->fptr;
	}

	const void *exec_function_ptr::external_user_pointer() const
	{
		function_external *fe=(function_external*)(((char*)this)+sizeof(*this));
		return fe->userptr;
	}

	owca_internal_string *exec_function_ptr::external_param_name(unsigned int index) const
	{
		function_external *fe=(function_external*)(((char*)this)+sizeof(*this));
		function_external_param *fep=(function_external_param*)(((char*)fe)+sizeof(*fe));
		return fep[index].ident;
	}

	void exec_function_ptr::external_set_param_name(unsigned int index, owca_internal_string *n)
	{
		function_external *fe=(function_external*)(((char*)this)+sizeof(*this));
		function_external_param *fep=(function_external_param*)(((char*)fe)+sizeof(*fe));
		fep[index].ident=n;
		n->gc_acquire();
	}

	const exec_variable &exec_function_ptr::external_param_default_value(unsigned int index) const
	{
		function_external *fe=(function_external*)(((char*)this)+sizeof(*this));
		function_external_param *fep=(function_external_param*)(((char*)fe)+sizeof(*fe));
		return fep[index].value;
	}

	exec_variable &exec_function_ptr::external_set_param_default_value(unsigned int index)
	{
		function_external *fe=(function_external*)(((char*)this)+sizeof(*this));
		function_external_param *fep=(function_external_param*)(((char*)fe)+sizeof(*fe));
		return fep[index].value;
	}

	exec_function_ptr::functionptr &exec_function_ptr::external_set_function(const void *ptr_)
	{
		function_external *fe=(function_external*)(((char*)this)+sizeof(*this));
		fe->userptr=ptr_;
		return fe->fptr;
	}

	void exec_function_ptr::external_set_function_mode(functionmode fmode)
	{
		mode_=fmode;
	}

	exec_function_ptr *exec_function_ptr::external_allocate(virtual_machine &vm, owca_internal_string *name, unsigned int paramcnt)
	{
		unsigned int size=sizeof(exec_function_ptr)+sizeof(function_external)+sizeof(function_external_param)*paramcnt;
		exec_function_ptr *p=vm.allocate_function_ptr(size);

		p->name_=name;
		name->gc_acquire();

#ifdef RCDEBUG
		function_external *fe=(function_external*)(((char*)p)+sizeof(*p));
		function_external_param *fep=(function_external_param*)(((char*)fe)+sizeof(*fe));
		p->_external_param=fep;
		p->_external=fe;
		p->_internal=NULL;
#endif

		return p;
	}

	void exec_function_ptr::set_class_name(exec_object *cn)
	{
		RCASSERT(classobject_==NULL);
		classobject_=cn;
		cn->gc_acquire();
	}

	std::string exec_function_ptr::declared_filename() const
	{
        if (mode_!=F_INTERNAL) 
            return "";
		return internal_opcodes()->get_file_name();
	}

	unsigned int exec_function_ptr::declared_file_line() const
	{
		if (mode_!=F_INTERNAL) return 0;
		return internal_opcodes()->get_location_from_opcode_index(internal_jump_start().get_offset()).line();
	}

	void exec_function_ptr::_mark_gc(const gc_iteration &gc) const
	{
		unsigned char cnt;

		if (classobject_) {
			gc_iteration::debug_info _d("exec_function_ptr %s: class object",name_->data_pointer());
			classobject_->gc_mark(gc);
		}
		if (nspace_) {
			gc_iteration::debug_info _d("exec_function_ptr %s: namespace", name_->data_pointer());
			nspace_->gc_mark(gc);
		}

		switch(mode_) {
		case F_INTERNAL:
			for(unsigned int i=0;i<internal_param_count();++i) {
				gc_iteration::debug_info _d("exec_function_ptr %s: default value %d", name_->data_pointer(), i);
				internal_param(i).defaultvalue.gc_mark(gc);
			}
			for(unsigned int i=0;i<internal_stack_variables_count();++i) {
				gc_iteration::debug_info _d("exec_function_ptr %s: stack variables depth %d", name_->data_pointer(), i);
				internal_stack_variables(i)->gc_mark(gc);
			}
			if (internal_parent_function()) {
				gc_iteration::debug_info _d("exec_function_ptr %s: internal parent function", name_->data_pointer());
				internal_parent_function()->gc_mark(gc);
			}
			break;
		case F_SELF:
		case F_FAST:
		case F_SELF_0:
		case F_FAST_0: cnt=0; goto process;
		case F_SELF_1:
		case F_FAST_1: cnt=1; goto process;
		case F_SELF_2:
		case F_FAST_2: cnt=2; goto process;
		case F_SELF_3:
		case F_FAST_3: cnt=3; goto process;
process:
			name_->gc_mark(gc);
			for(unsigned int i=0;i<cnt;++i) {
				{
					gc_iteration::debug_info _d("exec_function_ptr %s: external param name %d", name_->data_pointer(), i);
					external_param_name(i)->gc_mark(gc);
				}
				{
				gc_iteration::debug_info _d("exec_function_ptr %s: external default value %d", name_->data_pointer(), i);
					external_param_default_value(i).gc_mark(gc);
				}
			}
			break;
		default:
			RCASSERT(0);
		}
	}

	void exec_function_ptr::_release_resources(virtual_machine &vm)
	{
		unsigned char cnt;

		if (classobject_) classobject_->gc_release(vm);
		if (nspace_) nspace_->gc_release(vm);

		switch(mode_) {
		case F_INTERNAL:
			for(unsigned int i=0;i<internal_param_count();++i) {
				const_cast<exec_variable&>(internal_param(i).defaultvalue).gc_release(vm);
			}
			for(unsigned int i=0;i<internal_stack_variables_count();++i) internal_stack_variables(i)->gc_release(vm);
			if (--internal_stack_variables_level0Map()[0]==0) delete [] internal_stack_variables_level0Map();
			if (internal_parent_function()) internal_parent_function()->gc_release(vm);
			break;
		case F_SELF:
		case F_FAST:
		case F_SELF_0:
		case F_FAST_0: cnt=0; goto process;
		case F_SELF_1:
		case F_FAST_1: cnt=1; goto process;
		case F_SELF_2:
		case F_FAST_2: cnt=2; goto process;
		case F_SELF_3:
		case F_FAST_3: cnt=3; goto process;
process:
			name_->gc_release(vm);
			for(unsigned int i=0;i<cnt;++i) {
				external_param_name(i)->gc_release(vm);
				external_set_param_default_value(i).gc_release(vm);
			}
			break;
		default:
			RCASSERT(0);
		}
	}

	void exec_function_bound::_mark_gc(const gc_iteration &gc) const
	{
		{
			gc_iteration::debug_info _d("exec_function_bound: slf object");
			slf.gc_mark(gc);
		}
		{
			gc_iteration::debug_info _d("exec_function_bound: fnc object");
			fnc->gc_mark(gc);
		}
	}

	void exec_function_bound::_release_resources(virtual_machine &vm)
	{
		slf.gc_release(vm);
		fnc->gc_release(vm);
	}

	void exec_function_ptr::_function_set_params(exec_function_ptr *f, virtual_machine &vm, const std::string &nn0, const defval &d0)
	{
		owca_internal_string *n0=vm.allocate_string(nn0);
		f->external_set_param_name(0,n0);
		d0.get(f->external_set_param_default_value(0));
		n0->gc_release(vm);
	}
	void exec_function_ptr::_function_set_params(exec_function_ptr *f, virtual_machine &vm, const std::string &nn0, const std::string &nn1, const defval &d0, const defval &d1)
	{
		owca_internal_string *n0=vm.allocate_string(nn0);
		owca_internal_string *n1=vm.allocate_string(nn1);
		f->external_set_param_name(0,n0);
		f->external_set_param_name(1,n1);
		if (d0.type!=defval::NONE && d1.type==defval::NONE) {
			f->external_set_param_default_value(0).setmode(VAR_NO_DEF_VALUE);
			d0.get(f->external_set_param_default_value(1));
		}
		else {
			d0.get(f->external_set_param_default_value(0));
			d1.get(f->external_set_param_default_value(1));
		}
		n0->gc_release(vm);
		n1->gc_release(vm);
	}
	void exec_function_ptr::_function_set_params(exec_function_ptr *f, virtual_machine &vm, const std::string &nn0, const std::string &nn1, const std::string &nn2, const defval &d0, const defval &d1, const defval &d2)
	{
		owca_internal_string *n0=vm.allocate_string(nn0);
		owca_internal_string *n1=vm.allocate_string(nn1);
		owca_internal_string *n2=vm.allocate_string(nn2);
		f->external_set_param_name(0,n0);
		f->external_set_param_name(1,n1);
		f->external_set_param_name(2,n2);
		if (d1.type!=defval::NONE && d2.type==defval::NONE) {
			f->external_set_param_default_value(0).setmode(VAR_NO_DEF_VALUE);
			d0.get(f->external_set_param_default_value(1));
			d1.get(f->external_set_param_default_value(2));
		}
		else if (d0.type!=defval::NONE && d1.type==defval::NONE) {
			f->external_set_param_default_value(0).setmode(VAR_NO_DEF_VALUE);
			f->external_set_param_default_value(1).setmode(VAR_NO_DEF_VALUE);
			d0.get(f->external_set_param_default_value(2));
		}
		else {
			d0.get(f->external_set_param_default_value(0));
			d1.get(f->external_set_param_default_value(1));
			d2.get(f->external_set_param_default_value(2));
		}
		n0->gc_release(vm);
		n1->gc_release(vm);
		n2->gc_release(vm);
	}

	exec_function_ptr *exec_function_ptr::_function_create(virtual_machine &vm, const owca_string &name, unsigned int paramcnt)
	{
		exec_function_ptr *f=exec_function_ptr::external_allocate(vm,name._ss,paramcnt);
		return f;
	}

	exec_property *exec_function_ptr::_property_write(virtual_machine &vm, exec_function_ptr *r, exec_function_ptr *w)
	{
		exec_property *p=vm.allocate_property();
		p->read=r;
		p->write=w;
		return p;
	}

	void exec_function_ptr::_function_set_function(exec_function_ptr *f, exec_function_create_external_stack__ ptr, const void *userptr) { f->external_set_function(userptr).f__=ptr; f->external_set_function_mode(exec_function_ptr::F_FAST); }
	void exec_function_ptr::_function_set_function(exec_function_ptr *f, exec_function_create_external_stack_0 ptr, const void *userptr) { f->external_set_function(userptr).f_0=ptr; f->external_set_function_mode(exec_function_ptr::F_FAST_0); }
	void exec_function_ptr::_function_set_function(exec_function_ptr *f, exec_function_create_external_stack_1 ptr, const void *userptr) { f->external_set_function(userptr).f_1=ptr; f->external_set_function_mode(exec_function_ptr::F_FAST_1); }
	void exec_function_ptr::_function_set_function(exec_function_ptr *f, exec_function_create_external_stack_2 ptr, const void *userptr) { f->external_set_function(userptr).f_2=ptr; f->external_set_function_mode(exec_function_ptr::F_FAST_2); }
	void exec_function_ptr::_function_set_function(exec_function_ptr *f, exec_function_create_external_stack_3 ptr, const void *userptr) { f->external_set_function(userptr).f_3=ptr; f->external_set_function_mode(exec_function_ptr::F_FAST_3); }
	void exec_function_ptr::_function_set_function(exec_function_ptr *f, exec_function_create_external_stack_self__ ptr, const void *userptr) { f->external_set_function(userptr).s__=ptr; f->external_set_function_mode(exec_function_ptr::F_SELF); }
	void exec_function_ptr::_function_set_function(exec_function_ptr *f, exec_function_create_external_stack_self_0 ptr, const void *userptr) { f->external_set_function(userptr).s_0=ptr; f->external_set_function_mode(exec_function_ptr::F_SELF_0); }
	void exec_function_ptr::_function_set_function(exec_function_ptr *f, exec_function_create_external_stack_self_1 ptr, const void *userptr) { f->external_set_function(userptr).s_1=ptr; f->external_set_function_mode(exec_function_ptr::F_SELF_1); }
	void exec_function_ptr::_function_set_function(exec_function_ptr *f, exec_function_create_external_stack_self_2 ptr, const void *userptr) { f->external_set_function(userptr).s_2=ptr; f->external_set_function_mode(exec_function_ptr::F_SELF_2); }
	void exec_function_ptr::_function_set_function(exec_function_ptr *f, exec_function_create_external_stack_self_3 ptr, const void *userptr) { f->external_set_function(userptr).s_3=ptr; f->external_set_function_mode(exec_function_ptr::F_SELF_3); }


	//void user_function_t__noret::call(owca_local &ret) {
	//	owca_namespace n=fnc->ynamespace()->generate();
	//	((function)userptr)(n,_generate_parameters(*vm,cp));
	//}

	//void user_function_t0_noret::call(owca_local &ret) {
	//	owca_namespace n=fnc->ynamespace()->generate();
	//	((function)userptr)(n);
	//}

	//void user_function_t_::call(owca_local &ret) {
	//	owca_namespace n=fnc->ynamespace()->generate();
	//	ret=((function)userptr)(n,_generate_parameters(*vm,cp));
	//}

	//void user_function_t0::call(owca_local &ret) {
	//	owca_namespace n=fnc->ynamespace()->generate();
	//	ret=((function)userptr)(n);
	//}

	owca_global user_function_t__spec::call() {
		owca_namespace n=fnc->ynamespace()->generate();
		return ((function)userptr)(n,_generate_parameters(*vm,cp));
	}

	owca_global user_function_t0_spec::call()
	{
		owca_namespace n=fnc->ynamespace()->generate();
		return ((function)userptr)(n);
	}
} }












