#include "parameters.h"
#include "string.h"

#ifndef _RC_Y_EXEC_FUNCTION_PTR_H
#define _RC_Y_EXEC_FUNCTION_PTR_H

#include "exec_base.h"
#include "exec_variable.h"
#include "exec_variablelocation.h"
#include "executionstackreturnvalue.h"
#include "vm_execution_stack_elem_external.h"
#include "exec_namespace.h"
#include "defval.h"

namespace owca {
	class owca_namespace;

	namespace __owca__ {
		class exec_namespace;
	}
}

namespace owca {
	namespace __owca__ {
		DLLEXPORT owca_parameters _generate_parameters(virtual_machine &vm, const __owca__::callparams &ci_);

	}
}

namespace owca {
	class gc_iteration;
	class owca_user_function_base_object;
	namespace __owca__ {
		struct defval;
		class opcode_data;
		class exec_base;
		class obj_constructor_base;
		class obj_constructor_function;
		class exec_function_bound;
		class exec_function_ptr;
		class exec_stack_variables;
		class exec_variable;
		class exec_variable_location;
		class exec_property;
		class virtual_machine;
		class owca_internal_string;
		class exec_stack;
		class vm_execution_stack_elem_internal_jump;
		struct vm_execution_stack_elem_external_0;
		struct vm_execution_stack_elem_external_1;
		struct vm_execution_stack_elem_external_2;
		struct vm_execution_stack_elem_external_3;
		struct vm_execution_stack_elem_external__;
		struct vm_execution_stack_elem_external_self_0;
		struct vm_execution_stack_elem_external_self_1;
		struct vm_execution_stack_elem_external_self_2;
		struct vm_execution_stack_elem_external_self_3;
		struct vm_execution_stack_elem_external_self__;
	}
}

namespace owca {
	namespace __owca__ {

		typedef vm_execution_stack_elem_external_0 *(*exec_function_create_external_stack_0)(virtual_machine &, const void *ptr);
		typedef vm_execution_stack_elem_external_1 *(*exec_function_create_external_stack_1)(virtual_machine &, const void *ptr);
		typedef vm_execution_stack_elem_external_2 *(*exec_function_create_external_stack_2)(virtual_machine &, const void *ptr);
		typedef vm_execution_stack_elem_external_3 *(*exec_function_create_external_stack_3)(virtual_machine &, const void *ptr);
		typedef vm_execution_stack_elem_external__ *(*exec_function_create_external_stack__)(virtual_machine &, const void *ptr);
		typedef vm_execution_stack_elem_external_self_0 *(*exec_function_create_external_stack_self_0)(virtual_machine &, const void *ptr);
		typedef vm_execution_stack_elem_external_self_1 *(*exec_function_create_external_stack_self_1)(virtual_machine &, const void *ptr);
		typedef vm_execution_stack_elem_external_self_2 *(*exec_function_create_external_stack_self_2)(virtual_machine &, const void *ptr);
		typedef vm_execution_stack_elem_external_self_3 *(*exec_function_create_external_stack_self_3)(virtual_machine &, const void *ptr);
		typedef vm_execution_stack_elem_external_self__ *(*exec_function_create_external_stack_self__)(virtual_machine &, const void *ptr);

	#ifdef RCDEBUG
		struct function_external_param;
		struct function_external;
		struct function_internal;
	#endif

		class exec_function_ptr : public exec_base {
			friend class virtual_machine;
			friend class obj_constructor_base;
			friend class obj_constructor_function;

