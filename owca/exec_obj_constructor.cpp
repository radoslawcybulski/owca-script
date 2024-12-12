#include "stdafx.h"
#include "base.h"
#include "obj_constructor_base.h"
#include "virtualmachine.h"
#include "exec_function_ptr.h"
#include "exec_property.h"
#include "exec_string.h"
#include "global.h"
#include "string.h"
#include "map.h"
#include "list.h"
#include "tuple.h"
#include "exec_object.h"
#include "exec_namespace.h"

namespace owca { namespace __owca__ {
	obj_constructor_function::obj_constructor_function(exec_namespace *ns, owca_internal_string *ident) : obj_constructor_base(ns->vm,ident),nspace(ns)
	{
	}

	owca_parameters _generate_parameters(virtual_machine &vm, const __owca__::callparams &ci)
	{
		owca_parameters cp;
		cp._init(vm,ci);
		return cp;
	}

	owca_global obj_constructor_base::read() const
	{
		used=true;
		g._update_vm(&_vm);
		_read(g._object);
		return g;
	}

	obj_constructor_base::operator const owca_global &()
	{
		used=true;
		g._update_vm(&_vm);
		_read(g._object);
		return g;
	}

	owca_vm &obj_constructor_base::__get(virtual_machine &vm)
	{
		return *vm.owner_vm;
	}

	void obj_constructor_base::set(exec_function_ptr *o)
	{
		exec_variable v;
		v.set_function_fast(o);
		write(v);
	}

	void obj_constructor_base::set(exec_property *o)
	{
		exec_variable v;
		v.set_property(o);
		write(v);
	}

	void obj_constructor_base::set(exec_object *o)
	{
		exec_variable v;
		v.set_object(o);
		write(v);
	}

	exec_variable &obj_constructor_base::_get_var(owca_global &g)
	{
		return g._object;
	}


	const owca_local &obj_constructor_base::operator = (const owca_local &l) {
		//l._object.gc_acquire();
		write(l._object); return l;
	}
	owca_global obj_constructor_base::type() const { return read().type(); }

	bool obj_constructor_base::map_is() const { return read().map_is(); }
	owca_map obj_constructor_base::map_get() const { return read().map_get(); }
	void obj_constructor_base::map_set(const owca_map &m) {
		exec_variable v;
		v.set_object(m.obj);
		//m.obj->gc_acquire();
		write(v);
	}
	bool obj_constructor_base::list_is() const { return read().list_is(); }
	owca_list obj_constructor_base::list_get() const { return read().list_get(); }
	void obj_constructor_base::list_set(const owca_list &l) {
		exec_variable v;
		v.set_object(l.obj);
		//l.obj->gc_acquire();
		write(v);
	}
	bool obj_constructor_base::tuple_is() const { return read().tuple_is(); }
	owca_tuple obj_constructor_base::tuple_get() const { return read().tuple_get(); }
	void obj_constructor_base::tuple_set(const owca_tuple &t) {
		exec_variable v;
		v.set_object(t.obj);
		//t.obj->gc_acquire();
		write(v);
	}

