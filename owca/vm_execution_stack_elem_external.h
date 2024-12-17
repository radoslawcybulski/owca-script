#ifndef _RC_Y_VM_EXECUTION_STACK_ELEM_EXTERNAL_H
#define _RC_Y_VM_EXECUTION_STACK_ELEM_EXTERNAL_H

#include "vm_execution_stack_elem_base.h"
#include "op_base.h"
#include "exec_callparams.h"
#include "global.h"
#include "parameters.h"
#include "executionreturnvalue.h"

namespace owca {
	class owca_global;
	class owca_local;
	class owca_string;
	class owca_map;
	class owca_tuple;
	class owca_list;
	class owca_vm;

	namespace __owca__ {
		class virtual_machine;
		class exec_variable;
		class exec_map_object;
		class exec_object;
		class gc_iterarion;
		class exec_function_ptr;
	}
}

namespace owca {
	namespace __owca__ {

		template <class A> struct RT {
			typedef A R;
		};
		template <> struct RT<const owca_global &> {
			typedef owca_global R;
		};
		template <> struct RT<const owca_local &> {
			typedef owca_global R;
		};
		template <> struct RT<const owca_string &> {
			typedef owca_string R;
		};
		template <> struct RT<const owca_list &> {
			typedef owca_list R;
		};
		template <> struct RT<const owca_map &> {
			typedef owca_map R;
		};
		template <> struct RT<const owca_tuple &> {
			typedef owca_tuple R;
		};
		struct vm_execution_stack_elem_external_base : public vm_execution_stack_elem_base {
			virtual_machine *vm;
			const void *userptr;
			const exec_variable *param_array;

			vm_execution_stack_elem_external_base() : param_array(NULL) { }
			~vm_execution_stack_elem_external_base() { delete [] param_array; }

			void set_param_array(const exec_variable *pm) { param_array=pm; }

			DLLEXPORT executionstackreturnvalue first_time_execute(executionstackreturnvalue mode);

			template <class A> static A *_create(virtual_machine &vm, const void *ptr)  {
                void *mem_ptr = vm.allocate_memory(sizeof(A),typeid(A));
				A *sf=new (mem_ptr) A();
				vm_execution_stack_elem_external_base *base = static_cast<vm_execution_stack_elem_external_base*>(sf);
				base->vm=&vm;
				vm._allocated(sf);
				if (vm.push_execution_stack_frame(sf)) {
					sf->userptr=ptr;
					return sf;
				}
				sf->gc_release(vm);
				return NULL;
			}
			void create_self() { }
			virtual bool init() { return true; }
		};

		struct vm_execution_stack_elem_external_0 : public vm_execution_stack_elem_external_base {
			static const unsigned int paramcount=0;

			DLLEXPORT void _mark_gc(const gc_iteration &gc) const;
			DLLEXPORT void _release_resources(virtual_machine &vm);

			template <class A> static vm_execution_stack_elem_external_0 *create(virtual_machine &vm, const void *ptr) { return _create<A>(vm,ptr); }
		};

		struct vm_execution_stack_elem_external_1 : public vm_execution_stack_elem_external_base {
			static const unsigned int paramcount=1;
			exec_variable v_params[1];

			DLLEXPORT void _mark_gc(const gc_iteration &gc) const;
			DLLEXPORT void _release_resources(virtual_machine &vm);

			vm_execution_stack_elem_external_1() { v_params[0].reset(); }
			template <class A> static vm_execution_stack_elem_external_1 *create(virtual_machine &vm, const void *ptr) { return _create<A>(vm,ptr); }
		};

		struct vm_execution_stack_elem_external_2 : public vm_execution_stack_elem_external_base {
			static const unsigned int paramcount=2;
			exec_variable v_params[2];

			DLLEXPORT void _mark_gc(const gc_iteration &gc) const;
			DLLEXPORT void _release_resources(virtual_machine &vm);

			vm_execution_stack_elem_external_2() { v_params[0].reset(); v_params[1].reset(); }
			template <class A> static vm_execution_stack_elem_external_2 *create(virtual_machine &vm, const void *ptr) { return _create<A>(vm,ptr); }
		};

		struct vm_execution_stack_elem_external_3 : public vm_execution_stack_elem_external_base {
			static const unsigned int paramcount=3;
			exec_variable v_params[3];

			DLLEXPORT void _mark_gc(const gc_iteration &gc) const;
			DLLEXPORT void _release_resources(virtual_machine &vm);

