#ifndef _RC_Y_EXEC_VARIABLE_H
#define _RC_Y_EXEC_VARIABLE_H

#include "debug_assert.h"
#include "operatorcodes.h"
#include "exectype.h"

namespace owca {
	class gc_iteration;
	namespace __owca__ {
		struct callparams;
		class exec_function_bound;
		class exec_function_ptr;
		struct vm_execution_stack_elem_base;
		class exec_map_object;
		class exec_object;
		class exec_property;
		class exec_variable;
		class virtual_machine;
		class owca_internal_string;
		class owca_internal_string_nongc;
		class vm_execution_stack;
		class exec_namespace;
		class exec_weakref_object;
	}
}

namespace owca { namespace __owca__ {

	class lookupreturnvalue {
	public:
		enum type_ {
			LOOKUP_FOUND,
			LOOKUP_NOT_FOUND,
			LOOKUP_FUNCTION_CALL,
		};
	private:
		type_ tp;
		lookupreturnvalue() : tp(LOOKUP_NOT_FOUND) {}
	public:
		lookupreturnvalue(type_ tp_) : tp(tp_) { }

		type_ type() const { return tp; }
	};


	class exec_variable {
	public:
		struct fncfasttype {
			exec_object *slf;
			exec_function_ptr *fnc;
		};
	private:
		union {
			bool no_return_value;
			owca_int i;
			owca_real r;
			bool b;
			exec_object *o;
			exec_weakref_object *weakref;
			owca_internal_string *s;
			owca_internal_string_nongc *ss;
			vm_execution_stack_elem_base *generator;
			exec_function_bound *fnc;
			fncfasttype fncfast;
			exec_property *prop;
			exec_namespace *nspace;
			vm_execution_stack *co;
		} data;
		exectype _mode;
	public:
		exectype mode() const { RCASSERT(_mode<VAR_UNDEFINED); return _mode; }
		void setmode(exectype m) {
			_mode=m;
		}
		void reset() { set_null(); }

		DLLEXPORT void gc_acquire() const;
		DLLEXPORT void gc_release(virtual_machine &);
		DLLEXPORT void gc_mark(const gc_iteration &gc) const;

		bool type(exectype tp) const;
		bool type(exec_object *tp) const;
		//bool type_raise(virtual_machine &, exectype tp) const;
		//bool type_raise(virtual_machine &, exec_object *tp) const;

		owca_int get_int() const { RCASSERT(_mode==VAR_INT); return data.i; }
		owca_real get_real() const { RCASSERT(_mode==VAR_REAL); return data.r; }
		bool get_bool() const { RCASSERT(_mode==VAR_BOOL); return data.b; }
		bool get_bool_() const { return data.b; }
		exec_object *get_object() const { RCASSERT(_mode==VAR_OBJECT); return data.o; }
		owca_internal_string *get_string() const { RCASSERT(_mode==VAR_STRING); return data.s; }
		vm_execution_stack_elem_base *get_generator() const { RCASSERT(_mode==VAR_GENERATOR); return data.generator; }
		exec_function_bound *get_function() const { RCASSERT(_mode==VAR_FUNCTION); return data.fnc; }
		fncfasttype get_function_fast() const { RCASSERT(_mode==VAR_FUNCTION_FAST); return data.fncfast; }
		exec_property *get_property() const { RCASSERT(_mode==VAR_PROPERTY); return data.prop; }
		exec_namespace *get_namespace() const { RCASSERT(_mode==VAR_NAMESPACE); return data.nspace; }
		exec_weakref_object *get_weak_ref() const { RCASSERT(_mode==VAR_WEAK_REF); return data.weakref; }

		void set_object(exec_object *o) { RCASSERT(o); _mode=VAR_OBJECT; data.o=o; }
		void set_object_null(exec_object *o) { if (o) { _mode=VAR_OBJECT; data.o=o; } else _mode=VAR_NULL; }
		void set_string(owca_internal_string *s) { _mode=VAR_STRING; data.s=s; }
		void set_function_s(virtual_machine &vm, const exec_variable &slf, exec_function_ptr *fnc);
		void set_function(exec_function_bound *p) { RCASSERT(p); _mode=VAR_FUNCTION; data.fnc=p; }
		void set_function_fast(exec_function_ptr *fnc, exec_object *slf=NULL) { RCASSERT(fnc); _mode=VAR_FUNCTION_FAST; data.fncfast.slf=slf; data.fncfast.fnc=fnc; }
		void set_property(exec_property *p) { RCASSERT(p); _mode=VAR_PROPERTY; data.prop=p; }
		void set_namespace(exec_namespace *n) { RCASSERT(n); _mode=VAR_NAMESPACE; data.nspace=n; }
		void set_null(bool no_return_value = false) { _mode = VAR_NULL; data.no_return_value = no_return_value; }
		void set_bool(bool z) { _mode=VAR_BOOL; data.b=z; }
		void set_int(owca_int i) { _mode=VAR_INT; data.i=i; }
		void set_real(owca_real r) { _mode=VAR_REAL; data.r=r; }
		void set_generator(vm_execution_stack_elem_base *st) { RCASSERT(st); _mode=VAR_GENERATOR; data.generator=st; }
		void set_weak_ref(exec_weakref_object *w) { RCASSERT(w); _mode=VAR_WEAK_REF; data.weakref=w; }

		bool is_no_return_value() const { return _mode == VAR_NULL && data.no_return_value; }
		bool get_int_min(owca_int &ret) const;
		bool get_int_max(owca_int &ret) const;
		bool get_int(owca_int &ret) const;
		bool is_same(const exec_variable &other) const;

		bool lookup_operator(exec_variable &retval, virtual_machine &vm, operatorcodes oper) const;
		exec_function_ptr *get_operator_function(virtual_machine &vm, operatorcodes oper) const;
		bool has_operator(virtual_machine &vm, operatorcodes oper) const;
		lookupreturnvalue lookup_read(exec_variable &ret, virtual_machine &vm, owca_internal_string *ident, bool alloc_stack_on_function_call=false) const;
		lookupreturnvalue lookup_write(exec_variable &ret, virtual_machine &vm, owca_internal_string *ident, const exec_variable &val, bool alloc_stack_on_function_call=false) const;
	};
} }

#endif
