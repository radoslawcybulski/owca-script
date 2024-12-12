#include "stdafx.h"
#include "base.h"
#include "exec_object.h"
#include "exec_class_object.h"
//#include "exec_class_data.h"
#include "exec_string.h"
#include "virtualmachine.h"
#include "returnvalue.h"
#include "structinfo.h"

namespace owca { namespace __owca__ {
	void exec_weakref_object::_release_resources(virtual_machine &vm)
	{
		if (ptr) {
			ptr->weakref=NULL;
		}
	}

	bool exec_object::inherit_from(exec_object *t)
	{
#ifdef RCDEBUG
		RCASSERT(is_type());
		RCASSERT(t->is_type());
#endif
		exec_class_object &co=CO();
		return co.offsetmap[t]!=NULL;
	}

	RCLMFUNCTION void exec_object::_mark_gc(const gc_iteration &gc) const
	{
		RCASSERT((_refcnt&0x80000000)==0);

		{
			gc_iteration::debug_info _d("exec_object %x: type object",this);
			type_->gc_mark(gc);
		}
		{
			gc_iteration::debug_info _d("exec_object %x: members hashmap",this);
			members.gc_mark(gc);
		}
		exec_class_object *p=gc.vm.data_from_object<exec_class_object>(type_);
		for(unsigned int i=0;i<p->offsetmap.size();++i) {
			exec_object *ptt=p->offsetmap.get_key(i);
			exec_class_object *pt=gc.vm.data_from_object<exec_class_object>(ptt);
			void (*d)(const void *, const gc_iteration &gc)=pt->marker;
			if (d) {
				gc_iteration::debug_info _d("exec_object %x: user object %d",this,i);
				d(_getptr(p->offsetmap.get_value(i)),gc);
			}
		}
	}

	RCLMFUNCTION void exec_object::_release_resources(virtual_machine &vm)
	{
		if (weakref) {
			weakref->ptr=NULL;
			//weakref->gc_release(vm);
		}
		members.clear(vm);
		exec_class_object *p=(exec_class_object*)type_->_getptr(0);
		for(unsigned int i=0;i<p->offsetmap.size();++i) {
			exec_object *ptt=p->offsetmap.get_key(i);
			exec_class_object *pt=(exec_class_object*)ptt->_getptr(0);
			void (*d)(void *, virtual_machine &vm)=pt->destructor;
			if (d) d(_getptr(p->offsetmap.get_value(i)),vm);
			//p->release(vm);
		}
		type_->gc_release(vm);
		type_=NULL;
	}

	void exec_object::set_member(virtual_machine &vm, owca_internal_string *id, const exec_variable &val)
	{
		exec_object::hashmap::hash_map_finder mi(members,id->hash());
		for(;mi.valid() && !members.getkey(mi.get()).k->equal(id);mi.next()) ;
		if (mi.valid()) {
			exec_variable &v=members.getval(mi.get());
			v.gc_release(vm);
			v=val;
		}
		else {
			members.elem_insert(vm,members.write_position(id->hash()),exec_object::key(id),val);
			id->gc_acquire();
		}
		val.gc_acquire();
	}

	bool exec_object::pop_member(virtual_machine &vm, owca_internal_string *id, exec_variable &retval)
	{
		exec_object::hashmap::hash_map_finder mi(members,id->hash());
		for(;mi.valid() && !members.getkey(mi.get()).k->equal(id);mi.next()) ;
		if (mi.valid()) {
			retval=members.getval(mi.get());
			members.getkey(mi.get()).k->gc_release(vm);
			members.getkey(mi.get()).hashmap_setdeleted();
			return true;
		}
		return false;
	}

	void exec_object::pop_member(virtual_machine &vm, owca_internal_string *id, exec_variable &retval, const exec_variable &defval)
	{
		exec_object::hashmap::hash_map_finder mi(members,id->hash());
		for(;mi.valid() && !members.getkey(mi.get()).k->equal(id);mi.next()) ;
		if (mi.valid()) {
			retval=members.getval(mi.get());
			members.getkey(mi.get()).k->gc_release(vm);
			members.getkey(mi.get()).hashmap_setdeleted();
		}
		else {
			retval=defval;
			retval.gc_acquire();
		}
	}
} }