			~exec_function_ptr() { }
		public:
	#ifdef RCDEBUG
			struct internal_param_info;
			function_external_param *_external_param;
			function_external *_external;
			function_internal *_internal;
	#endif
			union functionptr {
				exec_function_create_external_stack_0 f_0;
				exec_function_create_external_stack_1 f_1;
				exec_function_create_external_stack_2 f_2;
				exec_function_create_external_stack_3 f_3;
				exec_function_create_external_stack__ f__;
				exec_function_create_external_stack_self_0 s_0;
				exec_function_create_external_stack_self_1 s_1;
				exec_function_create_external_stack_self_2 s_2;
				exec_function_create_external_stack_self_3 s_3;
				exec_function_create_external_stack_self__ s__;
			};
			enum functionmode {
				F_INTERNAL,F_FAST,F_FAST_0,F_FAST_1,F_FAST_2,F_FAST_3,F_SELF,F_SELF_0,F_SELF_1,F_SELF_2,F_SELF_3
			};
			enum selftype { NONE,CLASS,SELF };
			enum functiontype { FUNCTION,PROPERTY_READ,PROPERTY_WRITE };
		private:
			functionmode mode_;
			owca_internal_string *name_;
			exec_object *classobject_;
			exec_namespace *nspace_;

			exec_function_ptr() : classobject_(NULL),nspace_(NULL) { }
		public:
			owca_internal_string *name() const { return name_; }
			exec_object *classobject() const { return classobject_; }
			functionmode mode() const { return mode_; }
			exec_namespace *ynamespace() const { return nspace_; }
			DLLEXPORT void set_namespace(exec_namespace *n);
			std::string declared_filename() const;
			unsigned int declared_file_line() const;

			functionptr external_function() const;
			const void *external_data() const;
		private:
			functionptr &external_set_function(const void *ptr_);
			void external_set_function_mode(functionmode fmode);
			void external_set_param_name(unsigned int index, owca_internal_string *);
			exec_variable &external_set_param_default_value(unsigned int index);
		public:
			owca_internal_string *external_param_name(unsigned int index) const;
			const exec_variable &external_param_default_value(unsigned int index) const;
			const void *external_user_pointer() const;

			struct internal_param_info_nodef {
				owca_internal_string *ident;
				exec_variable_location location;
			};
			struct internal_variable_info {
				owca_internal_string *ident;
				exec_variable_location location;
			};
			struct internal_param_info {
				internal_variable_info *var;
				exec_variable defaultvalue;
			};
			selftype internal_self_type() const;
			functiontype internal_function_type() const;
			const exec_variable_location &internal_self_location() const;
			unsigned char internal_is_generator() const;
			const internal_param_info_nodef &internal_map_param() const;
			const internal_param_info_nodef &internal_list_param() const;
			unsigned int internal_variable_count() const;
			const internal_variable_info &internal_variable(unsigned int index) const;
			unsigned int internal_param_count() const;
			const internal_param_info &internal_param(unsigned int index) const;
			unsigned int internal_stack_yield_size() const;
			unsigned int internal_temporary_variables_count() const;
			opcode_data *internal_opcodes() const;
			vm_execution_stack_elem_internal_jump internal_jump_start() const;
			unsigned int internal_stack_variables_count() const;
			unsigned int *internal_stack_variables_level0Map() const;
			exec_stack_variables *internal_stack_variables(unsigned int) const;
			unsigned int internal_local_stack_size() const;
			exec_function_ptr *internal_parent_function() const;

			static exec_function_ptr *internal_allocate(virtual_machine &, owca_internal_string *name, unsigned char is_generator, opcode_data *opcodes,
				const exec_variable_location &selflocation,
				vm_execution_stack_elem_internal_jump *&begin, internal_param_info *&params, internal_variable_info *&variables,
				internal_param_info_nodef *&list_param, internal_param_info_nodef *&map_param,
				unsigned int paramcount, unsigned int varcount, unsigned int localstacksize, selftype stype, functiontype ftype,
				unsigned int maxyielddatasize, unsigned int temporaryvariablescount, exec_stack *st, exec_function_ptr *parentfnc);

			static exec_function_ptr *external_allocate(virtual_machine &, owca_internal_string *name, unsigned int paramcnt);

