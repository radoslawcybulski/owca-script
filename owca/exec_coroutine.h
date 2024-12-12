#ifndef _RC_Y_EXEC_COROUTINE_OBJECT_H
#define _RC_Y_EXEC_COROUTINE_OBJECT_H

namespace owca {
	class gc_iteration;
	namespace __owca__ {
		class exec_tuple_object;
		class exec_variable;
		class virtual_machine;
	}
}

namespace owca { namespace __owca__ {
	class exec_coroutine_object : public object_base {
		vm_execution_stack *_stack;
	public:
		exec_coroutine_object(virtual_machine &vm, unsigned int oversize) : _stack(NULL) { }

		vm_execution_stack *coroutine() const {
			return _stack;
		}
		void set_coroutine(vm_execution_stack *s) {
			_stack=s;
		}

		//void _create(virtual_machine &, unsigned int oversize);
		void _marker(const gc_iteration &gc) const;
		void _destroy(virtual_machine &vm);
	};
} }
#endif
