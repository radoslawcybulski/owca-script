#include "stdafx.h"
#include "base.h"
#include "vm.h"
#include "global.h"
#include "exception.h"
#include "exec_string.h"
#include "virtualmachine.h"
#include "exec_object.h"
#include "exec_function_ptr.h"
#include "op_compare.h"
#include "exec_array_object.h"
#include "exec_tuple_object.h"
#include "exec_class_object.h"
#include "string.h"
#include "map.h"
#include "list.h"
#include "tuple.h"
#include "parameters.h"
#include "namespace.h"
#include "exec_namespace.h"
#include "exec_class_int.h"

namespace owca {
	using namespace __owca__;

	owca_local owca_local::null;
	owca_local owca_local::null_no_value;

	owca_local::owca_local(const owca_string &s) : _vm(NULL)
	{
		if (s._ss) {
			_update_vm(s._vm);
			_object.set_string(s._ss);
			s._ss->gc_acquire();
		}
		else _object.set_string(owca_internal_string_nongc::allocate("",0,0));
	}

	owca_local::owca_local(const owca_list &l) : _vm(NULL)
	{
		_update_vm(l.vm);
		_object.set_object(l.obj);
		l.obj->gc_acquire();
	}

	owca_local::owca_local(const owca_map &l) : _vm(NULL)
	{
		_update_vm(l.vm);
		_object.set_object(l.obj);
		l.obj->gc_acquire();
	}

	owca_local::owca_local(const owca_tuple &l) : _vm(NULL)
	{
		_update_vm(l.vm);
		_object.set_object(l.obj);
		l.obj->gc_acquire();
	}

	owca_local::owca_local(const std::string &s) : _vm(NULL)/*,_next(NULL),_prev(NULL)*/
	{
		unsigned int index = 0;
		unsigned int char_count = owca_internal_string::calculate_char_count_and_missing_bytes_if_any(index, s.c_str(),(unsigned int)s.size());
		if (index != 0)
			throw owca_exception(ExceptionCode::INVALID_UTF8_STRING, "invalid utf8 string");
		_object.set_string(owca_internal_string_nongc::allocate(s.c_str(), (unsigned int)s.size(),char_count));
	}

	owca_local::owca_local(const char *c) : _vm(NULL)/*,_next(NULL),_prev(NULL)*/
	{
		const char *e=c;
		while(*e) ++e;
		unsigned int index = 0;
		unsigned int char_count = owca_internal_string::calculate_char_count_and_missing_bytes_if_any(index, c, (unsigned int)(e-c));
		if (index != 0)
			throw owca_exception(ExceptionCode::INVALID_UTF8_STRING, "invalid utf8 string");
		_object.set_string(owca_internal_string_nongc::allocate(c, (unsigned int)(e - c),char_count));
	}

	owca_local::owca_local(char c) : _vm(NULL)/*,_next(NULL),_prev(NULL)*/
	{
		unsigned int index = 0;
		unsigned int char_count = owca_internal_string::calculate_char_count_and_missing_bytes_if_any(index, &c,1);
		if (index != 0)
			throw owca_exception(ExceptionCode::INVALID_UTF8_STRING, "invalid utf8 string");
		_object.set_string(owca_internal_string_nongc::allocate(&c, 1,char_count));
	}

	void owca_local::bind(owca_vm &vm)
	{
		if (_vm!=NULL && _vm!=vm.vm) throw owca_exception(ExceptionCode::INVALID_VM, "VM already set and is different");
		_update_vm(vm.vm);
	}

	void *owca_local::_member_data(const structid_type &type) const
	{
		return _object.mode()==VAR_OBJECT ? _vm->_data_from_object(_object.get_object(),type) : NULL;
	}

