#include "stdafx.h"
#include "base.h"
#include "exec_base.h"
#include "exec_variable.h"
#include "exec_tuple_object.h"

namespace owca { namespace __owca__ {

	exec_tuple_object::exec_tuple_object(virtual_machine &, unsigned int oversize) {
		RCASSERT((oversize%sizeof(exec_variable))==0);
		size_=oversize/sizeof(exec_variable);
	}

	exec_variable &exec_tuple_object::get(unsigned int index)
	{
		RCASSERT(index<size_);
		return ((exec_variable*)(((char*)this)+sizeof(*this)))[index];
	}

	const exec_variable *exec_tuple_object::ptr() const
	{
		return ((exec_variable*)(((char*)this)+sizeof(*this)));
	}

	const exec_variable &exec_tuple_object::get(unsigned int index) const
	{
		RCASSERT(index<size_);
		return ((exec_variable*)(((char*)this)+sizeof(*this)))[index];
	}

	void exec_tuple_object::_marker(const gc_iteration &gc) const
	{
		for(unsigned int i=0;i<size_;++i) {
			gc_iteration::debug_info _d("exec_tuple_object %x: member variable %d",this,i);
			get(i).gc_mark(gc);
		}
	}

	void exec_tuple_object::_destroy(virtual_machine &vm)
	{
		for(unsigned int i=0;i<size_;++i) get(i).gc_release(vm);
	}

} }












