#include "returnvalue.h"
#include "defval.h"
#include "global.h"
#include "virtualmachine.h"
#include "exception.h"
#include "parameters.h"
#include "vm.h"
#include "string.h"
#include <cassert>

#ifndef _RC_Y_OBJ_CONSTRUCTOR_BASE_H
#define _RC_Y_OBJ_CONSTRUCTOR_BASE_H

#include "exec_function_ptr.h"
#include "exec_property.h"
#include "exec_variable.h"

namespace owca {
	class owca_global;
	class owca_local;
	class owca_parameters;
	class owca_vm;
	class owca_class;
	class owca_map;
	class owca_list;
	class owca_tuple;

	namespace __owca__ {
		struct callparams;
		struct defval;
		class exec_object;
		class exec_variable;
		class internal_class;
		class virtual_machine;
		class obj_constructor_base;
	}
}

namespace owca {
	namespace __owca__ {
		class obj_constructor_base_used {
		protected:
			mutable bool used;

			obj_constructor_base_used() : used(false) { }
			obj_constructor_base_used(const obj_constructor_base_used &) = delete;
			obj_constructor_base_used(obj_constructor_base_used &&d) : used(d.used) {
				d.used = true;
			}
		};
		class obj_constructor_base : protected obj_constructor_base_used {
		private:
			friend class local_obj_constructor;
			friend class owca_class;
			friend class virtual_machine;
			static owca_vm &__get(virtual_machine &);
		protected:
			virtual_machine &_vm;
			mutable owca_global g;
			owca_internal_string *name;
			virtual void _read(__owca__::exec_variable &val) const=0;
			virtual void _write(const __owca__::exec_variable &)=0;
			DLLEXPORT owca_global read() const;
			void write(const exec_variable &val) { used=true; _write(val); }
			DLLEXPORT static exec_variable &_get_var(owca_global &g);
			DLLEXPORT void _update_vm(owca_global &g);
		static bool _is_oper(const char *_name) { return _name[0]=='$' && _name!=std::string("$init") && _name!=std::string("$call"); }

		private:
			DLLEXPORT obj_constructor_base() = delete;
		protected:
			obj_constructor_base(virtual_machine &vm_, owca_internal_string *ident_) : _vm(vm_),name(ident_) { }
			DLLEXPORT obj_constructor_base(obj_constructor_base &&) = default;

			DLLEXPORT void set(exec_object *o);
			DLLEXPORT void set(exec_function_ptr *o);
			DLLEXPORT void set(exec_property *o);
		public:
			DLLEXPORT virtual ~obj_constructor_base();
			DLLEXPORT operator const owca_global &();

#define Z(a) a operator = (a i) { int_set((owca_int)i); return i; }
			Z(unsigned long int);
			Z(unsigned long long int);
			Z(long int);
			Z(long long int);
			Z(int);
#undef Z
#define Z(a) a operator = (a i) { real_set((owca_real)i); return i; }
			Z(float);
			Z(double);
			Z(long double);
#undef Z
			DLLEXPORT bool operator = (bool b);
			DLLEXPORT const char *operator = (const char *);
			DLLEXPORT const std::string &operator = (const std::string &);

			DLLEXPORT const owca_local &operator = (const owca_local &l);

			DLLEXPORT local_obj_constructor operator [] (const std::string& name);

			DLLEXPORT owca_global type() const;

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
			DLLEXPORT const char *string_get(unsigned int &sz) const;
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
			DLLEXPORT owca_global function_bind(const owca_local &obj) const;
			DLLEXPORT owca_global function_obj() const;
			DLLEXPORT bool is(const owca_local &) const;
			DLLEXPORT bool type_is() const;
			DLLEXPORT owca_string type_name() const;

			template <class A> bool get(A &a) const {
				return read().get(a);
			}
			DLLEXPORT std::string str() const;

			DLLEXPORT owca_global get_member(const owca_string &member) const;
			DLLEXPORT owca_global set_member(const owca_string &member, const owca_global &val) const;
			DLLEXPORT owca_global call() const;
			DLLEXPORT owca_global call(const owca_global &p1) const;
			DLLEXPORT owca_global call(const owca_global &p1, const owca_global &p2) const;
			DLLEXPORT owca_global call(const owca_global &p1, const owca_global &p2, const owca_global &p3) const;
			DLLEXPORT owca_global call(const owca_global &p1, const owca_global &p2, const owca_global &p3, const owca_global &p4) const;
			DLLEXPORT owca_global call(const owca_call_parameters &cp) const;
			DLLEXPORT owca_global call(const owca_parameters &cp) const;