	void owca_local::_check_vm(__owca__::virtual_machine *vm) const
	{
		switch(_object.mode()) {
		case VAR_NULL:
		case VAR_BOOL:
		case VAR_INT:
		case VAR_REAL:
		case VAR_STRING:
			return;
		case VAR_GENERATOR:
		case VAR_PROPERTY:
		case VAR_FUNCTION:
		case VAR_FUNCTION_FAST:
		case VAR_NAMESPACE:
		case VAR_WEAK_REF:
		case VAR_OBJECT:
			RCASSERT(_vm);
			if (vm!=NULL && _vm!=vm) throw owca_exception(ExceptionCode::INVALID_VM, "different VMs in use");
			break;
		default:
			RCASSERT(0);
		}
	}

	owca_vm *owca_local::vm() const
	{
		return _vm ? _vm->owner_vm : NULL;
	}

	void owca_local::_check_vm(const owca_local &l) const
	{
		l._check_vm(_vm);
	}

	RCLMFUNCTION owca_global owca_local::type() const
	{
		if (_vm==NULL) return owca_global::null;
		owca_global o(_vm);
		if (_object.mode()==VAR_OBJECT) {
			o._object.set_object(_object.get_object()->type());
			o._object.get_object()->gc_acquire();
		}
		else {
			RCASSERT(_vm->basicmap[_object.mode()]);
			_vm->basicmap[_object.mode()]->gc_acquire();
			o._object.set_object(_vm->basicmap[_object.mode()]);
		}
		return o;
	}

	void owca_local::_update_vm(__owca__::virtual_machine *vm)
	{
		if (vm) _vm=vm;
	}

	void owca_global::_attach()
	{
		RCASSERT(_vm);
		if (_vm->vars) {
			_prev=_vm->vars;
			_next=_vm->vars->_next;
			_next->_prev=_prev->_next=this;
		}
		else _vm->vars=_next=_prev=this;
		RCASSERT(_prev && _next);
	}

	void owca_global::_detach()
	{
		RCASSERT(_vm);
		if (_vm->vars==this) {
			_vm->vars=_next;
			if (_vm->vars==this) {
				RCASSERT(_next==_prev && _next==_vm->vars);
				_vm->vars=NULL;
				goto done;
			}
		}
		RCASSERT(_prev && _next);
		_next->_prev=_prev;
		_prev->_next=_next;
	done:
		_next=_prev=NULL;
	}

	void owca_global::_update_vm(__owca__::virtual_machine *vm)
	{
		if (vm && vm!=_vm) {
			if (_vm) _detach();
			_vm=vm;
			_attach();
		}
	}

	const owca_local &owca_local::operator = (const owca_local &o)
	{
		if (this!=&o) {
			o._object.gc_acquire();
			_object.gc_release(*_vm);
			_update_vm(o._vm);
			_object=o._object;
		}
		return *this;
	}

	owca_local::owca_local(const owca::owca_local &o) : _vm(NULL)
	{
		_object.set_null();
		*this=o;
	}

	void owca_local::string_set(const std::string &s)
	{
		string_set(s.c_str(),(unsigned int)s.size());
	}

	void owca_local::string_set(const char *p)
	{
		const char *e=p;
		while(*e) ++e;
		string_set(p,(unsigned int)(e-p));
	}

	void owca_local::string_set(const char *p, unsigned int size)
	{
		unsigned int index = 0;
		unsigned int char_count = owca_internal_string::calculate_char_count_and_missing_bytes_if_any(index, p, size);
		if (index != 0)
			throw owca_exception(ExceptionCode::INVALID_UTF8_STRING, "invalid utf8 string");
		_object.gc_release(*_vm);
		_object.set_string(owca_internal_string_nongc::allocate(p,size,char_count));
	}

	owca_string owca_local::string_get() const
	{
		if (_object.mode()!=VAR_STRING) throw owca_exception(ExceptionCode::INVALID_PARAM_TYPE, "not a string");
		owca_string s;
		s._ss=_object.get_string();
		s._vm=_vm;
		return s;
	}

