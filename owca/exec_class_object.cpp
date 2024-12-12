#include "stdafx.h"
#include "base.h"
#include "exec_class_object.h"
#include "exec_class_int.h"
#include "exec_object.h"
#include "op_base.h"
#include "virtualmachine.h"
#include "exec_class_int.h"
#include "exec_string.h"
#include "returnvalue.h"

namespace owca { namespace __owca__ {

	void exec_class_object::_marker(const gc_iteration &gc) const
	{
		for(unsigned int i=0;i<inherited.size();++i) {
			gc_iteration::debug_info _d("exec_class_object %x: inherited object %d",this,i);
			inherited[i]->gc_mark(gc);
		}
		gc_iteration::debug_info _d("exec_class_object %x: name object %d",this);
		name->gc_mark(gc);
	}

	void exec_class_object::_destroy(virtual_machine &vm)
	{
		for(unsigned int i=0;i<inherited.size();++i) inherited[i]->gc_release(vm);
		//for(unsigned int i=0;i<lookup_order.size();++i) lookup_order[i]->gc_release(vm);
		//for(unsigned int i=0;i<offsetmap.size();++i) offsetmap.get_key(i)->gc_release(vm);
		name->gc_release(vm);

		vm.remove_type(vm_class_object_data_lookup_type);
	}

	RCLMFUNCTION std::string exec_class_object::_create(internal_class &c, exec_object *master)
	{
		virtual_machine &vm=c.vm;

		for(std::map<std::string,exec_variable>::iterator it=c.mp.begin();it!=c.mp.end();++it) {
			owca_internal_string *id=vm.allocate_string(it->first);
			exec_variable &v=it->second;

			exec_object::hashmap::hash_map_finder mi(master->members,id->hash());
			for(;mi.valid();mi.next()) RCASSERT(!master->members.getkey(mi.get()).k->equal(id));

			master->members.elem_insert(vm,master->members.write_position(id->hash()),exec_object::key(id),v);
			v.gc_acquire();
		}

		inheritable=c._inheritable;
		constructable=c._constructable;
		constructor=c.sinfo.constructor;
		destructor=c.sinfo.destructor;
		marker=c.sinfo.marker;
		storage_size=((c.sinfo.storagespace+sizeof(int)-1)/sizeof(int))*sizeof(int);
		name=vm.allocate_string(c._name);

		std::vector<exec_variable> vtt(c._inherited.size());

		unsigned int index=0;
		for(std::list<exec_object *>::iterator it=c._inherited.begin();it!=c._inherited.end();++it,++index) {
			vtt[index].set_object(*it);
		}
		return _create(c.vm,index==0 ? NULL : &vtt[0],index,master);
	}

	exec_variable *exec_class_object::lookup(owca_internal_string *ident)
	{
		for(unsigned int i=0;i<lookup_order.size();++i) {
			exec_object *p=lookup_order[i];
			exec_object::hashmap::hash_map_finder mi(p->members,ident->hash());
			for(;mi.valid() && !p->members.getkey(mi.get()).k->equal(ident);mi.next()) ;
			if (mi.valid()) return &p->members.getval(mi.get());
		}
		return NULL;
	}

	void exec_class_object::_fill_names(virtual_machine &vm, exec_object *master, std::list<exec_object*> &lkp, std::map<exec_object*,unsigned int> &classes, unsigned int &total_size)
	{
		if (classes.find(master)!=classes.end()) return;
		classes[master]=total_size;
		total_size+=storage_size;
		lkp.push_back(master);

		for(unsigned int i=0;i<lookup_order.size();++i) {
			exec_object *p=lookup_order[i];
			p->CO()._fill_names(vm,lookup_order[i],lkp,classes,total_size);
		}
	}

	extern unsigned char operator_operand_count[];

	RCLMFUNCTION std::string exec_class_object::_create(virtual_machine &vm, const exec_variable *inheritance, unsigned int inheritancecount, exec_object *master)
	{
		std::map<exec_object*,unsigned int> classes;
		std::list<exec_object *> lkp;

		RCASSERT(master->is_type());

		for(hash_map_iterator mi;master->members.next(mi);) {
			exec_variable &v=master->members.getval(mi);

			if (v.mode()==VAR_FUNCTION_FAST && v.get_function_fast().slf==NULL) {
				v.get_function_fast().fnc->set_class_name(master);
			}
		}


		total_storage_size=0;
		_fill_names(vm,master,lkp,classes,total_storage_size);
		inherited.reserve(inheritancecount);
		for(unsigned int i=0;i<inheritancecount;++i) {
			exec_object *p=inheritance[i].get_object();
			RCASSERT(p->is_type());
			if (p->CO().inheritable) {
				p->CO()._fill_names(vm,p,lkp,classes,total_storage_size);
				inherited.push_back(p);
				p->gc_acquire();
			}
			else {
				return OWCA_ERROR_FORMAT1("cant inherit from class %1",p->CO().name->str());
			}
		}

		lookup_order.reserve(lkp.size());
		for(std::list<exec_object*>::iterator it=lkp.begin();it!=lkp.end();++it) lookup_order.push_back(*it);
		RCASSERT(lookup_order.size()==lkp.size());

		for(unsigned int j=0;j<E_COUNT;++j) {
			operators[j]=lookup(vm.operator_identificators[j]);
			if (operators[j]) {
				RCASSERT(operators[j]->mode()==VAR_FUNCTION_FAST);
				exec_function_ptr *fp=operators[j]->get_function_fast().fnc;

				if (operator_operand_count[j]!=0) {
					switch(fp->mode()) {
					case exec_function_ptr::F_INTERNAL:
						if (!fp->internal_map_param().location.valid() && !fp->internal_list_param().location.valid()) break;
					case exec_function_ptr::F_FAST:
					case exec_function_ptr::F_SELF:
					case exec_function_ptr::F_FAST_0:
					case exec_function_ptr::F_FAST_1:
					case exec_function_ptr::F_FAST_2:
					case exec_function_ptr::F_FAST_3:
						return OWCA_ERROR_FORMAT1("funtion %1 has invalid signature for an operator",fp->name()->str());
					case exec_function_ptr::F_SELF_0:
					case exec_function_ptr::F_SELF_1:
					case exec_function_ptr::F_SELF_2:
					case exec_function_ptr::F_SELF_3:
						break;
					default:
						RCASSERT(0);
					}
				}
			}
		}
		offsetmap.set(classes);
		return "";
	}

	//unsigned int exec_class_object::calculate_size(unsigned int inheritedcount: unsigned int varloccount)
	//{
	//	return sizeof(exec_class_data)+sizeof(op_expression)*inheritedcount+sizeof(varloctt)*varloccount;
	//}

} }