			template <class A> A *member_data() const { A *p = nullptr; read().get(p); return p; }
		};


		class obj_constructor_function : public obj_constructor_base {
			friend class owca_class;
			friend class virtual_machine;
			friend class owca_namespace;

		protected:
			exec_namespace *nspace;

			DLLEXPORT obj_constructor_function(exec_namespace *ns, owca_internal_string *ident);
			using obj_constructor_base::set;
		public:
			DLLEXPORT obj_constructor_function(obj_constructor_function &&) = default;

			~obj_constructor_function() {
				if (!used) read();
			}
			using obj_constructor_base::operator =;

			template <class A> void set(void) {
				exec_variable v;
				v.set_function_fast(exec_function_ptr::generate_user_function<A>(_vm,owca_string(&_vm,name)));
				v.get_function_fast().fnc->set_namespace(nspace);
				write(v);
				v.get_function_fast().fnc->gc_release(_vm);
			}
			void set(owca_global(*fnc)(owca_namespace &ns, const owca_parameters &)) {
				exec_variable v;
				assert(sizeof(fnc) <= sizeof(void*));
				v.set_function_fast(exec_function_ptr::generate_user_function__<user_function_t__spec>(_vm,owca_string(&_vm,name),(void*)(fnc)));
				v.get_function_fast().fnc->set_namespace(nspace);
				write(v);
				v.get_function_fast().fnc->gc_release(_vm);
			}
			void set(owca_global (*fnc)(owca_namespace &ns)) {
				exec_variable v;
				assert(sizeof(fnc) <= sizeof(void*));
				v.set_function_fast(exec_function_ptr::generate_user_function_0<user_function_t0_spec>(_vm,owca_string(&_vm,name),(void*)(fnc)));
				v.get_function_fast().fnc->set_namespace(nspace);
				write(v);
				v.get_function_fast().fnc->gc_release(_vm);
			}
			template <class A> void set(owca_global (*fnc)(owca_namespace &ns,A), const std::string &n1, const defval &d1=defval()) {
				exec_variable v;
				assert(sizeof(fnc) <= sizeof(void*));
				v.set_function_fast(exec_function_ptr::generate_user_function_1<user_function_t1_spec<A> >(_vm,owca_string(&_vm,name),(void*)(fnc),n1,d1));
				v.get_function_fast().fnc->set_namespace(nspace);
				write(v);
				v.get_function_fast().fnc->gc_release(_vm);
			}
			template <class A, class B> void set(owca_global (*fnc)(owca_namespace &ns,A,B), const std::string &n1, const std::string &n2, const defval &d1=defval(), const defval &d2=defval()) {
				exec_variable v;
				assert(sizeof(fnc) <= sizeof(void*));
				v.set_function_fast(exec_function_ptr::generate_user_function_2<user_function_t2_spec<A,B> >(_vm,owca_string(&_vm,name),(void*)(fnc),n1,n2,d1,d2));
				v.get_function_fast().fnc->set_namespace(nspace);
				write(v);
				v.get_function_fast().fnc->gc_release(_vm);
			}
			template <class A, class B, class C> void set(owca_global (*fnc)(owca_namespace &ns,A,B,C), const std::string &n1, const std::string &n2, const std::string &n3, const defval &d1=defval(), const defval &d2=defval(), const defval &d3=defval()) {
				exec_variable v;
				assert(sizeof(fnc) <= sizeof(void*));
				v.set_function_fast(exec_function_ptr::generate_user_function_3<user_function_t3_spec<A,B,C> >(_vm,owca_string(&_vm,name),(void*)(fnc),n1,n2,n3,d1,d2,d3));
				v.get_function_fast().fnc->set_namespace(nspace);
				write(v);
				v.get_function_fast().fnc->gc_release(_vm);
			}