	const char *owca_local::string_get(unsigned int &sz) const
	{
		if (_object.mode()!=VAR_STRING) throw owca_exception(ExceptionCode::INVALID_PARAM_TYPE, "not a string");
		sz = _object.get_string()->data_size();
		return _object.get_string()->data_pointer();
	}

    std::string owca_local::function_name() const
    {
        if (!function_is()) throw owca_exception(ExceptionCode::INVALID_PARAM_TYPE, "not a function");
        RCASSERT(_vm != NULL);
		switch(_object.mode()) {
		case VAR_FUNCTION:
            return _object.get_function()->function()->name()->str();
			break;
		case VAR_FUNCTION_FAST:
            return _object.get_function_fast().fnc->name()->str();
			break;
		default:
			RCASSERT(0);
		}
		return "";
    }

	std::string owca_local::function_file_name() const
    {
        if (!function_is()) throw owca_exception(ExceptionCode::INVALID_PARAM_TYPE, "not a function");
        RCASSERT(_vm != NULL);
		switch(_object.mode()) {
		case VAR_FUNCTION:
            return _object.get_function()->function()->declared_filename();
			break;
		case VAR_FUNCTION_FAST:
            return _object.get_function_fast().fnc->declared_filename();
			break;
		default:
			RCASSERT(0);
		}
		return "";
    }

	unsigned int owca_local::function_file_line() const
    {
        if (!function_is()) throw owca_exception(ExceptionCode::INVALID_PARAM_TYPE, "not a function");
        RCASSERT(_vm != NULL);
		switch(_object.mode()) {
		case VAR_FUNCTION:
            return _object.get_function()->function()->declared_file_line();
			break;
		case VAR_FUNCTION_FAST:
            return _object.get_function_fast().fnc->declared_file_line();
			break;
		default:
			RCASSERT(0);
		}
		return 0;
    }

	RCLMFUNCTION owca_global owca_local::function_bind(const owca_local &obj) const
	{
		if (!function_is()) throw owca_exception(ExceptionCode::INVALID_PARAM_TYPE, "not a function");

		_check_vm(obj);
		owca_global o(_vm);
		switch(_object.mode()) {
		case VAR_FUNCTION:
			o._object.set_function_s(*_vm,obj._object,_object.get_function()->function());
			break;
		case VAR_FUNCTION_FAST:
			o._object.set_function_s(*_vm,obj._object,_object.get_function_fast().fnc);
			break;
		default:
			RCASSERT(0);
		}
		return o;
	}

	RCLMFUNCTION owca_global owca_local::function_obj() const
	{
		if (!function_is()) throw owca_exception(ExceptionCode::INVALID_PARAM_TYPE, "not a function");

		owca_global o(_vm);
		switch(_object.mode()) {
		case VAR_FUNCTION_FAST:
			o._object.set_object_null(_object.get_function_fast().slf);
			break;
		case VAR_FUNCTION:
			o._object=_object.get_function()->slf;
			break;
		default:
			RCASSERT(0);
		}
		o._object.gc_acquire();
		return o;
	}

	RCLMFUNCTION owca_global owca_local::function_member_of() const
	{
		if (!function_is()) throw owca_exception(ExceptionCode::INVALID_PARAM_TYPE, "not a function");

		owca_global o(_vm);
		exec_object *co=NULL;
		switch(_object.mode()) {
		case VAR_FUNCTION:
			co=_object.get_function()->function()->classobject();
			break;
		case VAR_FUNCTION_FAST:
			co=_object.get_function_fast().fnc->classobject();
			break;
		default:
			RCASSERT(0);
		}
		if (co) {
			o._object.set_object(co);
			co->gc_acquire();
		}
		return o;
	}