			vm_execution_stack_elem_external_3() { v_params[0].reset(); v_params[1].reset(); v_params[2].reset(); }
			template <class A> static vm_execution_stack_elem_external_3 *create(virtual_machine &vm, const void *ptr) { return _create<A>(vm,ptr); }
		};

		struct vm_execution_stack_elem_external__ : public vm_execution_stack_elem_external_base {
			callparams cp;
			exec_object *mapobject;

			vm_execution_stack_elem_external__() : mapobject(NULL) { }

			DLLEXPORT void _mark_gc(const gc_iteration &gc) const;
			DLLEXPORT void _release_resources(virtual_machine &vm);

			template <class A> static vm_execution_stack_elem_external__ *create(virtual_machine &vm, const void *ptr) { return _create<A>(vm,ptr); }
		};

		struct vm_execution_stack_elem_external_self_0 : public vm_execution_stack_elem_external_0 {
			exec_variable v_self;

			DLLEXPORT void _mark_gc(const gc_iteration &gc) const;
			DLLEXPORT void _release_resources(virtual_machine &vm);

			vm_execution_stack_elem_external_self_0() { v_self.reset(); }
			template <class A> static vm_execution_stack_elem_external_self_0 *create(virtual_machine &vm, const void *ptr) { return _create<A>(vm,ptr); }
		};

		struct vm_execution_stack_elem_external_self_1 : public vm_execution_stack_elem_external_1 {
			exec_variable v_self;

			DLLEXPORT void _mark_gc(const gc_iteration &gc) const;
			DLLEXPORT void _release_resources(virtual_machine &vm);

			vm_execution_stack_elem_external_self_1() { v_self.reset(); }
			template <class A> static vm_execution_stack_elem_external_self_1 *create(virtual_machine &vm, const void *ptr) { return _create<A>(vm,ptr); }
		};

		struct vm_execution_stack_elem_external_self_2 : public vm_execution_stack_elem_external_2 {
			exec_variable v_self;

			DLLEXPORT void _mark_gc(const gc_iteration &gc) const;
			DLLEXPORT void _release_resources(virtual_machine &vm);

			vm_execution_stack_elem_external_self_2() { v_self.reset(); }
			template <class A> static vm_execution_stack_elem_external_self_2 *create(virtual_machine &vm, const void *ptr) { return _create<A>(vm,ptr); }
		};

		struct vm_execution_stack_elem_external_self_3 : public vm_execution_stack_elem_external_3 {
			exec_variable v_self;

			DLLEXPORT void _mark_gc(const gc_iteration &gc) const;
			DLLEXPORT void _release_resources(virtual_machine &vm);

			vm_execution_stack_elem_external_self_3() { v_self.reset(); }
			template <class A> static vm_execution_stack_elem_external_self_3 *create(virtual_machine &vm, const void *ptr) { return _create<A>(vm,ptr); }
		};

		struct vm_execution_stack_elem_external_self__ : public vm_execution_stack_elem_external__ {
			friend class exec_function_ptr;
			exec_variable v_self;

			DLLEXPORT void _mark_gc(const gc_iteration &gc) const;
			DLLEXPORT void _release_resources(virtual_machine &vm);

			vm_execution_stack_elem_external_self__() { v_self.reset(); }
			template <class A> static vm_execution_stack_elem_external_self__ *create(virtual_machine &vm, const void *ptr) { return (vm_execution_stack_elem_external_self__*)_create<A>(vm,ptr); }
		};

		template <class SELF>
		struct vm_execution_stack_elem_external_self_t : public vm_execution_stack_elem_external_self__ {
			typename RT<SELF>::R self;

			bool init() { return vm->convert(self,v_self); }
		};
		typedef vm_execution_stack_elem_external__ vm_execution_stack_elem_external_t;



		template <class SELF>
		struct vm_execution_stack_elem_external_self_t0 : public vm_execution_stack_elem_external_self_0 {
			typename RT<SELF>::R self;

			bool init() {
				RCASSERT(paramcount==0);
				return vm->convert(self,v_self);
			}
		};
		template <class SELF, class P1>
		struct vm_execution_stack_elem_external_self_t1 : public vm_execution_stack_elem_external_self_1 {
			typename RT<SELF>::R self;
			typename RT<P1>::R p1;

			bool init() {
				RCASSERT(paramcount==1);
				return vm->convert(self,v_self) && vm->convert(p1,v_params[0]);
			}
		};
		template <class SELF, class P1, class P2>
		struct vm_execution_stack_elem_external_self_t2 : public vm_execution_stack_elem_external_self_2 {
			typename RT<SELF>::R self;
			typename RT<P1>::R p1;
			typename RT<P2>::R p2;

