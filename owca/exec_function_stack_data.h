#ifndef _RC_Y_EXEC_FUNCTION_STACK_DATA_H
#define _RC_Y_EXEC_FUNCTION_STACK_DATA_H

#include "op_base.h"
#include "exec_base.h"
#include "op_flow_data_object.h"

namespace owca {
	class gc_iteration;
	namespace __owca__ {
		class exec_base;
		class exec_function_stack_data;
		class exec_generator_function;
		class virtual_machine;
		class returnvalueflow;
		struct vm_execution_stack_elem_internal;
	}
}

namespace owca {
	namespace __owca__ {

		class exec_function_stack_data : public exec_base {
			friend class virtual_machine;

			~exec_function_stack_data() { }
			unsigned int actpos,totalsize;

			exec_function_stack_data(unsigned int sz) : totalsize(sz),actpos(sz) { }

			op_flow_data_object *ptr(unsigned int off) { return (op_flow_data_object*)(((char*)this)+sizeof(*this)+off); }
			const op_flow_data_object *ptr(unsigned int off) const { return (const op_flow_data_object*)(((const char*)this)+sizeof(*this)+off); }
		public:
			void gc_acquire() { 
				RCPRINTF("acquire %p\n", this);
				_gc_acquire();
			}
			void gc_release(virtual_machine &vm) { 
				RCPRINTF("release %p\n", this);
				_gc_release(vm);
			}
			void gc_mark(const gc_iteration &gi) const { _gc_mark(gi); }

			template <class A> void push(A *&p, unsigned int addedsize) {
				unsigned int sz=sizeof(A);
				RCASSERT(actpos>=sizeof(A)+addedsize);
				actpos-=sizeof(A)+addedsize;
				p=new (ptr(actpos)) A();
			}
			op_flow_data_object *peek();
			void pop(virtual_machine &vm);
			bool empty() const { return actpos==totalsize; }
		protected:
			void _mark_gc(const gc_iteration &gc) const;
			void _release_resources(virtual_machine &vm);
			void _link_to_destroy(virtual_machine &vm);
		};

	}
}
#endif