	bool owca_local::get(owca_int &i) const
	{
		switch(_object.mode()) {
		case VAR_INT: i=_object.get_int(); return true;
		case VAR_REAL: {
			owca_int ii=(owca_int)_object.get_real();
			if ((owca_real)ii==_object.get_real()) {
				i=ii;
				return true;
			}
			break; }
		case VAR_NULL:
		case VAR_BOOL:
		case VAR_STRING:
		case VAR_GENERATOR:
		case VAR_PROPERTY:
		case VAR_FUNCTION:
		case VAR_FUNCTION_FAST:
		case VAR_OBJECT:
		case VAR_NAMESPACE:
		case VAR_WEAK_REF:
		case VAR_COUNT:
		case VAR_HASH_DELETED:
		case VAR_HASH_EMPTY:
		case VAR_NO_DEF_VALUE:
		case VAR_NO_PARAM_GIVEN:
		case VAR_UNDEFINED:
			break;
		}
		return false;
	}

	bool owca_local::get(owca_real &r) const
	{
		switch(_object.mode()) {
		case VAR_REAL: r=_object.get_real(); return true;
		case VAR_INT: {
			owca_real rr=(owca_real)_object.get_int();
			if ((owca_int)rr==_object.get_int()) {
				r=rr;
				return true;
			}
			break; }
		case VAR_NULL:
		case VAR_BOOL:
		case VAR_STRING:
		case VAR_GENERATOR:
		case VAR_PROPERTY:
		case VAR_FUNCTION:
		case VAR_FUNCTION_FAST:
		case VAR_OBJECT:
		case VAR_NAMESPACE:
		case VAR_WEAK_REF:
		case VAR_COUNT:
		case VAR_HASH_DELETED:
		case VAR_HASH_EMPTY:
		case VAR_NO_DEF_VALUE:
		case VAR_NO_PARAM_GIVEN:
		case VAR_UNDEFINED:
			break;
		}
		return false;
	}

	bool owca_local::get(owca_map &m) const
	{
		if (map_is()) {
			m=map_get();
			return true;
		}
		return false;
	}

	bool owca_local::get(owca_list &l) const
	{
		if (list_is()) {
			l=list_get();
			return true;
		}
		return false;
	}

	bool owca_local::get(owca_tuple &t) const
	{
		if (tuple_is()) {
			t=tuple_get();
			return true;
		}
		return false;
	}

	bool owca_local::get(bool &b) const
	{
		switch(_object.mode()) {
		case VAR_BOOL: b=_object.get_bool(); return true;
		case VAR_NULL:
		case VAR_INT:
		case VAR_REAL:
		case VAR_STRING:
		case VAR_GENERATOR:
		case VAR_PROPERTY:
		case VAR_FUNCTION:
		case VAR_FUNCTION_FAST:
		case VAR_OBJECT:
		case VAR_NAMESPACE:
		case VAR_WEAK_REF:
		case VAR_COUNT:
		case VAR_HASH_DELETED:
		case VAR_HASH_EMPTY:
		case VAR_NO_DEF_VALUE:
		case VAR_NO_PARAM_GIVEN:
		case VAR_UNDEFINED:
			break;
		}
		return false;
	}

	bool owca_local::get(owca_string &s) const
	{
		switch(_object.mode()) {
		case VAR_STRING: s=string_get(); return true;
		case VAR_NULL:
		case VAR_INT:
		case VAR_REAL:
		case VAR_BOOL:
		case VAR_GENERATOR:
		case VAR_PROPERTY:
		case VAR_FUNCTION:
		case VAR_FUNCTION_FAST:
		case VAR_OBJECT:
		case VAR_NAMESPACE:
		case VAR_WEAK_REF:
		case VAR_COUNT:
		case VAR_HASH_DELETED:
		case VAR_HASH_EMPTY:
		case VAR_NO_DEF_VALUE:
		case VAR_NO_PARAM_GIVEN:
		case VAR_UNDEFINED:
			break;
		}
		return false;
	}

	bool owca_local::namespace_is() const
	{
		return _object.mode()==VAR_NAMESPACE;
	}

