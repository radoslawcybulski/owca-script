#include "stdafx.h"
#include "base.h"
#include "exec_array_object.h"
#include "exec_variable.h"
#include "list.h"
#include "returnvalue.h"
#include "exception.h"
#include "global.h"
#include "virtualmachine.h"
#include "exec_object.h"
#include "exec_class_object.h"
#include "exec_string.h"

namespace owca {
	using namespace __owca__;

	void owca_list::obj_constructor::_write(const exec_variable &v)
	{
		ls->_set(index,v);
	}

	void owca_list::obj_constructor::_read(exec_variable &val) const
	{
		ls->_get(index,val);
	}

	owca_list::obj_constructor owca_list::operator [] (owca_int index)
	{
		return obj_constructor(this,index);
	}

	unsigned int owca_list::size() const
	{
		return ao->size();
	}

	void owca_list::clear(void)
	{
		ao->resize(*vm,0);
	}

	owca_global owca_list::clone(void) const
	{
		exec_array_object *oo;
		exec_object *o=vm->allocate_array(oo);
		oo->resize(*vm,ao->size());

		for(unsigned int i=0;i<oo->size();++i) {
			(oo->get(i)=ao->get(i)).gc_acquire();
		}

		owca_global ret(*vm);
		ret._object.set_object(o);
		return ret;
	}

	void owca_list::resize(unsigned int newsize)
	{
		ao->resize(*vm,newsize);
	}

	void owca_list::push_front(const owca_global &value)
	{
		value._check_vm(vm);
		ao->insert(*vm,0,value._object);
	}

	void owca_list::push_back(const owca_global &value)
	{
		value._check_vm(vm);
		ao->insert(*vm,ao->size(),value._object);
	}

	unsigned int owca_list::_update_index(owca_int index, bool allow_one_index_after_size) const
	{
		if (index<0) index+=ao->size();
		if (index<0 || (allow_one_index_after_size ? (index>ao->size()) : (index>=ao->size()))) throw owca_exception(ExceptionCode::INVALID_PARAM_TYPE, OWCA_ERROR_FORMAT2("index %1 is invalid for list of size %2", int_to_string(index), int_to_string(ao->size())));
		return (unsigned int)index;
	}

	void owca_list::insert(owca_int index, const owca_global &value)
	{
		value._check_vm(vm);
		ao->insert(*vm,_update_index(index,true),value._object);
	}

	owca_global owca_list::pop_front()
	{
		owca_global g;
		if (ao->size()==0) throw owca_exception(ExceptionCode::EMPTY_LIST, "cant pop fron empty list");
		ao->pop(g._object,*vm,0);
		g._update_vm(vm);
		return g;
	}

	owca_global owca_list::pop_back()
	{
		owca_global g;
		if (ao->size()==0) throw owca_exception(ExceptionCode::EMPTY_LIST, "cant pop fron empty list");
		ao->pop(g._object,*vm,ao->size()-1);
		g._update_vm(vm);
		return g;
	}

	owca_global owca_list::pop(owca_int index)
	{
		owca_global g;
		ao->pop(g._object,*vm,_update_index(index,false));
		g._update_vm(vm);
		return g;
	}

	void owca_list::_get(owca_int index, exec_variable &value) const
	{
		value = ao->get(_update_index(index, false));
		value.gc_acquire();
	}

	void owca_list::_set(owca_int index, const exec_variable &value)
	{
		exec_variable &v = ao->get(_update_index(index, false));
		v.gc_release(*vm);
		v=value;
		v.gc_acquire();
	}

	void owca_list::set(owca_int index, const owca_global &value)
	{
		value._check_vm(vm);
		_set(index,value._object);
	}

	owca_global owca_list::get(owca_int index) const
	{
		owca_global g;
		_get(index,g._object);
		g._update_vm(vm);
		return g;
	}

	namespace __owca__ {
		void update_2index(owca_int &i1, owca_int &i2, owca_int size);
	}

	owca_global owca_list::get(owca_int from, owca_int to) const
	{
		exec_array_object *oo;
		__owca__::update_2index(from,to,ao->size());
		RCASSERT(to>=from);
		RCASSERT(from>=0);
		RCASSERT(to<=ao->size());

		exec_object *o=vm->allocate_array(oo);

		oo->resize(*vm,(unsigned int)(to-from));
		for(unsigned int i=0;i<oo->size();++i) {
			(oo->get(i)=ao->get((unsigned int)(from+i))).gc_acquire();
		}

		owca_global ret(*vm);
		ret._object.set_object(o);
		return ret;
	}

}

