#include "stdafx.h"
#include "base.h"
#include "global.h"
#include "virtualmachine.h"
#include "exec_string.h"
#include "exec_object.h"
#include "exec_function_ptr.h"
#include "exec_class_object.h"
#include "exec_property.h"
#include "exec_variable.h"
#include "op_base.h"
#include "exec_map_object.h"
#include "returnvalue.h"
#include "structinfo.h"
#include "defval.h"
#include "exception.h"
#include "vm_execution_stack_elem_base.h"
#include "exec_namespace.h"

namespace owca { namespace __owca__ {
	defval dv_null(defval::NULL_),dv_unused(defval::NONE),dv_no_param_given(defval::NO_PARAM_GIVEN);

	exec_variable_location exec_variable_location::invalid;

	defval::defval(char *c) : type(STRING)
	{
		char *e=c;
		while(*e) ++e;

		unsigned int index = 0;
		unsigned int char_count = owca_internal_string::calculate_char_count_and_missing_bytes_if_any(index, c, (unsigned int)(e-c));
		RCASSERT(index == 0);
		data.s = owca_internal_string_nongc::allocate(c, (unsigned int)(e - c), char_count);
	}

	defval::~defval()
	{
		if (type == STRING) data.s->gc_release(*(virtual_machine*)0); // TODO: fix it
	}

	std::string exec_variable::kind_as_text() const {
		switch (mode()) {
		case exectype::VAR_NULL: return "NULL";
		case exectype::VAR_INT: return "integer";
		case exectype::VAR_REAL: return "real";
		case exectype::VAR_BOOL: return "bool";
		case exectype::VAR_STRING: return "string";
		case exectype::VAR_GENERATOR: return "generator";
		case exectype::VAR_PROPERTY: return "property";
		case exectype::VAR_FUNCTION: return "function";
		case exectype::VAR_FUNCTION_FAST: return "function";
		case exectype::VAR_OBJECT: return "object";
		case exectype::VAR_NAMESPACE: return "namespace";
		case exectype::VAR_WEAK_REF: return "weak-ref";
		case exectype::VAR_COUNT:
		case exectype::VAR_HASH_DELETED:
		case exectype::VAR_HASH_EMPTY:
		case exectype::VAR_NO_DEF_VALUE:
		case exectype::VAR_NO_PARAM_GIVEN:
			RCASSERT(0);
		}
		return "<unknown variable type>";
	}

	void defval::get(exec_variable &r) const
	{
		switch(type) {
		case defval::INT: r.set_int(data.i); break;
		case defval::REAL: r.set_real(data.r); break;
		case defval::NULL_: r.set_null(); break;
		case defval::NONE: r.setmode(VAR_NO_DEF_VALUE); break;
		case defval::NO_PARAM_GIVEN: r.setmode(VAR_NO_PARAM_GIVEN); return;
		case defval::BOOL: r.set_bool(data.b); break;
		case defval::STRING: r.set_string(data.s); data.s->gc_acquire(); return;
		default:
			RCASSERT(0);
		}
	}

	bool exec_variable::type(exectype tp) const
	{
		switch(tp) {
		case VAR_NULL:
		case VAR_INT:
		case VAR_REAL:
		case VAR_BOOL:
		case VAR_STRING:
		case VAR_GENERATOR:
		case VAR_PROPERTY:
		case VAR_FUNCTION:
		case VAR_FUNCTION_FAST:
		case VAR_NAMESPACE:
		case VAR_WEAK_REF:
			return mode()==tp;
		case VAR_OBJECT:
		default:
			RCASSERT(0);
		}
		return false;
	}

	bool exec_variable::type(exec_object *tp) const
	{
		RCASSERT(tp->is_type());
		return mode()==VAR_OBJECT && data.o->type()->inherit_from(tp);
	}

	void exec_variable::gc_acquire() const
	{
		switch(mode()) {
		case VAR_INT:
		case VAR_NULL:
		case VAR_REAL:
		case VAR_BOOL:
			break;
		case VAR_STRING:
			data.s->gc_acquire();
			break;
		case VAR_GENERATOR:
			data.generator->gc_acquire();
			break;
		case VAR_PROPERTY:
			data.prop->gc_acquire();
			break;
		case VAR_FUNCTION_FAST:
			data.fncfast.fnc->gc_acquire();
			if (data.fncfast.slf) data.fncfast.slf->gc_acquire();
			break;
		case VAR_FUNCTION:
			data.fnc->gc_acquire();
			break;
		case VAR_NAMESPACE:
			data.nspace->gc_acquire();
			break;
		case VAR_WEAK_REF:
			data.weakref->gc_acquire();
			break;
		case VAR_OBJECT:
			data.o->gc_acquire();
			break;
		case VAR_NO_PARAM_GIVEN:
			break;
		default:
			RCASSERT(0);
		}
	}