	owca_namespace owca_local::namespace_get() const
	{
		if (!namespace_is()) throw owca_exception(ExceptionCode::INVALID_PARAM_TYPE, "not a namespace");
		return _object.get_namespace()->generate();
	}

	void owca_local::namespace_set(const owca_namespace &n)
	{
		_object.gc_release(*_vm);
		_update_vm(&n.ns->vm);
		_object.set_namespace(n.ns);
		n.ns->gc_acquire();
	}

	bool owca_local::map_is() const
	{
		return _object.mode()==VAR_OBJECT && _vm->data_from_object<exec_map_object>(_object.get_object())!=NULL;
	}

	owca_map owca_local::map_get() const
	{
		if (!map_is()) throw owca_exception(ExceptionCode::INVALID_PARAM_TYPE, "not a map");
		exec_map_object *o=_vm->data_from_object<exec_map_object>(_object.get_object());
		owca_map m;
		m.obj=_object.get_object();
		m.vm=_vm;
		m.mo=o;
		return m;
	}

	void owca_local::map_set(const owca_map &a)
	{
		_object.gc_release(*_vm);
		_update_vm(a.vm);
		_object.set_object(a.obj);
		a.obj->gc_acquire();
	}

	bool owca_local::list_is() const
	{
		return _object.mode()==VAR_OBJECT && _vm->data_from_object<exec_array_object>(_object.get_object())!=NULL;
	}

	owca_list owca_local::list_get() const
	{
		if (!list_is()) throw owca_exception(ExceptionCode::INVALID_PARAM_TYPE, "not a list");
		exec_array_object *o=_vm->data_from_object<exec_array_object>(_object.get_object());
		owca_list m;
		m.obj=_object.get_object();
		m.vm=_vm;
		m.ao=o;
		return m;
	}

	void owca_local::list_set(const owca_list &a)
	{
		_object.gc_release(*_vm);
		_update_vm(a.vm);
		_object.set_object(a.obj);
		a.obj->gc_acquire();
	}

	bool owca_local::tuple_is() const
	{
		return _object.mode()==VAR_OBJECT && _vm->data_from_object<exec_tuple_object>(_object.get_object())!=NULL;
	}

	owca_tuple owca_local::tuple_get() const
	{
		if (!tuple_is()) throw owca_exception(ExceptionCode::INVALID_PARAM_TYPE, "not a tuple");
		exec_tuple_object *o=_vm->data_from_object<exec_tuple_object>(_object.get_object());
		owca_tuple m;
		m.obj=_object.get_object();
		m.vm=_vm;
		m.to=o;
		return m;
	}

	void owca_local::tuple_set(const owca_tuple &a)
	{
		_object.gc_release(*_vm);
		_update_vm(a.vm);
		_object.set_object(a.obj);
		a.obj->gc_acquire();
	}

	RCLMFUNCTION bool owca_local::is(const owca_local &p) const
	{
		return _object.is_same(p._object);
	}

	owca_global::owca_global(owca_vm &vm) : _next(NULL),_prev(NULL) { _vm=vm.vm; _attach(); }

	void owca_local::gc_mark(const gc_iteration &g) const
	{
		_object.gc_mark(g);
	}