			template <class SELF> void set(owca_global (*fnc)(SELF,owca_namespace &ns, const owca_parameters &)) {
				exec_variable v;
				assert(sizeof(fnc) <= sizeof(void*));
				v.set_function_fast(exec_function_ptr::generate_user_function__<user_function_s__spec<SELF> >(_vm,owca_string(&_vm,name),(void*)(fnc)));
				v.get_function_fast().fnc->set_namespace(nspace);
				write(v);
				v.get_function_fast().fnc->gc_release(_vm);
			}
			template <class SELF> void set(owca_global (*fnc)(SELF,owca_namespace &ns)) {
				exec_variable v;
				assert(sizeof(fnc) <= sizeof(void*));
				v.set_function_fast(exec_function_ptr::generate_user_function_0<user_function_s0_spec<SELF> >(_vm,owca_string(&_vm,name),(void*)(fnc)));
				v.get_function_fast().fnc->set_namespace(nspace);
				write(v);
				v.get_function_fast().fnc->gc_release(_vm);
			}
			template <class SELF, class A> void set(owca_global (*fnc)(SELF,owca_namespace &ns,A), const std::string &n1, const defval &d1=defval()) {
				exec_variable v;
				assert(sizeof(fnc) <= sizeof(void*));
				v.set_function_fast(exec_function_ptr::generate_user_function_1<user_function_s1_spec<SELF,A> >(_vm,owca_string(&_vm,name),(void*)(fnc),n1,d1));
				v.get_function_fast().fnc->set_namespace(nspace);
				write(v);
				v.get_function_fast().fnc->gc_release(_vm);
			}
			template <class SELF, class A, class B> void set(owca_global (*fnc)(SELF,owca_namespace &ns,A,B), const std::string &n1, const std::string &n2, const defval &d1=defval(), const defval &d2=defval()) {
				exec_variable v;
				assert(sizeof(fnc) <= sizeof(void*));
				v.set_function_fast(exec_function_ptr::generate_user_function_2<user_function_s2_spec<SELF,A,B> >(_vm,owca_string(&_vm,name),(void*)(fnc),n1,n2,d1,d2));
				v.get_function_fast().fnc->set_namespace(nspace);
				write(v);
				v.get_function_fast().fnc->gc_release(_vm);
			}
			template <class SELF, class A, class B, class C> void set(owca_global (*fnc)(SELF,owca_namespace &ns,A,B,C), const std::string &n1, const std::string &n2, const std::string &n3, const defval &d1=defval(), const defval &d2=defval(), const defval &d3=defval()) {
				exec_variable v;
				assert(sizeof(fnc) <= sizeof(void*));
				v.set_function_fast(exec_function_ptr::generate_user_function_3<user_function_s3_spec<SELF,A,B,C> >(_vm,owca_string(&_vm,name),(void*)(fnc),n1,n2,n3,d1,d2,d3));
				v.get_function_fast().fnc->set_namespace(nspace);
				write(v);
				v.get_function_fast().fnc->gc_release(_vm);
			}

			template <class SELF> void set_property(owca_global (*fnc)(SELF,owca_namespace &ns)) {
				exec_variable v;
				assert(sizeof(fnc) <= sizeof(void*));
				v.set_property(exec_function_ptr::generate_property_r<user_function_s0_spec<SELF> >(_vm,owca_string(&_vm,name),(void*)(fnc)));
				v.get_property()->read->set_namespace(nspace);
				write(v);
				v.get_property()->gc_release(_vm);
			}
			template <class SELF, class A> void set_property(owca_global (*fnc)(SELF,owca_namespace &ns,A)) {
				exec_variable v;
				assert(sizeof(fnc) <= sizeof(void*));
				v.set_property(exec_function_ptr::generate_property_w<user_function_s1_spec<SELF,A> >(_vm,owca_string(&_vm,name),(void*)(fnc)));
				v.get_property()->write->set_namespace(nspace);
				write(v);
				v.get_property()->gc_release(_vm);
			}
			template <class SELF, class A> void set_property(owca_global (*read)(SELF,owca_namespace &ns), owca_global (*write)(SELF,owca_namespace &ns,A)) {
				exec_variable v;
				assert(sizeof(read) <= sizeof(void*));
				assert(sizeof(write) <= sizeof(void*));
				v.set_property(exec_function_ptr::generate_property_rw<user_function_s0_spec<SELF>,user_function_s1_spec<SELF,A> >(_vm,owca_string(&_vm,name),(void*)(read),(void*)(write)));
				v.get_property()->read->set_namespace(nspace);
				v.get_property()->write->set_namespace(nspace);
				write(v);
				v.get_property()->gc_release(_vm);
			}
		};

		class local_obj_constructor : public __owca__::obj_constructor_base {
			obj_constructor_base& self;
			const owca_string& member;
			DLLEXPORT void _write(const __owca__::exec_variable &);
			DLLEXPORT void _read(__owca__::exec_variable &val) const;
		public:
			local_obj_constructor(obj_constructor_base& self, const owca_string& member) : obj_constructor_base(self._vm, member._ss), self(self), member(member) {}
			local_obj_constructor(local_obj_constructor &&) = default;
			using __owca__::obj_constructor_base::operator =;
		};
	}
}