	void exec_variable::gc_release(virtual_machine &vm)
	{
		switch(mode()) {
		case VAR_INT:
		case VAR_NULL:
		case VAR_REAL:
		case VAR_BOOL:
			break;
		case VAR_STRING:
			data.s->gc_release(vm);
			break;
		case VAR_GENERATOR:
			data.generator->gc_release(vm);
			break;
		case VAR_PROPERTY:
			data.prop->gc_release(vm);
			break;
		case VAR_FUNCTION_FAST: {
			data.fncfast.fnc->gc_release(vm);
			if (data.fncfast.slf) data.fncfast.slf->gc_release(vm);
			//if (c1 | c2) _mode=VAR_UNDEFINED;
			break; }
		case VAR_FUNCTION:
			data.fnc->gc_release(vm);
			break;
		case VAR_WEAK_REF:
			data.weakref->gc_release(vm);
			break;
		case VAR_NAMESPACE:
			data.nspace->gc_release(vm);
			break;
		case VAR_OBJECT:
			data.o->gc_release(vm);
			break;
		case VAR_NO_DEF_VALUE:
		case VAR_NO_PARAM_GIVEN:
		case VAR_COUNT:
			break;
		default:
			RCASSERT(0);
		}
	}

	void exec_variable::gc_mark(const gc_iteration &gc) const
	{
		switch(mode()) {
		case VAR_INT:
		case VAR_NULL:
		case VAR_REAL:
		case VAR_BOOL:
			break;
		case VAR_STRING:
			data.s->gc_mark(gc);
			//RCASSERT(data.s->id()>=0 && data.s->gc_count()>=0);
			break;
		case VAR_GENERATOR:
			data.generator->gc_mark(gc);
			break;
		case VAR_PROPERTY:
			data.prop->gc_mark(gc);
			break;
		case VAR_FUNCTION_FAST:
			if (data.fncfast.slf) data.fncfast.slf->gc_mark(gc);
			data.fncfast.fnc->gc_mark(gc);
			break;
		case VAR_FUNCTION:
			data.fnc->gc_mark(gc);
			break;
		case VAR_WEAK_REF:
			data.weakref->gc_mark(gc);
			break;
		case VAR_NAMESPACE:
			data.nspace->gc_mark(gc);
			break;
		case VAR_OBJECT:
			data.o->gc_mark(gc);
			break;
		case VAR_NO_DEF_VALUE:
		case VAR_NO_PARAM_GIVEN:
		case VAR_COUNT:
			break;
		default:
			RCASSERT(0);
		}
	}

	//bool exec_variable::ident(owca_int &hash, const char *&data, unsigned int &size) const
	//{
	//	switch(mode()) {
	//	case VAR_STRING:
	//		hash=get_string()->hash();
	//		data=get_string()->data();
	//		size=get_string()->size();
	//		return true;
	//	}
	//	return false;
	//}

	bool exec_variable::get_int(owca::owca_int &ret) const
	{
		switch(mode()) {
		case VAR_INT:
			ret=get_int();
			return true;
		case VAR_REAL:
			if ((owca_real)((owca_int)get_real())==get_real()) {
				ret=(owca_int)get_real();
				return true;
			}
		}
		return false;
	}

	bool exec_variable::get_int_min(owca::owca_int &ret) const
	{
		switch(mode()) {
		case VAR_NULL:
			ret=0;
			return true;
		case VAR_INT:
			ret=get_int();
			return true;
		case VAR_REAL:
			if ((owca_real)((owca_int)get_real())==get_real()) {
				ret=(owca_int)get_real();
				return true;
			}
		}
		return false;
	}

	bool exec_variable::get_int_max(owca::owca_int &ret) const
	{
		switch(mode()) {
		case VAR_NULL:
			ret=std::numeric_limits<owca_int>::max();
			return true;
		case VAR_INT:
			ret=get_int();
			return true;
		case VAR_REAL:
			if ((owca_real)((owca_int)get_real())==get_real()) {
				ret=(owca_int)get_real();
				return true;
			}
		}
		return false;
	}