	bool owca_local::bool_is() const { return _object.mode()==__owca__::VAR_BOOL; }
	bool owca_local::bool_get() const { return _object.get_bool(); }
	void owca_local::bool_set(bool b) { _object.gc_release(*_vm); _object.set_bool(b); }
	bool owca_local::string_is() const { return _object.mode()==__owca__::VAR_STRING; }
	void owca_local::int_set(owca_int i) { _object.gc_release(*_vm); _object.set_int(i); }
	owca_int owca_local::int_get() const { return _object.get_int(); }
	bool owca_local::int_is() const { return _object.mode()==__owca__::VAR_INT; }
	void owca_local::real_set(owca_real r) { _object.gc_release(*_vm); _object.set_real(r); }
	owca_real owca_local::real_get() const { return _object.get_real(); }
	bool owca_local::real_is() const { return _object.mode()==__owca__::VAR_REAL; }
	void owca_local::null_set(bool no_value) { _object.gc_release(*_vm); _object.set_null(no_value); }
	bool owca_local::null_is() const { return _object.mode()==__owca__::VAR_NULL; }
	bool owca_local::no_value_is() const { return _object.is_no_return_value(); }
	bool owca_local::function_is() const { return _object.mode()==__owca__::VAR_FUNCTION || _object.mode()==__owca__::VAR_FUNCTION_FAST; }
	bool owca_local::object_is() const { return _object.mode()==__owca__::VAR_OBJECT; }
	bool owca_local::type_is() const { return _object.mode()==__owca__::VAR_OBJECT && _object.get_object()->is_type(); }

	owca_string owca_local::type_name() const
	{
		if (!type_is()) throw owca_exception(ExceptionCode::INVALID_PARAM_TYPE, "not a type");
		owca_string s(_vm,_object.get_object()->CO().name);
		_object.get_object()->CO().name->gc_acquire();
		s._destroy=true;
		return s;
	}

	namespace __owca__ {
		std::string str_function(exec_function_ptr *f)
		{
			return (f->classobject() ? (f->classobject()->CO().name->str()+".") : "")+
				(f->name()->data_size()>0 ? f->name()->str() : "<unnamed>");
		}
	}

    std::string owca_local::type_str() const
    {
        return virtual_machine::to_stdstring_type(_object);
    }

	std::string owca_local::str() const
	{
		switch(_object.mode()) {
		case VAR_NULL: return "null";
		case VAR_BOOL: return _object.get_bool() ? "true" : "false";
		case VAR_INT: return int_to_string(_object.get_int());
		case VAR_REAL: return real_to_string(_object.get_real());
		case VAR_STRING: return _object.get_string()->str();
		case VAR_GENERATOR: return "generator object";
		case VAR_PROPERTY: return "property "+str_function(_object.get_property()->read ? _object.get_property()->read : _object.get_property()->write);
		case VAR_FUNCTION: return "function "+str_function(_object.get_function()->function());
		case VAR_FUNCTION_FAST: return "function "+str_function(_object.get_function_fast().fnc);
		case VAR_OBJECT: return "object of type "+_object.get_object()->type()->CO().name->str();
		case VAR_NAMESPACE: return "namespace object";
		case VAR_WEAK_REF: return "weakref object";
		default: RCASSERT(0);
		}
		return "";
	}

	RCLMFUNCTION owca_global owca_local::_call(const __owca__::exec_variable *params, unsigned int size) const
	{
		exec_variable tmp;
		auto pop = _vm->push_execution_stack();
		_vm->prepare_call_function(&tmp, _object, params, size);
		_vm->execute_stack();
		return { *_vm, tmp };
	}

	RCLMFUNCTION owca_global owca_local::_call(const __owca__::exec_variable *params, unsigned int size, const __owca__::exec_variable &list, const __owca__::exec_variable &map) const
	{
		__owca__::virtual_machine::keyword_param_iterator empty;
		exec_variable tmp;
		auto pop = _vm->push_execution_stack();
		_vm->prepare_call_function(&tmp, _object, list.mode() == VAR_NULL ? NULL : &list, map.mode() == VAR_NULL ? NULL : &map, empty, params, size, NULL);
		_vm->execute_stack();
		return { *_vm, tmp };
	}

	RCLMFUNCTION owca_global owca_local::_call(const callparams &cp) const
	{
		exec_variable tmp;
		auto pop = _vm->push_execution_stack();
		_vm->prepare_call_function(&tmp,_object,cp,NULL);
		_vm->execute_stack();
		return { *_vm, tmp };
	}