			void set_class_name(exec_object *cn);
			void gc_acquire() { _gc_acquire(); }
			void gc_release(virtual_machine &vm) { _gc_release(vm); }
			void gc_mark(const gc_iteration &gi) const { _gc_mark(gi); }
		protected:
			void _mark_gc(const gc_iteration &gc) const;
			void _release_resources(virtual_machine &vm);
		private:
			DLLEXPORT static void _function_set_params(exec_function_ptr *f, virtual_machine &vm, const std::string &nn0, const defval &d0);
			DLLEXPORT static void _function_set_params(exec_function_ptr *f, virtual_machine &vm, const std::string &nn0, const std::string &nn1, const defval &d0, const defval &d1);
			DLLEXPORT static void _function_set_params(exec_function_ptr *f, virtual_machine &vm, const std::string &nn0, const std::string &nn1, const std::string &nn2, const defval &d0, const defval &d1, const defval &d2);
			DLLEXPORT static exec_function_ptr *_function_create(virtual_machine &vm, const owca_string &name, unsigned int paramcnt);
			DLLEXPORT static void _function_set_function(exec_function_ptr *f, exec_function_create_external_stack__ ptr, const void *userptr);
			DLLEXPORT static void _function_set_function(exec_function_ptr *f, exec_function_create_external_stack_0 ptr, const void *userptr);
			DLLEXPORT static void _function_set_function(exec_function_ptr *f, exec_function_create_external_stack_1 ptr, const void *userptr);
			DLLEXPORT static void _function_set_function(exec_function_ptr *f, exec_function_create_external_stack_2 ptr, const void *userptr);
			DLLEXPORT static void _function_set_function(exec_function_ptr *f, exec_function_create_external_stack_3 ptr, const void *userptr);
			DLLEXPORT static void _function_set_function(exec_function_ptr *f, exec_function_create_external_stack_self__ ptr, const void *userptr);
			DLLEXPORT static void _function_set_function(exec_function_ptr *f, exec_function_create_external_stack_self_0 ptr, const void *userptr);
			DLLEXPORT static void _function_set_function(exec_function_ptr *f, exec_function_create_external_stack_self_1 ptr, const void *userptr);
			DLLEXPORT static void _function_set_function(exec_function_ptr *f, exec_function_create_external_stack_self_2 ptr, const void *userptr);
			DLLEXPORT static void _function_set_function(exec_function_ptr *f, exec_function_create_external_stack_self_3 ptr, const void *userptr);
			DLLEXPORT static exec_property *_property_write(virtual_machine &vm, exec_function_ptr *r, exec_function_ptr *w);
		public:
			template <class A> static exec_function_ptr *generate_user_function(virtual_machine &vm, owca_string &name) {
				//RCASSERT(dynamic_cast<owca_user_function_base_object*>((A*)NULL));
				exec_function_ptr *f=_function_create(vm,name,0);
				_function_set_function(f,A::template create<A>,NULL);
				return f;
			}
			template <class A> static exec_function_ptr *generate_user_function__(virtual_machine &vm, const owca_string &name, const void *userptr) {
				(void)A::FUNCTION_OBJECT_PARAMS_0;
				exec_function_ptr *f=_function_create(vm,name,0);
				_function_set_function(f,A::template create<A>,userptr);
				return f;
			}
			template <class A> static exec_function_ptr *generate_user_function_0(virtual_machine &vm, const owca_string &name, const void *userptr) {
				(void)A::FUNCTION_OBJECT_PARAMS_0;
				exec_function_ptr *f=_function_create(vm,name,0);
				_function_set_function(f,A::template create<A>,userptr);
				return f;
			}
			template <class A> static exec_function_ptr *generate_user_function_1(virtual_machine &vm, const owca_string &name, const void *userptr, const std::string &n1, const defval &d1=defval()) {
				(void)A::FUNCTION_OBJECT_PARAMS_1;
				exec_function_ptr *f=_function_create(vm,name,1);
				_function_set_function(f,A::template create<A>,userptr);
				_function_set_params(f,vm,n1,d1);
				return f;
			}
			template <class A> static exec_function_ptr *generate_user_function_2(virtual_machine &vm, const owca_string &name, const void *userptr, const std::string &n1, const std::string &n2, const defval &d1=defval(), const defval &d2=defval()) {
				(void)A::FUNCTION_OBJECT_PARAMS_2;
				exec_function_ptr *f=_function_create(vm,name,2);
				_function_set_function(f,A::template create<A>,userptr);
				_function_set_params(f,vm,n1,n2,d1,d2);
				return f;
			}
			template <class A> static exec_function_ptr *generate_user_function_3(virtual_machine &vm, const owca_string &name, const void *userptr, const std::string &n1, const std::string &n2, const std::string &n3, const defval &d1=defval(), const defval &d2=defval(), const defval &d3=defval()) {
				(void)A::FUNCTION_OBJECT_PARAMS_3;
				exec_function_ptr *f=_function_create(vm,name,3);
				_function_set_function(f,A::template create<A>,userptr);
				_function_set_params(f,vm,n1,n2,n3,d1,d2,d3);
				return f;
			}
			template <class A> static exec_function_ptr *generate_function__(virtual_machine &vm, const owca_string &name) {
				(void)A::FUNCTION_OBJECT_PARAMS_0;
				exec_function_ptr *f=_function_create(vm,name,0);
				_function_set_function(f,A::template create<A>,NULL);
				return f;
			}
			template <class A> static exec_function_ptr *generate_function_0(virtual_machine &vm, const owca_string &name) {
				(void)A::FUNCTION_OBJECT_PARAMS_0;
				exec_function_ptr *f=_function_create(vm,name,0);
				_function_set_function(f,A::template create<A>,NULL);
				return f;
			}
			template <class A> static exec_function_ptr *generate_function_1(virtual_machine &vm, const owca_string &name, const std::string &n1, const defval &d1=defval()) {
				(void)A::FUNCTION_OBJECT_PARAMS_1;
				exec_function_ptr *f=_function_create(vm,name,1);
				_function_set_function(f,A::template create<A>,NULL);
				_function_set_params(f,vm,n1,d1);
				return f;
			}
			template <class A> static exec_function_ptr *generate_function_2(virtual_machine &vm, const owca_string &name, const std::string &n1, const std::string &n2, const defval &d1=defval(), const defval &d2=defval()) {
				(void)A::FUNCTION_OBJECT_PARAMS_2;
				exec_function_ptr *f=_function_create(vm,name,2);
				_function_set_function(f,A::template create<A>,NULL);
				_function_set_params(f,vm,n1,n2,d1,d2);
				return f;
			}
			template <class A> static exec_function_ptr *generate_function_3(virtual_machine &vm, const owca_string &name, const std::string &n1, const std::string &n2, const std::string &n3, const defval &d1=defval(), const defval &d2=defval(), const defval &d3=defval()) {
				(void)A::FUNCTION_OBJECT_PARAMS_3;
				exec_function_ptr *f=_function_create(vm,name,3);
				_function_set_function(f,A::template create<A>,NULL);
				_function_set_params(f,vm,n1,n2,n3,d1,d2,d3);
				return f;
			}
			template <class R> static exec_property *generate_property_r(virtual_machine &vm, const owca_string &name, const void *userptr) {
				(void)R::FUNCTION_OBJECT_PARAMS_0;
				owca_internal_string *s=vm.allocate_string(name.str()+"_read");
				exec_function_ptr *r=_function_create(vm,owca_string(&vm,s),0);
				s->gc_release(vm);
				_function_set_function(r,R::template create<R>,userptr);
				return _property_write(vm,r,NULL);
			}
			template <class W> static exec_property *generate_property_w(virtual_machine &vm, const owca_string &name, const void *userptr) {
				(void)W::FUNCTION_OBJECT_PARAMS_1;
				owca_internal_string *s=vm.allocate_string(name.str()+"_write");
				exec_function_ptr *w=_function_create(vm,owca_string(&vm,s),1);
				s->gc_release(vm);
				_function_set_function(w,W::template create<W>,userptr);
				_function_set_params(w,vm,"value",defval());
				return _property_write(vm,NULL,w);
			}
			template <class R, class W> static exec_property *generate_property_rw(virtual_machine &vm, const owca_string &name, const void *userptrread, const void *userptrwrite) {
				(void)R::FUNCTION_OBJECT_PARAMS_0;
				(void)W::FUNCTION_OBJECT_PARAMS_1;
				owca_internal_string *s=vm.allocate_string(name.str()+"_read");
				exec_function_ptr *r=_function_create(vm,owca_string(&vm,s),0);
				s->gc_release(vm);
				_function_set_function(r,R::template create<R>,userptrread);
				s=vm.allocate_string(name.str()+"_write");
				exec_function_ptr *w=_function_create(vm,owca_string(&vm,s),1);
				s->gc_release(vm);
				_function_set_function(w,W::template create<W>,userptrwrite);
				_function_set_params(w,vm,"value",defval());
				return _property_write(vm,r,w);
			}
		};
		class exec_function_bound : public exec_base {
			friend class virtual_machine;
			exec_function_bound() { }
			~exec_function_bound() { }
			exec_function_ptr *fnc;
		public:
			exec_variable slf;
			exec_function_ptr *function() const { return fnc; }
			void gc_acquire() { _gc_acquire(); }
			void gc_release(virtual_machine &vm) { _gc_release(vm); }
			void gc_mark(const gc_iteration &gi) const { _gc_mark(gi); }
		protected:
			void _mark_gc(const gc_iteration &gc) const;
			void _release_resources(virtual_machine &vm);
		};