	bool obj_constructor_base::bool_is() const { return read().bool_is(); }
	bool obj_constructor_base::bool_get() const { return read().bool_get(); }
	void obj_constructor_base::bool_set(bool b) {
		exec_variable v;
		v.set_bool(b);
		write(v);
	}
	bool obj_constructor_base::string_is() const { return read().string_is(); }
	void obj_constructor_base::string_set(const char *s) {
		const char *e=s;
		while(*e) ++e;
		string_set(s, (unsigned int)(e - s));
	}
	void obj_constructor_base::string_set(const char *s, unsigned int size) {
		unsigned int index = 0;
		unsigned int char_count = owca_internal_string::calculate_char_count_and_missing_bytes_if_any(index, s, size);
		if (index != 0)
			throw owca_exception("invalid utf8 string");
		exec_variable v;
		v.set_string(_vm.allocate_string(s,size,char_count));
		write(v);
		v.gc_release(_vm);
	}
	void obj_constructor_base::string_set(const std::string &s) {
		string_set(s.c_str(), (unsigned int)s.size());
	}
	const char *obj_constructor_base::string_get(unsigned int &sz) const { return read().string_get(sz); }
	owca_string obj_constructor_base::string_get() const { return read().string_get(); }
	//owca_string obj_constructor_base::string_get() const { return read().string_get(); }
	void obj_constructor_base::int_set(owca_int i) {
		exec_variable v;
		v.set_int(i);
		write(v);
	}
	owca_int obj_constructor_base::int_get() const { return read().int_get(); }
	bool obj_constructor_base::int_is() const { return read().int_is(); }
	void obj_constructor_base::real_set(owca_real r) {
		exec_variable v;
		v.set_real(r);
		write(v);
	}
	owca_real obj_constructor_base::real_get() const { return read().real_get(); }
	bool obj_constructor_base::real_is()const  { return read().real_is(); }
	void obj_constructor_base::null_set() {
		exec_variable v;
		v.set_null();
		write(v);
	}
	bool obj_constructor_base::null_is() const { return read().null_is(); }
	bool obj_constructor_base::function_is() const { return read().function_is(); }
	owca_global obj_constructor_base::function_bind(const owca_local &obj) const { return read().function_bind(obj); }
	owca_global obj_constructor_base::function_obj() const { return read().function_obj(); }
	bool obj_constructor_base::is(const owca_local &s) const { return read().is(s); }
	bool obj_constructor_base::type_is() const { return read().type_is(); }
	owca_string obj_constructor_base::type_name() const { return read().type_name(); }
	std::string obj_constructor_base::str() const { return read().str(); }
	void obj_constructor_base::_update_vm(owca_global &g)
	{
		g._object.gc_release(*g._vm);
		g._update_vm(&_vm);
		g._object.reset();
	}

	obj_constructor_base::~obj_constructor_base() { RCASSERT(used); }

	owca_function_return_value obj_constructor_base::get_member(owca_global &res, const owca_string &callobject) const { return read().get_member(res,callobject); }
	owca_function_return_value obj_constructor_base::set_member(owca_global &res, const owca_string &callobject, const owca_global &val) const { return read().set_member(res,callobject,val); }
	owca_function_return_value obj_constructor_base::call(owca_global &res) const { return read().call(res); }
	owca_function_return_value obj_constructor_base::call(owca_global &res, const owca_global &p1) const { return read().call(res,p1); }
	owca_function_return_value obj_constructor_base::call(owca_global &res, const owca_global &p1, const owca_global &p2) const { return read().call(res,p1,p2); }
	owca_function_return_value obj_constructor_base::call(owca_global &res, const owca_global &p1, const owca_global &p2, const owca_global &p3) const { return read().call(res,p1,p2,p3); }
	owca_function_return_value obj_constructor_base::call(owca_global &res, const owca_global &p1, const owca_global &p2, const owca_global &p3, const owca_global &p4) const { return read().call(res,p1,p2,p3,p4); }
	owca_function_return_value obj_constructor_base::call(owca_global &res, const owca_call_parameters &cp) const { return read().call(res,cp); }
	owca_function_return_value obj_constructor_base::call(owca_global &res, const owca_parameters &cp) const { return read().call(res,cp); }

	owca_function_return_value obj_constructor_base::prepare_get_member(owca_global &res, const owca_string &callobject) const { return read().prepare_get_member(res,callobject); }
	owca_function_return_value obj_constructor_base::prepare_set_member(owca_global &res, const owca_string &callobject, const owca_global &val) const { return read().prepare_set_member(res,callobject,val); }
	owca_function_return_value obj_constructor_base::prepare_call(owca_global &res) const { return read().prepare_call(res); }
	owca_function_return_value obj_constructor_base::prepare_call(owca_global &res, const owca_global &p1) const { return read().prepare_call(res,p1); }
	owca_function_return_value obj_constructor_base::prepare_call(owca_global &res, const owca_global &p1, const owca_global &p2) const { return read().prepare_call(res,p1,p2); }
	owca_function_return_value obj_constructor_base::prepare_call(owca_global &res, const owca_global &p1, const owca_global &p2, const owca_global &p3) const { return read().prepare_call(res,p1,p2,p3); }
	owca_function_return_value obj_constructor_base::prepare_call(owca_global &res, const owca_global &p1, const owca_global &p2, const owca_global &p3, const owca_global &p4) const { return read().prepare_call(res,p1,p2,p3,p4); }
	owca_function_return_value obj_constructor_base::prepare_call(owca_global &res, const owca_call_parameters &cp) const { return read().prepare_call(res,cp); }
	owca_function_return_value obj_constructor_base::prepare_call(owca_global &res, const owca_parameters &cp) const { return read().prepare_call(res,cp); }

} }