	owca_global owca_local::get_member(const owca_string &member) const
	{
		if (_vm==NULL) throw owca_exception(ExceptionCode::INVALID_VM, "VM not set");
		auto pop = _vm->push_execution_stack();

		if (!internal_class::_check_name(member.data(), (unsigned int)member.data_size())) throw owca_exception(ExceptionCode::INVALID_IDENT, OWCA_ERROR_FORMAT1("'%1' is not a valid identificator", member.str()));

		if (_object.mode() == VAR_NAMESPACE) {
			exec_variable tmp;
			bool b=_object.get_namespace()->get_variable(member._ss,tmp);
			if (!b) {
				return owca_local ::null_no_value;
			}
			return { _object.get_namespace()->vm, tmp };
		}
		else {
			exec_variable tmp;
			lookupreturnvalue lrv=_object.lookup_read(tmp,*_vm,member._ss,true);
			switch(lrv.type()) {
			case lookupreturnvalue::LOOKUP_FOUND:
				return { *_vm, tmp };
			case lookupreturnvalue::LOOKUP_NOT_FOUND:
				return owca_local::null_no_value;
			case lookupreturnvalue::LOOKUP_FUNCTION_CALL:
				try {
					switch (_vm->execution_stack->peek_frame()->return_handling_mode) {
					case vm_execution_stack_elem_base::RETURN_HANDLING_NONE:
						_vm->execution_stack->peek_frame()->return_handling_mode = vm_execution_stack_elem_base::RETURN_HANDLING_NONE_FORCE_RETURN;
						break;
					case vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_INIT_EXCEPTION:
						RCASSERT(_vm->execution_stack->peek_frame()->return_value == &_vm->execution_exception_object_temp);
						break;
					default:
						RCASSERT(0);
					}
					_vm->execute_stack();
				}
				catch (...) {
					_vm->pop_execution_stack_impl();
					throw;
				}
				return { *_vm, tmp };
			default: RCASSERT(0);
			}
		}
		RCASSERT(0);
		return {};
	}

	owca_global owca_local::set_member(const owca_string &member, const owca_global &val) const
	{
		if (_vm==NULL) throw owca_exception(ExceptionCode::INVALID_VM, "VM not set");
		auto pop = _vm->push_execution_stack();

		if (!internal_class::_check_name(member.data(), (unsigned int)member.data_size())) throw owca_exception(ExceptionCode::INVALID_IDENT, OWCA_ERROR_FORMAT1("'%1' is not a valid identificator", member.str()));
		_check_vm(val);

		if (_object.mode() == VAR_NAMESPACE) {
			exec_variable tmp;
			bool b=_object.get_namespace()->get_variable(member._ss,tmp);
			if (!b) {
				return owca_local::null_no_value;
			}
			return owca_global{ _object.get_namespace()->vm, tmp };
		}
		else {
			exec_variable tmp;
			lookupreturnvalue lrv=_object.lookup_write(tmp,*_vm,member._ss,val._object,true);
			switch (lrv.type()) {
			case lookupreturnvalue::LOOKUP_FOUND: return owca_global{ *_vm, tmp };
			case lookupreturnvalue::LOOKUP_NOT_FOUND: return owca_local::null_no_value;
			case lookupreturnvalue::LOOKUP_FUNCTION_CALL:
				try {
					switch (_vm->execution_stack->peek_frame()->return_handling_mode) {
					case vm_execution_stack_elem_base::RETURN_HANDLING_NONE:
						_vm->execution_stack->peek_frame()->return_handling_mode = vm_execution_stack_elem_base::RETURN_HANDLING_NONE_FORCE_RETURN;
						break;
					case vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_INIT_EXCEPTION:
						RCASSERT(_vm->execution_stack->peek_frame()->return_value == &_vm->execution_exception_object_temp);
						break;
					default:
						RCASSERT(0);
					}
					return owca_global{ *_vm, tmp };
				}
				catch (...) {
					_vm->pop_execution_stack_impl();
					throw;
				}
			}
			RCASSERT(0);
		}
		RCASSERT(0);
		return {};
	}