		class unifunction;
		class unifunction {
			friend class virtual_machine;
			exec_function_ptr *fnc_;
			exec_variable slf_;
		public:
			exec_function_ptr *fnc() const { return fnc_; }
			const exec_variable &slf() const { return slf_; }
		};

		template <class A, class B> class user_function_base_simple : public A {
			bool first_time;
		protected:
			owca_function_return_value frv;
			user_function_base_simple() : first_time(true) { }
		private:
			executionstackreturnvalue execute(executionstackreturnvalue) {
				owca_global ret;
				((B*)this)->call(ret);
				switch(frv.type()) {
				case owca_function_return_value::EXCEPTION:
					this->vm->_raise_from_user(ret._object);
					return executionstackreturnvalue::EXCEPTION;
				case owca_function_return_value::RETURN_VALUE:
					*this->return_value=ret._object;
					ret._object.reset();
					RCASSERT(first_time);
					first_time=false;
					return executionstackreturnvalue::RETURN;
				case owca_function_return_value::NO_RETURN_VALUE:
					RCASSERT(first_time);
					first_time=false;
					this->return_value->set_null(true);
					return executionstackreturnvalue::RETURN;
				case owca_function_return_value::CREATE_GENERATOR:
					//vm->push_execution_stack();
					this->vm->raise_cant_create_generator_from_user_function();
					return executionstackreturnvalue::FUNCTION_CALL;
				case owca_function_return_value::FUNCTION_CALL:
					return executionstackreturnvalue::FUNCTION_CALL;
				case owca_function_return_value::COROUTINE_STOP:
					//vm->push_execution_stack();
					this->vm->raise_cant_stop_coroutine_from_user_function();
					return executionstackreturnvalue::FUNCTION_CALL;
				default:
					RCASSERT(0);
				}
				RCASSERT(0);
				this->return_value->set_null(true);
				return executionstackreturnvalue::RETURN;
			}
		};