#define __RC_TXT__(a) #a
#define M_OPER_(nspace,ident,name,params) nspace ["$" __RC_TXT__(name)].set(exec_function_ptr::generate_function__ <ident ## _ ## name ## _object> params);
#define M_OPER0(nspace,ident,name,params) nspace ["$" __RC_TXT__(name)].set(exec_function_ptr::generate_function_0 <ident ## _ ## name ## _object> params);
#define M_OPER1(nspace,ident,name,params) nspace ["$" __RC_TXT__(name)].set(exec_function_ptr::generate_function_1 <ident ## _ ## name ## _object> params);
#define M_OPER2(nspace,ident,name,params) nspace ["$" __RC_TXT__(name)].set(exec_function_ptr::generate_function_2 <ident ## _ ## name ## _object> params);
#define M_OPER3(nspace,ident,name,params) nspace ["$" __RC_TXT__(name)].set(exec_function_ptr::generate_function_3 <ident ## _ ## name ## _object> params);

#define M_OPER_A(nspace,name1,ident,name2,params) c["$" __RC_TXT__(name1)].set(exec_function_ptr::generate_function_1 <ident ## _ ## name2 ## _object> params);

#define M_FUNC_(nspace,ident,name,params) nspace [__RC_TXT__(name)].set(exec_function_ptr::generate_function__ <ident ## _ ## name ## _object> params);
#define M_FUNC0(nspace,ident,name,params) nspace [__RC_TXT__(name)].set(exec_function_ptr::generate_function_0 <ident ## _ ## name ## _object> params);
#define M_FUNC1(nspace,ident,name,params) nspace [__RC_TXT__(name)].set(exec_function_ptr::generate_function_1 <ident ## _ ## name ## _object> params);
#define M_FUNC2(nspace,ident,name,params) nspace [__RC_TXT__(name)].set(exec_function_ptr::generate_function_2 <ident ## _ ## name ## _object> params);
#define M_FUNC3(nspace,ident,name,params) nspace [__RC_TXT__(name)].set(exec_function_ptr::generate_function_3 <ident ## _ ## name ## _object> params);

#define M_PROP_R (nspace,ident,name,vm) nspace [__RC_TXT__(name)].set(exec_function_ptr::generate_property_r  <ident ## _ ## name ## _read_object> (vm,__RC_TXT__(name),NULL));
#define M_PROP_W (nspace,ident,name,vm) nspace [__RC_TXT__(name)].set(exec_function_ptr::generate_property_w  <ident ## _ ## name ## _write_object> (vm,__RC_TXT__(name),NULL));
#define M_PROP_RW(nspace,ident,name,vm) nspace [__RC_TXT__(name)].set(exec_function_ptr::generate_property_rw <ident ## _ ## name ## _read_object,ident ## _ ## name ## _write_object> (vm,__RC_TXT__(name),NULL,NULL));

#define D_FUNC_(ident,name) \
	class ident ## _ ## name ## _object : public vm_execution_stack_elem_external__ { \
	public: \
		enum { FUNCTION_OBJECT_PARAMS_0,FUNCTION_OBJECT_MODE_f_ }; \
		typedef vm_execution_stack_elem_external__ BASE; \
		typedef ident ## _ ## name ## _object OBJECTTYPE; \
		ident ## _ ## name ## _object() { create_self(); } \
		executionstackreturnvalue execute(executionstackreturnvalue r)
#define D_FUNC0(ident,name) \
	class ident ## _ ## name ## _object : public vm_execution_stack_elem_external_0 { \
	public: \
		enum { FUNCTION_OBJECT_PARAMS_0,FUNCTION_OBJECT_MODE_f0 }; \
		typedef vm_execution_stack_elem_external_0 BASE; \
		typedef ident ## _ ## name ## _object OBJECTTYPE; \
		ident ## _ ## name ## _object() { create_self(); } \
		executionstackreturnvalue execute(executionstackreturnvalue r)
#define D_FUNC1(ident,name,type1) \
	class ident ## _ ## name ## _object : public vm_execution_stack_elem_external_t1<type1> { \
	public: \
		enum { FUNCTION_OBJECT_PARAMS_1,FUNCTION_OBJECT_MODE_f1 }; \
		typedef vm_execution_stack_elem_external_t1<type1> BASE; \
		typedef ident ## _ ## name ## _object OBJECTTYPE; \
		ident ## _ ## name ## _object() { create_self(); } \
		executionstackreturnvalue execute(executionstackreturnvalue r)