	owca_global owca_local::call() const
	{
		if (_vm==NULL) throw owca_exception(ExceptionCode::INVALID_VM, "VM not set");
		auto pop = _vm->push_execution_stack();
		return _call(NULL,0);
	}

	owca_global owca_local::call(const owca_global &p1) const
	{
		if (_vm==NULL) throw owca_exception(ExceptionCode::INVALID_VM, "VM not set");
		auto pop = _vm->push_execution_stack();
		_check_vm(p1);
		return _call(&p1._object,1);
	}

	owca_global owca_local::call(const owca_global &p1, const owca_global &p2) const
	{
		if (_vm==NULL) throw owca_exception(ExceptionCode::INVALID_VM, "VM not set");
		auto pop = _vm->push_execution_stack();
		_check_vm(p1);
		_check_vm(p2);
		exec_variable arr[] = {p1._object, p2._object};
		return _call(arr,2);
	}

	owca_global owca_local::call(const owca_global &p1, const owca_global &p2, const owca_global &p3) const
	{
		if (_vm==NULL) throw owca_exception(ExceptionCode::INVALID_VM, "VM not set");
		auto pop = _vm->push_execution_stack();
		_check_vm(p1);
		_check_vm(p2);
		_check_vm(p3);
		exec_variable arr[] = { p1._object, p2._object, p3._object };
		return _call(arr,3);
	}

	owca_global owca_local::call(const owca_global &p1, const owca_global &p2, const owca_global &p3, const owca_global &p4) const
	{
		if (_vm==NULL) throw owca_exception(ExceptionCode::INVALID_VM, "VM not set");
		auto pop = _vm->push_execution_stack();
		_check_vm(p1);
		_check_vm(p2);
		_check_vm(p3);
		_check_vm(p4);
		exec_variable arr[] = {p1._object, p2._object, p3._object, p4._object};
		return _call(arr,4);
	}

	owca_global owca_local::call(const owca_call_parameters &cp) const
	{
		if (_vm==NULL) throw owca_exception(ExceptionCode::INVALID_VM, "VM not set");
		auto pop = _vm->push_execution_stack();
		_check_vm(cp.list_parameter);
		_check_vm(cp.map_parameter);

		if (cp.parameters.empty()) return _call(NULL,0,cp.list_parameter._object,cp.map_parameter._object);
		exec_variable *tmp=new exec_variable[cp.parameters.size()];
		for(unsigned int i=0;i<cp.parameters.size();++i) {
			_check_vm(cp.parameters[i]);
			tmp[i]=cp.parameters[i]._object;
		}
		return _call(tmp,(unsigned int)cp.parameters.size(),cp.list_parameter._object,cp.map_parameter._object);
	}

	owca_global owca_local::call(const owca_parameters &cp) const
	{
		if (_vm==NULL) throw owca_exception(ExceptionCode::INVALID_VM, "VM not set");
		auto pop = _vm->push_execution_stack();
		_check_vm(cp.vm);
		return _call(*cp.ci);
	}

	//bool owca_local::convert(owca_global &exception_object, owca_vm &vm, owca_int &a, const char *pname, owca_int minval, owca_int maxval)
	//{
	//	if (!convert(exception_object,vm,a,pname)) return false;
	//	if (a<minval) {
	//		exception_object=vm.construct_builtin_exception(INTEGER_OUT_OF_BOUNDS,OWCA_ERROR_FORMAT2("number %1 is lower than required minimum %2",int_to_string(a),int_to_string(minval)));
	//		return false;
	//	}
	//	if (a>maxval) {
	//		exception_object=vm.construct_builtin_exception(INTEGER_OUT_OF_BOUNDS,OWCA_ERROR_FORMAT2("number %1 is higher than required maximum %2",int_to_string(a),int_to_string(maxval)));
	//		return false;
	//	}
	//	return true;
	//}

}