			bool init() {
				RCASSERT(paramcount==2);
				return vm->convert(self,v_self) && vm->convert(p1,v_params[0]) && vm->convert(p2,v_params[1]);
			}
		};
		template <class SELF, class P1, class P2, class P3>
		struct vm_execution_stack_elem_external_self_t3 : public vm_execution_stack_elem_external_self_3 {
			typename RT<SELF>::R self;
			typename RT<P1>::R p1;
			typename RT<P2>::R p2;
			typename RT<P3>::R p3;

			bool init() {
				RCASSERT(paramcount==3);
				return vm->convert(self,v_self) && vm->convert(p1,v_params[0]) && vm->convert(p2,v_params[1]) && vm->convert(p3,v_params[2]);
			}
		};


		typedef vm_execution_stack_elem_external_0 vm_execution_stack_elem_external_t0;
		template <class P1>
		struct vm_execution_stack_elem_external_t1 : public vm_execution_stack_elem_external_1 {
			typename RT<P1>::R p1;

			bool init() {
				RCASSERT(paramcount==1);
				return vm->convert(p1,v_params[0]);
			}
		};
		template <class P1, class P2>
		struct vm_execution_stack_elem_external_t2 : public vm_execution_stack_elem_external_2 {
			typename RT<P1>::R p1;
			typename RT<P2>::R p2;

			bool init() {
				RCASSERT(paramcount==2);
				return vm->convert(p1,v_params[0]) && vm->convert(p2,v_params[1]);
			}
		};
		template <class P1, class P2, class P3>
		struct vm_execution_stack_elem_external_t3 : public vm_execution_stack_elem_external_3 {
			typename RT<P1>::R p1;
			typename RT<P2>::R p2;
			typename RT<P3>::R p3;

			bool init() {
				RCASSERT(paramcount==3);
				return vm->convert(p1,v_params[0]) && vm->convert(p2,v_params[1]) && vm->convert(p3,v_params[2]);
			}
		};
	}

	class returnvalue;

	//class owca_function_return_value {
	//public:
	//	enum state_t {
	//		_NONE,
	//		RETURN_VALUE,
	//		CREATE_GENERATOR,
	//		COROUTINE_STOP,
	//		FUNCTION_CALL,
	//	};
	//private:
	//	state_t _state;
	//public:
	//	owca_function_return_value(state_t t=_NONE) : _state(t) { }
	//	explicit owca_function_return_value(__owca__::executionreturnvalue);
	//	bool operator == (state_t t) const { return _state==t; }
	//	bool operator != (state_t t) const { return _state!=t; }
	//	friend bool operator == (state_t t, const owca_function_return_value &c) { return t==c._state; }
	//	friend bool operator != (state_t t, const owca_function_return_value &c) { return t!=c._state; }

	//	state_t type() const { return _state; }
	//};

	class owca_user_function_base_object : private __owca__::vm_execution_stack_elem_external_self__ {
	public:
		friend class __owca__::exec_function_ptr;
		friend struct __owca__::vm_execution_stack_elem_external_base;

		owca_local self;
		owca_parameters parameters;
		const bool generator;

		using __owca__::vm_execution_stack_elem_base::show_in_exception_stack;
		using __owca__::vm_execution_stack_elem_base::catch_exceptions;
	protected:
		owca_user_function_base_object(bool generator) : generator(generator) { }

		virtual owca_global initialize() { return run(); }
		virtual owca_global run()=0;

		enum class ExceptionHandled {
			No,
			Yes
		};
		virtual ExceptionHandled exception_thrown(const owca_global&) { return ExceptionHandled::No; }
		DLLEXPORT owca_vm &vm() const;
		template <class A> bool convert_self(owca_global &ret, A &ptr) {
			return self.convert(ret,vm(),ptr,"self");
		}

		DLLEXPORT void _mark_gc(const gc_iteration &gc) const override;
		DLLEXPORT void _release_resources(__owca__::virtual_machine &vm) override;
	private:
		__owca__::executionstackreturnvalue first_time_execute(__owca__::executionstackreturnvalue mode);
		__owca__::executionstackreturnvalue execute(__owca__::executionstackreturnvalue r);
		__owca__::executionstackreturnvalue parse_owca_exc(owca_exception& oe);
	};

}

#endif