	RCLMFUNCTION bool exec_variable::is_same(const exec_variable &other) const
	{
		if (mode()!=other.mode()) return false;
		switch(mode()) {
		case VAR_NULL: return true;
		case VAR_BOOL: return get_bool()==other.get_bool();
		case VAR_INT: return get_int()==other.get_int();
		case VAR_REAL: return get_real()==other.get_real();
		case VAR_STRING: return get_string()==other.get_string();
		case VAR_GENERATOR: return get_generator()==other.get_generator();
		case VAR_PROPERTY: return get_property()==other.get_property();
		case VAR_FUNCTION: return get_function()->function()==other.get_function()->function() &&
							   get_function()->slf.is_same(other.get_function()->slf);
		case VAR_FUNCTION_FAST: return get_function_fast().fnc==other.get_function_fast().fnc && get_function_fast().slf==other.get_function_fast().slf;
		case VAR_NAMESPACE: return get_namespace()==other.get_namespace(); break;
		case VAR_WEAK_REF: return get_weak_ref()==other.get_weak_ref(); break;
		case VAR_OBJECT: return get_object()==other.get_object();
		default:
			RCASSERT(0);
		}
		return false;
	}

	// NOTE: u cant assign to operators
	bool exec_variable::has_operator(virtual_machine &vm, operatorcodes opc) const
	{
		exec_object *tp=(mode()==VAR_OBJECT ? get_object()->type() : vm.basicmap[mode()]);
		RCASSERT(tp);
		return tp->CO().operators[opc]!=NULL;
	}

	exec_function_ptr *exec_variable::get_operator_function(virtual_machine &vm, operatorcodes oper) const
	{
		exec_object *tp=(mode()==VAR_OBJECT ? get_object()->type() : vm.basicmap[mode()]);
		RCASSERT(tp);
		exec_variable *v=tp->CO().operators[oper];
		if (v==NULL) return NULL;

		if (v->mode()==VAR_FUNCTION_FAST) return v->get_function_fast().fnc;
		else RCASSERT(0);
		return NULL;
	}

	RCLMFUNCTION bool exec_variable::lookup_operator(exec_variable &retval, virtual_machine &vm, operatorcodes opc) const
	{
		exec_function_ptr *pfnc=get_operator_function(vm,opc);
		if (pfnc==NULL) return false;
		retval.set_function_s(vm,*this,pfnc);
		return true;
	}

	RCLMFUNCTION lookupreturnvalue exec_variable::lookup_read(exec_variable &ret, virtual_machine &vm, owca_internal_string *ident, bool alloc_stack_on_function_call) const
	{
		exec_object *tp;
		exec_variable *v;

		if (mode()==VAR_OBJECT) {
			exec_object *o=get_object();

			if (o->is_type()) {
				v=o->CO().lookup(ident);
				if (v) {
					switch(v->mode()) {
					case VAR_FUNCTION_FAST:
						get_object()->gc_acquire();
						v->get_function_fast().fnc->gc_acquire();
						ret.set_function_fast(v->get_function_fast().fnc,get_object());
						break;
					case VAR_FUNCTION:
						get_object()->gc_acquire();
						v->get_function()->function()->gc_acquire();
						ret.set_function_fast(v->get_function()->function(),get_object());
						break;
					default:
						ret=*v;
						ret.gc_acquire();
						break;
					}
					return lookupreturnvalue::LOOKUP_FOUND;
				}

				return lookupreturnvalue::LOOKUP_NOT_FOUND;
			}
			else {
				exec_object::hashmap::hash_map_finder mi(o->members,ident->hash());
				for(;mi.valid() && !o->members.getkey(mi.get()).k->equal(ident);mi.next()) ;
				if (mi.valid()) {
					ret=o->members.getval(mi.get());
					ret.gc_acquire();
					return lookupreturnvalue::LOOKUP_FOUND;
				}
			}
			tp=o->type();
		}
		else if (mode()==VAR_NAMESPACE) {
			if (!get_namespace()->get_variable(ident,ret)) return lookupreturnvalue::LOOKUP_NOT_FOUND;
			return lookupreturnvalue::LOOKUP_FOUND;
		}
		else {
			tp=vm.basicmap[mode()];
		}

		RCASSERT(tp);
		v=tp->CO().lookup(ident);
		if (v==NULL) return lookupreturnvalue::LOOKUP_NOT_FOUND;
		switch(v->mode()) {
		case VAR_FUNCTION_FAST:
			ret.set_function_s(vm,*this,v->get_function_fast().fnc);
			return lookupreturnvalue::LOOKUP_FOUND;
		case VAR_FUNCTION:
			ret.set_function_s(vm,*this,v->get_function()->function());
			return lookupreturnvalue::LOOKUP_FOUND;
		case VAR_PROPERTY:
			if (alloc_stack_on_function_call) vm.push_execution_stack();
			if (v->get_property()->read) {
				exec_variable z;
				z.set_function_s(vm,*this,v->get_property()->read);
				vm.prepare_call_function(&ret,z,NULL,0);
				z.gc_release(vm);
			}
			else vm.raise_not_rvalue_member(*this,ident->str());
			return lookupreturnvalue::LOOKUP_FUNCTION_CALL;
		case VAR_INT:
		case VAR_REAL:
		case VAR_BOOL:
		case VAR_NULL:
		case VAR_STRING:
		case VAR_GENERATOR:
		case VAR_OBJECT:
		case VAR_WEAK_REF:
			ret=*v;
			ret.gc_acquire();
			return lookupreturnvalue::LOOKUP_FOUND;
		default:
			RCASSERT(0);
			return lookupreturnvalue::LOOKUP_FOUND;
		}
	}

