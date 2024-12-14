#ifndef _RC_Y_LOCAL_H
#define _RC_Y_LOCAL_H

#include "returnvalue.h"
#include "exec_variable.h"
#include "structinfo.h"
#include "exectype.h"

namespace owca {
	class owca_class;
	class owca_global;
	class owca_local;
	class owca_parameters;
	struct owca_call_parameters;
	class owca_function_return_value;
	class owca_string;
	class owca_vm;
	class owca_list;
	class owca_map;
	class owca_tuple;
	class owca_string_buffer;
	class owca_namespace;
	class owca_user_function_base_object;
	namespace __owca__ {
		struct defval;
		class exec_variable;
		class internal_class;
		class virtual_machine;
		class obj_constructor_base;
		class owca_internal_string;
		class local_obj_constructor;
		class exec_namespace;

		template <class A, class B, bool RETVAL> class user_function_base;
		template <class A, class B> class user_function_base_simple;
	}
}

namespace owca {
	namespace __owca__ {
		template <class A> struct __type_name__ { enum { INDEX=0xff }; };
		template <> struct __type_name__<owca_string> { enum { INDEX=0 }; };
		template <> struct __type_name__<owca_int> { enum { INDEX=1 }; };
		template <> struct __type_name__<owca_real> { enum { INDEX=2 }; };
		template <> struct __type_name__<bool> { enum { INDEX=3 }; };
		template <> struct __type_name__<owca_list> { enum { INDEX=4 }; };
		template <> struct __type_name__<owca_map> { enum { INDEX=5 }; };
		template <> struct __type_name__<owca_tuple> { enum { INDEX=6 }; };
	}
	class owca_local {
		template <class A, class B, bool RETVAL> friend class __owca__::user_function_base;
		template <class A, class B> friend class __owca__::user_function_base_simple;
		friend class owca_user_function_base_object;
		friend class __owca__::virtual_machine;
		friend class __owca__::local_obj_constructor;
		friend class owca_parameters;
		friend class owca_namespace;
		friend class owca_list;
		friend class owca_map;
		friend class owca_tuple;
		friend class owca_string;
		friend class owca_string_buffer;
		friend class __owca__::obj_constructor_base;
		friend class owca_vm;
		friend class __owca__::internal_class;
		friend class owca_class;
		friend class owca_global;
		friend struct __owca__::defval;
		friend class __owca__::exec_namespace;

	protected:
		__owca__::virtual_machine *_vm;
	private:
		__owca__::exec_variable _object;
		owca_local(__owca__::virtual_machine *vm_) : _vm(vm_) { _object.reset(); }
		owca_local(__owca__::virtual_machine &vm_, const __owca__::exec_variable &object_) : _vm(&vm_),_object(object_) { }
		owca_local(__owca__::exectype etyp) : _vm(NULL) { _object.setmode(etyp); }

		DLLEXPORT virtual void _update_vm(__owca__::virtual_machine *vm);

		DLLEXPORT void *_member_data(const __owca__::structid_type &type) const;

		void _check_vm(const owca_local &) const;
		void _check_vm(__owca__::virtual_machine *) const;

		void _call_prepare_after(owca_global &result) const;
		owca_function_return_value _call(owca_global &result, const __owca__::exec_variable &fncval, const __owca__::exec_variable *params, unsigned int size) const;
		owca_function_return_value _call(owca_global &result, const __owca__::exec_variable &fncval, const __owca__::exec_variable *params, unsigned int size, const __owca__::exec_variable &list, const __owca__::exec_variable &map) const;
		owca_function_return_value _call(owca_global &result, const __owca__::exec_variable &fncval, const __owca__::callparams &cp) const;
		owca_function_return_value _finalize_get_set_member(owca_function_return_value r) const;
		void _execute(void) const;
	public:
		owca_local(bool b) : _vm(NULL) { _object.set_bool(b); }
		DLLEXPORT owca_local(char c);
		owca_local(unsigned int i) : _vm(NULL) { _object.set_int(i); }
		owca_local(int i) : _vm(NULL) { _object.set_int(i); }
		owca_local(unsigned long long i) : _vm(NULL) { _object.set_int(i); }
		owca_local(long long i) : _vm(NULL) { _object.set_int(i); }
		owca_local(double r) : _vm(NULL) { _object.set_real(r); }
		owca_local(long double r) : _vm(NULL) { _object.set_real(r); }
		DLLEXPORT owca_local(const char *);
		DLLEXPORT owca_local(const owca_namespace &);
		DLLEXPORT owca_local(const owca_string &);
		DLLEXPORT owca_local(const std::string &);
		DLLEXPORT owca_local(const owca_list &);
		DLLEXPORT owca_local(const owca_map &);
		DLLEXPORT owca_local(const owca_tuple &);
		owca_local() : _vm(NULL) { _object.set_null(); }
		DLLEXPORT owca_local(const owca_local &);
		virtual ~owca_local() { _object.gc_release(*_vm); }
		DLLEXPORT const owca_local &operator = (const owca_local &);