		struct user_function_t__spec : public user_function_base_simple<vm_execution_stack_elem_external__,user_function_t__spec > {
			enum { FUNCTION_OBJECT_PARAMS_0 };

			typedef owca_function_return_value (*function)(owca_global &retval, owca_namespace &ns, const owca_parameters &);
			DLLEXPORT void call(owca_global &ret);
		};
		struct user_function_t0_spec : public user_function_base_simple<vm_execution_stack_elem_external_0,user_function_t0_spec > {
			enum { FUNCTION_OBJECT_PARAMS_0 };

			typedef owca_function_return_value (*function)(owca_global &retval, owca_namespace &ns);
			DLLEXPORT void call(owca_global &ret);
		};
		template <class A> struct user_function_t1_spec : public user_function_base_simple<vm_execution_stack_elem_external_t1<A>,user_function_t1_spec<A> > {
			enum { FUNCTION_OBJECT_PARAMS_1 };
			typedef owca_function_return_value (*function)(owca_global &retval, owca_namespace &ns, A);

			inline void call(owca_global &ret) {
				this->frv=((function)this->userptr)(ret,this->fnc->ynamespace()->generate(),this->p1);
			}
		};
		template <class A, class B> struct user_function_t2_spec : public user_function_base_simple<vm_execution_stack_elem_external_t2<A,B>,user_function_t2_spec<A,B> > {
			enum { FUNCTION_OBJECT_PARAMS_2 };
			typedef owca_function_return_value (*function)(owca_global &retval, owca_namespace &ns,A,B);

			inline void call(owca_global &ret) {
				this->frv=((function)this->userptr)(ret,this->fnc->ynamespace()->generate(),this->p1,this->p2);
			}
		};
		template <class A, class B, class C> struct user_function_t3_spec : public user_function_base_simple<vm_execution_stack_elem_external_t3<A,B,C>,user_function_t3_spec<A,B,C> > {
			enum { FUNCTION_OBJECT_PARAMS_3 };
			typedef owca_function_return_value (*function)(owca_global &retval, owca_namespace &ns,A,B,C);

			inline void call(owca_global &ret) {
				this->frv=((function)this->userptr)(ret,this->fnc->ynamespace()->generate(),this->p1,this->p2,this->p3);
			}
		};
		template <class SELF> struct user_function_s__spec : public user_function_base_simple<vm_execution_stack_elem_external_self_t<SELF>,user_function_s__spec<SELF> > {
			enum { FUNCTION_OBJECT_PARAMS_0 };
			typedef owca_function_return_value (*function)(owca_global &retval, SELF,owca_namespace &ns, const owca_parameters &);

			inline void call(owca_global &ret) {
				this->frv=((function)this->userptr)(ret,this->self,this->fnc->ynamespace()->generate(),_generate_parameters(*this->vm,this->cp));
			}
		};
		template <class SELF> struct user_function_s0_spec : public user_function_base_simple<vm_execution_stack_elem_external_self_t0<SELF>,user_function_s0_spec<SELF> > {
			enum { FUNCTION_OBJECT_PARAMS_0 };
			typedef owca_function_return_value (*function)(owca_global &retval, SELF,owca_namespace &ns);

			inline void call(owca_global &ret) {
				this->frv=((function)this->userptr)(ret,this->self,this->fnc->ynamespace()->generate());
			}
		};
		template <class SELF, class A> struct user_function_s1_spec : public user_function_base_simple<vm_execution_stack_elem_external_self_t1<SELF,A>,user_function_s1_spec<SELF,A> > {
			enum { FUNCTION_OBJECT_PARAMS_1 };
			typedef owca_function_return_value (*function)(owca_global &retval, SELF,owca_namespace &ns, A);

			inline void call(owca_global &ret) {
				this->frv=((function)this->userptr)(ret,this->self,this->fnc->ynamespace()->generate(),this->p1);
			}
		};
		template <class SELF, class A, class B> struct user_function_s2_spec : public user_function_base_simple<vm_execution_stack_elem_external_self_t2<SELF,A,B>,user_function_s2_spec<SELF,A,B> > {
			enum { FUNCTION_OBJECT_PARAMS_2 };
			typedef owca_function_return_value (*function)(owca_global &retval, SELF,owca_namespace &ns,A,B);

			inline void call(owca_global &ret) {
				this->frv=((function)this->userptr)(ret,this->self,this->fnc->ynamespace()->generate(),this->p1,this->p2);
			}
		};
		template <class SELF, class A, class B, class C> struct user_function_s3_spec : public user_function_base_simple<vm_execution_stack_elem_external_self_t3<SELF,A,B,C>,user_function_s3_spec<SELF,A,B,C> > {
			enum { FUNCTION_OBJECT_PARAMS_3 };
			typedef owca_function_return_value (*function)(owca_global &retval, SELF,owca_namespace &ns,A,B,C);

			inline void call(owca_global &ret) {
				this->frv=((function)this->userptr)(ret,this->self,this->fnc->ynamespace()->generate(),this->p1,this->p2,this->p3);
			}
		};

	}
}
#endif
