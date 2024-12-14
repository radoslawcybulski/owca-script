#include "stdafx.h"
#include "base.h"
#include "exec_class_int.h"
#include "virtualmachine.h"
#include "exec_class_object.h"
#include "exec_string.h"
#include "exec_object.h"
#include "returnvalue.h"
#include "exception.h"
#include "class.h"
#include "exec_namespace.h"

namespace owca {
	void owca_class::obj_constructor::_write(const __owca__::exec_variable &val)
	{
		std::pair<std::map<std::string,__owca__::exec_variable>::iterator,bool> ret=ic.mp.insert(std::pair<std::string,__owca__::exec_variable>(name->str(),__owca__::exec_variable()));
		if (!ret.second) {
			ret.first->second.gc_release(_vm);
		}
		ret.first->second=val;
		val.gc_acquire();
	}

	void owca_class::obj_constructor::_read(__owca__::exec_variable &val) const
	{
		std::map<std::string,__owca__::exec_variable>::iterator it=ic.mp.find(name->str());
		if (it==ic.mp.end()) throw owca_exception(ExceptionCode::MISSING_MEMBER, OWCA_ERROR_FORMAT1("missing member %1",name->str()));
		val=it->second;
		val.gc_acquire();
	}

	owca_class::obj_constructor::obj_constructor(__owca__::internal_class &ic_, const owca_string &name_) : __owca__::obj_constructor_function(ic_.nspace,name_._ss),ic(ic_)
	{
	}

	namespace __owca__ {

		void internal_class::obj_constructor::_write(const __owca__::exec_variable &val)
		{
			std::pair<std::map<std::string,__owca__::exec_variable>::iterator,bool> ret=ic.mp.insert(std::pair<std::string,__owca__::exec_variable>(name->str(),__owca__::exec_variable()));
			if (!ret.second) {
				ret.first->second.gc_release(_vm);
			}
			ret.first->second=val;
		}

		void internal_class::obj_constructor::_read(__owca__::exec_variable &val) const
		{
			std::map<std::string,__owca__::exec_variable>::iterator it=ic.mp.find(name->str());
			if (it==ic.mp.end()) throw owca_exception(ExceptionCode::MISSING_MEMBER, OWCA_ERROR_FORMAT1("missing member %1",name->str()));
			val=it->second;
			val.gc_acquire();
		}

		internal_class::obj_constructor::obj_constructor(__owca__::internal_class &ic_, const owca_string &name_) : __owca__::obj_constructor_base(ic_.vm,name_._ss),ic(ic_)
		{
		}

		internal_class::obj_constructor internal_class::operator [] (const owca_string &s)
		{
			RCASSERT(_check_name(s.data(),s.data_size()));
			return obj_constructor(*this,s);
		}

		bool internal_class::_check_name(const char *name, unsigned int size)
		{
			if (size==0) return false;
			unsigned int i=name[0]=='$' ? 1 : 0;
			if (i>=size) return false;
			if (name[i]>='0' && name[i]<='9') return false;
			for(;i<size;++i) {
				if (!(
					(name[i]>='0' && name[i]<='9') ||
					(name[i]>='a' && name[i]<='z') ||
					(name[i]>='A' && name[i]<='Z') ||
					name[i]=='_')) return false;
			}
			return true;
		}

		void internal_class::gc_mark(gc_iteration &g)
		{
			for(std::map<std::string,exec_variable>::iterator it=mp.begin();it!=mp.end();++it) it->second.gc_mark(g);
			for(std::list<exec_object*>::iterator it=_inherited.begin();it!=_inherited.end();++it) (*it)->gc_mark(g);
		}

		void internal_class::_add_inherit(exec_object *o)
		{
			_inherited.push_back(o);
			o->gc_acquire();
		}

		internal_class::~internal_class()
		{
			if (next==prev) {
				RCASSERT(next==this);
				RCASSERT(vm.internalclases==this);
				vm.internalclases=NULL;
			}
			else {
				next->prev=prev;
				prev->next=next;
				if (vm.internalclases==this) vm.internalclases=next;
				RCASSERT(vm.internalclases!=this && vm.internalclases!=NULL);
			}
			for(std::map<std::string,exec_variable>::iterator it=mp.begin();it!=mp.end();++it) it->second.gc_release(vm);
			for(std::list<exec_object*>::iterator it=_inherited.begin();it!=_inherited.end();++it) (*it)->gc_release(vm);
		}

		internal_class::internal_class(virtual_machine &vm_, const std::string &name_) : vm(vm_),nspace(NULL),_name(name_),_inheritable(true),_constructable(true)
		{
			if (!vm.internalclases) vm.internalclases=next=prev=this;
			else {
				next=vm.internalclases->next;
				prev=vm.internalclases;
				vm.internalclases=next->prev=prev->next=this;
			}
		}

		internal_class::internal_class(exec_namespace &nspace_, const std::string &name_) : vm(nspace_.vm),nspace(&nspace_),_name(name_),_inheritable(true),_constructable(true)
		{
			if (!vm.internalclases) vm.internalclases=next=prev=this;
			else {
				next=vm.internalclases->next;
				prev=vm.internalclases;
				vm.internalclases=next->prev=prev->next=this;
			}
		}

		std::string internal_class::create(exec_object *&o)
		{
			//exec_object *ob=new (new char[sizeof(exec_object)+sizeof(exec_class_object)]) exec_object(vm);
			exec_class_object *oo;
			o=vm.allocate_type(oo);
			RCASSERT(o);
			std::string z=oo->_create(*this,o);
			if (!z.empty()) {
				o->gc_release(vm);
				o=NULL;
			}
			else {
				if (sinfo.structident) {
					vm._registerstructid(sinfo.structident,o);
					oo->vm_class_object_data_lookup_type=sinfo.structident;
				}
			}
			return z;
		}
	}
}