		DLLEXPORT owca_vm *vm() const;
		DLLEXPORT owca_function_return_value get_member(owca_global &result, const owca_string &member) const;
		DLLEXPORT owca_function_return_value set_member(owca_global &result, const owca_string &member, const owca_global &val) const;
		DLLEXPORT owca_function_return_value call(owca_global &result) const;
		DLLEXPORT owca_function_return_value call(owca_global &result, const owca_global &p1) const;
		DLLEXPORT owca_function_return_value call(owca_global &result, const owca_global &p1, const owca_global &p2) const;
		DLLEXPORT owca_function_return_value call(owca_global &result, const owca_global &p1, const owca_global &p2, const owca_global &p3) const;
		DLLEXPORT owca_function_return_value call(owca_global &result, const owca_global &p1, const owca_global &p2, const owca_global &p3, const owca_global &p4) const;
		DLLEXPORT owca_function_return_value call(owca_global &result, const owca_call_parameters &cp) const;
		DLLEXPORT owca_function_return_value call(owca_global &result, const owca_parameters &cp) const;
		DLLEXPORT owca_function_return_value prepare_get_member(owca_global &result, const owca_string &member) const;
		DLLEXPORT owca_function_return_value prepare_set_member(owca_global &result, const owca_string &member, const owca_global &val) const;
		DLLEXPORT owca_function_return_value prepare_call(owca_global &result) const;
		DLLEXPORT owca_function_return_value prepare_call(owca_global &result, const owca_global &p1) const;
		DLLEXPORT owca_function_return_value prepare_call(owca_global &result, const owca_global &p1, const owca_global &p2) const;
		DLLEXPORT owca_function_return_value prepare_call(owca_global &result, const owca_global &p1, const owca_global &p2, const owca_global &p3) const;
		DLLEXPORT owca_function_return_value prepare_call(owca_global &result, const owca_global &p1, const owca_global &p2, const owca_global &p3, const owca_global &p4) const;
		DLLEXPORT owca_function_return_value prepare_call(owca_global &result, const owca_call_parameters &cp) const;
		DLLEXPORT owca_function_return_value prepare_call(owca_global &result, const owca_parameters &cp) const;

		DLLEXPORT void bind(owca_vm &vm);
		DLLEXPORT owca_global type() const;
		template <class A> bool get(A *&ptr) const {
			if (_vm) {
				ptr=(A*)_member_data(__owca__::structinfo::structid<A*>());
				return ptr!=NULL;
			}
			return false;
		}
		DLLEXPORT bool get(owca_int &) const;
		DLLEXPORT bool get(owca_real &) const;
		DLLEXPORT bool get(owca_map &) const;
		DLLEXPORT bool get(owca_list &) const;
		DLLEXPORT bool get(owca_tuple &) const;
		DLLEXPORT bool get(bool &) const;
		DLLEXPORT bool get(owca_string &) const;

		DLLEXPORT bool namespace_is() const;
		DLLEXPORT owca_namespace namespace_get() const;
		DLLEXPORT void namespace_set(const owca_namespace &);
		DLLEXPORT bool map_is() const;
		DLLEXPORT owca_map map_get() const;
		DLLEXPORT void map_set(const owca_map &);
		DLLEXPORT bool list_is() const;
		DLLEXPORT owca_list list_get() const;
		DLLEXPORT void list_set(const owca_list &);
		DLLEXPORT bool tuple_is() const;
		DLLEXPORT owca_tuple tuple_get() const;
		DLLEXPORT void tuple_set(const owca_tuple &);
		DLLEXPORT bool bool_is() const;
		DLLEXPORT bool bool_get() const;
		DLLEXPORT void bool_set(bool b);
		DLLEXPORT bool string_is() const;
		DLLEXPORT void string_set(const char *);
		DLLEXPORT void string_set(const char *, unsigned int size);
		DLLEXPORT void string_set(const std::string &);
		DLLEXPORT const char *string_get(unsigned int &size) const;
		DLLEXPORT owca_string string_get() const;
		DLLEXPORT void int_set(owca_int i);
		DLLEXPORT owca_int int_get() const;
		DLLEXPORT bool int_is() const;
		DLLEXPORT void real_set(owca_real r);
		DLLEXPORT owca_real real_get() const;
		DLLEXPORT bool real_is() const;
		DLLEXPORT void null_set();
		DLLEXPORT bool null_is() const;
		DLLEXPORT bool function_is() const;
		DLLEXPORT bool object_is() const;
		DLLEXPORT owca_global function_bind(const owca_local &obj) const;
		DLLEXPORT owca_global function_obj() const;
		DLLEXPORT owca_global function_member_of() const;
		DLLEXPORT std::string function_name() const;
		DLLEXPORT std::string function_file_name() const;
		DLLEXPORT unsigned int function_file_line() const;
		DLLEXPORT bool type_is() const;
		DLLEXPORT owca_string type_name() const;

		DLLEXPORT bool is(const owca_local &) const;

		DLLEXPORT std::string str() const;
        DLLEXPORT std::string type_str() const;

		//template <class A> A *member_data() const { return (A*)_member_data(__owca__::structinfo::structid<A*>()); }

		DLLEXPORT void gc_mark(const gc_iteration &g) const;

		DLLEXPORT static owca_local null;

		template <class A, typename VM> bool convert(owca_global &exception_object, VM &vm, A &a, const char *pname) {
			static const char *typenames[]={
				"a string","an integer","a real","a boolean","a list","a map","a tuple" };
			if (get(a)) return true;
			if (__owca__::__type_name__<A>::INDEX!=0xff) {
				const char *tname=typenames[__owca__::__type_name__<A>::INDEX];
				exception_object = vm.construct_builtin_exception(ExceptionCode::INVALID_PARAM_TYPE,OWCA_ERROR_FORMAT2("parameter %1 is not %2",pname,tname));
			}
			else {
				exception_object = vm.construct_builtin_exception(ExceptionCode::INVALID_PARAM_TYPE,OWCA_ERROR_FORMAT1("parameter %1 is of an invalid type", pname));
			}
			return false;
		}
	};
}

#endif