#define D_FUNC2(ident,name,type1,type2) \
	class ident ## _ ## name ## _object : public vm_execution_stack_elem_external_t2<type1,type2> { \
	public: \
		enum { FUNCTION_OBJECT_PARAMS_2,FUNCTION_OBJECT_MODE_f2 }; \
		typedef vm_execution_stack_elem_external_t2<type1,type2> BASE; \
		typedef ident ## _ ## name ## _object OBJECTTYPE; \
		ident ## _ ## name ## _object() { create_self(); } \
		executionstackreturnvalue execute(executionstackreturnvalue r)
#define D_FUNC3(ident,name,type1,type2,type3) \
	class ident ## _ ## name ## _object : public vm_execution_stack_elem_external_t3<type1,type2,type3> { \
	public: \
		enum { FUNCTION_OBJECT_PARAMS_3,FUNCTION_OBJECT_MODE_f3 }; \
		typedef vm_execution_stack_elem_external_t3<type1,type2,type3> BASE; \
		typedef ident ## _ ## name ## _object OBJECTTYPE; \
		ident ## _ ## name ## _object() { create_self(); } \
		executionstackreturnvalue execute(executionstackreturnvalue r)

#define D_SELF_(ident,name,typeself) \
	class ident ## _ ## name ## _object : public vm_execution_stack_elem_external_self_t<typeself> { \
	public: \
		enum { FUNCTION_OBJECT_PARAMS_0,FUNCTION_OBJECT_MODE_s_ }; \
		typedef vm_execution_stack_elem_external_self_t<typeself> BASE; \
		typedef ident ## _ ## name ## _object OBJECTTYPE; \
		ident ## _ ## name ## _object() { create_self(); } \
		executionstackreturnvalue execute(executionstackreturnvalue r)
#define D_SELF0(ident,name,typeself) \
	class ident ## _ ## name ## _object : public vm_execution_stack_elem_external_self_t0<typeself> { \
	public: \
		enum { FUNCTION_OBJECT_PARAMS_0,FUNCTION_OBJECT_MODE_s0 }; \
		typedef vm_execution_stack_elem_external_self_t0<typeself> BASE; \
		typedef ident ## _ ## name ## _object OBJECTTYPE; \
		ident ## _ ## name ## _object() { create_self(); } \
		executionstackreturnvalue execute(executionstackreturnvalue r)
#define D_SELF1(ident,name,typeself,type1) \
	class ident ## _ ## name ## _object : public vm_execution_stack_elem_external_self_t1<typeself,type1> { \
	public: \
		enum { FUNCTION_OBJECT_PARAMS_1,FUNCTION_OBJECT_MODE_s1 }; \
		typedef vm_execution_stack_elem_external_self_t1<typeself,type1> BASE; \
		typedef ident ## _ ## name ## _object OBJECTTYPE; \
		ident ## _ ## name ## _object() { create_self(); } \
		executionstackreturnvalue execute(executionstackreturnvalue r)
#define D_SELF2(ident,name,typeself,type1,type2) \
	class ident ## _ ## name ## _object : public vm_execution_stack_elem_external_self_t2<typeself,type1,type2> { \
	public: \
		enum { FUNCTION_OBJECT_PARAMS_2,FUNCTION_OBJECT_MODE_s2 }; \
		typedef vm_execution_stack_elem_external_self_t2<typeself,type1,type2> BASE; \
		typedef ident ## _ ## name ## _object OBJECTTYPE; \
		ident ## _ ## name ## _object() { create_self(); } \
		executionstackreturnvalue execute(executionstackreturnvalue r)
#define D_SELF3(ident,name,typeself,type1,type2,type3) \
	class ident ## _ ## name ## _object : public vm_execution_stack_elem_external_self_t3<typeself,type1,type2,type3> { \
	public: \
		enum { FUNCTION_OBJECT_PARAMS_3,FUNCTION_OBJECT_MODE_s3 }; \
		typedef vm_execution_stack_elem_external_self_t3<typeself,type1,type2,type3> BASE; \
		typedef ident ## _ ## name ## _object OBJECTTYPE; \
		ident ## _ ## name ## _object() { create_self(); } \
		executionstackreturnvalue execute(executionstackreturnvalue r)

#define D_END \
	};

#define D_GCMARK \
	void _mark_gc(const gc_iteration &gc) const \
	{ \
		BASE::_mark_gc(gc);
#define D_RELRES \
	void _release_resources(virtual_machine &vm) \
	{ \
		BASE::_release_resources(vm);

#endif
