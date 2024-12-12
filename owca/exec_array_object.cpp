#include "stdafx.h"
#include "base.h"
#include "exec_array_object.h"
#include "virtualmachine.h"
#include "exec_variable.h"

namespace owca { namespace __owca__ {

	exec_variable &exec_array_object::get(unsigned int index)
	{
		RCASSERT(index<size_);
		return vars[index];
	}

	RCLMFUNCTION void exec_array_object::swap(exec_array_object *o)
	{
		std::swap(vars,o->vars);
		std::swap(size_,o->size_);
	}

	exec_variable *exec_array_object::_allocate_table(virtual_machine &vm, unsigned int size)
	{
		return (exec_variable*)vm.allocate_memory(sizeof(exec_variable)*size,typeid(exec_variable*));
	}

	void exec_array_object::_release_table(virtual_machine &vm, exec_variable *table)
	{
		vm.free_memory(table);
	}

	exec_variable *exec_array_object::_swap_table(exec_variable *new_table)
	{
		std::swap(vars,new_table);
		return new_table;
	}

	RCLMFUNCTION void exec_array_object::resize(virtual_machine &vm, unsigned int newsize)
	{
		if (newsize==0) {
			for(unsigned int i=0;i<size_;++i) vars[i].gc_release(vm);
			if (vars) _release_table(vm,vars);
			vars=NULL;
		}
		else if (newsize<size_) {
			for(unsigned int i=newsize;i<size_;++i) vars[i].gc_release(vm);
			exec_variable *vv=_allocate_table(vm,newsize);
			for(unsigned int i=0;i<newsize;++i) vv[i]=vars[i];
			_release_table(vm,vars);
			vars=vv;
		}
		else if (newsize>size_) {
			exec_variable *vv=_allocate_table(vm,newsize);
			for(unsigned int i=0;i<size_;++i) vv[i]=vars[i];
			for(unsigned int i=size_;i<newsize;++i) vv[i].reset();
			if (vars) _release_table(vm,vars);
			vars=vv;
		}
		size_=newsize;
	}

	void exec_array_object::insert(virtual_machine &vm, unsigned int index, const exec_variable &v)
	{
		RCASSERT(index<=size());
		resize(vm,size()+1);
		for(unsigned int i=size()-1;i>index;--i) get(i)=get(i-1);
		get(index)=v;
		v.gc_acquire();
	}

	void exec_array_object::pop(exec_variable &dst, virtual_machine &vm, unsigned int index)
	{
		dst=get(index);
		for(unsigned int i=index+1;i<size();++i) get(i-1)=get(i);
		get(size()-1).set_null();
		resize(vm,size()-1);
	}

	RCLMFUNCTION void exec_array_object::_marker(const gc_iteration &gc) const
	{
		for(unsigned int i=0;i<size_;++i) {
			gc_iteration::debug_info _d("exec_array_object %x: member variable %d",this,i);
			vars[i].gc_mark(gc);
		}
	}

	RCLMFUNCTION void exec_array_object::_destroy(virtual_machine &vm)
	{
		if (vars) {
			for(unsigned int i=0;i<size_;++i) vars[i].gc_release(vm);
			_release_table(vm,vars);
			vars=NULL;
		}
	}

	void exec_array_object::_release_array(virtual_machine &vm)
	{
		if (vars) {
			_release_table(vm,vars);
			vars=NULL;
		}
	}

} }












