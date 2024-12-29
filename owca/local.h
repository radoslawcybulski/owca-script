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
	class owca_exception;
	namespace __owca__ {
		struct defval;
		class exec_variable;
		class internal_class;
		class virtual_machine;
		class obj_constructor_base;
		class local_obj_constructor;
		class owca_internal_string;
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

		OWCA_SCRIPT_DLLEXPORT virtual void _update_vm(__owca__::virtual_machine *vm);

		OWCA_SCRIPT_DLLEXPORT void *_member_data(const __owca__::structid_type &type) const;

		void _check_vm(const owca_local &) const;
		void _check_vm(__owca__::virtual_machine *) const;

		owca_global _call(const __owca__::exec_variable *params, unsigned int size) const;
		owca_global _call(const __owca__::exec_variable *params, unsigned int size, const __owca__::exec_variable &list, const __owca__::exec_variable &map) const;
		owca_global _call(const __owca__::callparams &cp) const;
		void _execute(void) const;

		void _prepare_get_member(const owca_string &member) const;
		void _prepare_set_member(const owca_string &member, const owca_global &val) const;

		static owca_exception _construct_invalid_param_exception(std::string msg);
	public:
		owca_local(bool b) : _vm(NULL) { _object.set_bool(b); }
		OWCA_SCRIPT_DLLEXPORT owca_local(char c);
		owca_local(unsigned int i) : _vm(NULL) { _object.set_int(i); }
		owca_local(int i) : _vm(NULL) { _object.set_int(i); }
		owca_local(unsigned long long i) : _vm(NULL) { _object.set_int(i); }
		owca_local(long long i) : _vm(NULL) { _object.set_int(i); }
		owca_local(unsigned long i) : _vm(NULL) { _object.set_int(i); }
		owca_local(long i) : _vm(NULL) { _object.set_int(i); }
		owca_local(double r) : _vm(NULL) { _object.set_real(r); }
		owca_local(long double r) : _vm(NULL) { _object.set_real(r); }
		OWCA_SCRIPT_DLLEXPORT owca_local(const char *);
		OWCA_SCRIPT_DLLEXPORT owca_local(const owca_namespace &);
		OWCA_SCRIPT_DLLEXPORT owca_local(const owca_string &);
		OWCA_SCRIPT_DLLEXPORT owca_local(const std::string &);
		OWCA_SCRIPT_DLLEXPORT owca_local(const owca_list &);
		OWCA_SCRIPT_DLLEXPORT owca_local(const owca_map &);
		OWCA_SCRIPT_DLLEXPORT owca_local(const owca_tuple &);
		owca_local() : _vm(NULL) { _object.set_null(); }
		OWCA_SCRIPT_DLLEXPORT owca_local(const owca_local &);
		virtual ~owca_local() { _object.gc_release(*_vm); }
		OWCA_SCRIPT_DLLEXPORT const owca_local &operator = (const owca_local &);

		OWCA_SCRIPT_DLLEXPORT owca_vm *vm() const;
		OWCA_SCRIPT_DLLEXPORT owca_global get_member(const owca_string &member) const;
		OWCA_SCRIPT_DLLEXPORT owca_global set_member(const owca_string &member, const owca_global &val) const;
		OWCA_SCRIPT_DLLEXPORT owca_global call() const;
		OWCA_SCRIPT_DLLEXPORT owca_global call(const owca_global &p1) const;
		OWCA_SCRIPT_DLLEXPORT owca_global call(const owca_global &p1, const owca_global &p2) const;
		OWCA_SCRIPT_DLLEXPORT owca_global call(const owca_global &p1, const owca_global &p2, const owca_global &p3) const;
		OWCA_SCRIPT_DLLEXPORT owca_global call(const owca_global &p1, const owca_global &p2, const owca_global &p3, const owca_global &p4) const;
		OWCA_SCRIPT_DLLEXPORT owca_global call(const owca_call_parameters &cp) const;
		OWCA_SCRIPT_DLLEXPORT owca_global call(const owca_parameters &cp) const;

		OWCA_SCRIPT_DLLEXPORT void bind(owca_vm &vm);
		OWCA_SCRIPT_DLLEXPORT owca_global type() const;
		template <class A> bool get(A *&ptr) const {
			if (_vm) {
				ptr=(A*)_member_data(__owca__::structinfo::structid<A*>());
				return ptr!=NULL;
			}
			return false;
		}
		OWCA_SCRIPT_DLLEXPORT bool get(owca_int &) const;
		OWCA_SCRIPT_DLLEXPORT bool get(owca_real &) const;
		OWCA_SCRIPT_DLLEXPORT bool get(owca_map &) const;
		OWCA_SCRIPT_DLLEXPORT bool get(owca_list &) const;
		OWCA_SCRIPT_DLLEXPORT bool get(owca_tuple &) const;
		OWCA_SCRIPT_DLLEXPORT bool get(bool &) const;
		OWCA_SCRIPT_DLLEXPORT bool get(owca_string &) const;

		OWCA_SCRIPT_DLLEXPORT bool namespace_is() const;
		OWCA_SCRIPT_DLLEXPORT owca_namespace namespace_get() const;
		OWCA_SCRIPT_DLLEXPORT void namespace_set(const owca_namespace &);
		OWCA_SCRIPT_DLLEXPORT bool map_is() const;
		OWCA_SCRIPT_DLLEXPORT owca_map map_get() const;
		OWCA_SCRIPT_DLLEXPORT void map_set(const owca_map &);
		OWCA_SCRIPT_DLLEXPORT bool list_is() const;
		OWCA_SCRIPT_DLLEXPORT owca_list list_get() const;
		OWCA_SCRIPT_DLLEXPORT void list_set(const owca_list &);
		OWCA_SCRIPT_DLLEXPORT bool tuple_is() const;
		OWCA_SCRIPT_DLLEXPORT owca_tuple tuple_get() const;
		OWCA_SCRIPT_DLLEXPORT void tuple_set(const owca_tuple &);
		OWCA_SCRIPT_DLLEXPORT bool bool_is() const;
		OWCA_SCRIPT_DLLEXPORT bool bool_get() const;
		OWCA_SCRIPT_DLLEXPORT void bool_set(bool b);
		OWCA_SCRIPT_DLLEXPORT bool string_is() const;
		OWCA_SCRIPT_DLLEXPORT void string_set(const char *);
		OWCA_SCRIPT_DLLEXPORT void string_set(const char *, unsigned int size);
		OWCA_SCRIPT_DLLEXPORT void string_set(const std::string &);
		OWCA_SCRIPT_DLLEXPORT const char *string_get(unsigned int &size) const;
		OWCA_SCRIPT_DLLEXPORT owca_string string_get() const;
		OWCA_SCRIPT_DLLEXPORT void int_set(owca_int i);
		OWCA_SCRIPT_DLLEXPORT owca_int int_get() const;
		OWCA_SCRIPT_DLLEXPORT bool int_is() const;
		OWCA_SCRIPT_DLLEXPORT void real_set(owca_real r);
		OWCA_SCRIPT_DLLEXPORT owca_real real_get() const;
		OWCA_SCRIPT_DLLEXPORT bool real_is() const;
		OWCA_SCRIPT_DLLEXPORT void null_set(bool no_value = false);
		OWCA_SCRIPT_DLLEXPORT bool null_is() const;
		OWCA_SCRIPT_DLLEXPORT bool no_value_is() const;
		OWCA_SCRIPT_DLLEXPORT bool function_is() const;
		OWCA_SCRIPT_DLLEXPORT bool object_is() const;
		OWCA_SCRIPT_DLLEXPORT owca_global function_bind(const owca_local &obj) const;
		OWCA_SCRIPT_DLLEXPORT owca_global function_obj() const;
		OWCA_SCRIPT_DLLEXPORT owca_global function_member_of() const;
		OWCA_SCRIPT_DLLEXPORT std::string function_name() const;
		OWCA_SCRIPT_DLLEXPORT std::string function_file_name() const;
		OWCA_SCRIPT_DLLEXPORT unsigned int function_file_line() const;
		OWCA_SCRIPT_DLLEXPORT bool type_is() const;
		OWCA_SCRIPT_DLLEXPORT owca_string type_name() const;

		OWCA_SCRIPT_DLLEXPORT bool is(const owca_local &) const;

		OWCA_SCRIPT_DLLEXPORT std::string str() const;
        OWCA_SCRIPT_DLLEXPORT std::string type_str() const;

		//template <class A> A *member_data() const { return (A*)_member_data(__owca__::structinfo::structid<A*>()); }

		OWCA_SCRIPT_DLLEXPORT void gc_mark(const gc_iteration &g) const;

		OWCA_SCRIPT_DLLEXPORT static owca_local null, null_no_value;

		template <class A> void convert(A &a, const char *pname) {
			static const char *typenames[]={
				"a string","an integer","a real","a boolean","a list","a map","a tuple" };
			if (get(a)) return;
			if (__owca__::__type_name__<A>::INDEX!=0xff) {
				const char *tname=typenames[__owca__::__type_name__<A>::INDEX];
				throw _construct_invalid_param_exception(OWCA_ERROR_FORMAT2("parameter %1 is not %2",pname,tname));
			}
			else {
				throw _construct_invalid_param_exception(OWCA_ERROR_FORMAT1("parameter %1 is of an invalid type", pname));
			}
		}
	};
}

#endif