	RCLMFUNCTION lookupreturnvalue exec_variable::lookup_write(exec_variable &ret, virtual_machine &vm, owca_internal_string *ident, const exec_variable &val, bool alloc_stack_on_function_call) const
	{
		exec_object *tp;
		exec_object::hashmap *members;

		if (mode()==VAR_NAMESPACE) {
			RCASSERT(get_namespace()->insert_variable(ident,val)>=0);
			//if (get_namespace()->insert_variable(ident,val)<0) {
			//	if (alloc_stack_on_function_call) vm.push_execution_stack();
			//	vm.raise_not_lvalue_member(*this,ident->str());
			//	return lookupreturnvalue::LOOKUP_FUNCTION_CALL;
			//}
			ret=val;
			val.gc_acquire();
			return lookupreturnvalue::LOOKUP_FOUND;
		}
		if (ident->data_size() == 0) {
			if (alloc_stack_on_function_call) vm.push_execution_stack();
			vm.raise_invalid_ident(ident->str());
			return lookupreturnvalue::LOOKUP_FUNCTION_CALL;
		}
		if (ident->data_pointer()[0]=='$') {
			if (alloc_stack_on_function_call) vm.push_execution_stack();
			vm.raise_not_lvalue_member(*this,ident->str());
			return lookupreturnvalue::LOOKUP_FUNCTION_CALL;
		}
		if (mode()==VAR_OBJECT) {
			exec_object *o=get_object();
			members=&o->members;
			exec_object::hashmap::hash_map_finder mi(o->members,ident->hash());
			for(;mi.valid() && !o->members.getkey(mi.get()).k->equal(ident);mi.next()) ;
			if (mi.valid()) {
				exec_variable &v=o->members.getval(mi.get());
				v.gc_release(vm);
				ret=v=val;
				v.gc_acquire();
				ret.gc_acquire();
				return lookupreturnvalue::LOOKUP_FOUND;
			}
			tp=o->type();
		}
		else {
			tp=vm.basicmap[mode()];
			members=NULL;
		}

		RCASSERT(tp);
		exec_variable *v=tp->CO().lookup(ident);
		if (v==NULL) goto update_members;
		switch(v->mode()) {
		case VAR_INT:
		case VAR_REAL:
		case VAR_BOOL:
		case VAR_NULL:
		case VAR_STRING:
		case VAR_GENERATOR:
		case VAR_OBJECT:
		case VAR_FUNCTION:
		case VAR_FUNCTION_FAST:
		case VAR_WEAK_REF:
	update_members:
			if (members) {
				members->elem_insert(vm,members->write_position(ident->hash()),exec_object::key(ident),val);
				ret=val;
				ret.gc_acquire();
				ret.gc_acquire();
				ident->gc_acquire();
				return lookupreturnvalue::LOOKUP_FOUND;
			}
			return lookupreturnvalue::LOOKUP_NOT_FOUND;
		case VAR_PROPERTY:
			if (alloc_stack_on_function_call) vm.push_execution_stack();
			if (v->get_property()->write) {
				exec_variable z;
				z.set_function_s(vm,*this,v->get_property()->write);
				vm.prepare_call_function(&ret,z,&val,1);
				z.gc_release(vm);
			}
			else vm.raise_not_lvalue_member(*this,ident->str());
			return lookupreturnvalue::LOOKUP_FUNCTION_CALL;
		default:
			RCASSERT(0);
			return lookupreturnvalue::LOOKUP_NOT_FOUND;
		}
	}

	void exec_variable::set_function_s(virtual_machine &vm, const exec_variable &slf, exec_function_ptr *fnc)
	{
		fnc->gc_acquire();
		switch(slf.mode()) {
		case VAR_NULL:
			set_function_fast(fnc);
			break;
		case VAR_OBJECT:
			slf.get_object()->gc_acquire();
			set_function_fast(fnc,slf.get_object());
			break;
		default:
			slf.gc_acquire();
			set_function(vm.allocate_function_bound(fnc,slf));
		}
	}



} }












