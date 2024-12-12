#ifndef _RC_Y_EXEC_STACK_H
#define _RC_Y_EXEC_STACK_H

#include "structinfo.h"
#include "exec_base.h"
#include "exec_variablelocation.h"
#include "exec_variable.h"

namespace owca {
	class gc_iteration;
	namespace __owca__ {
		class exec_base;
		class exec_stack;
		class exec_function_ptr;
		class exec_stack_variables;
		class exec_variable;
		class exec_variable_location;
		class virtual_machine;
	}
}

namespace owca { namespace __owca__ {
	class exec_stack_variables : public exec_base {
		unsigned int size_;
		exec_function_ptr *owner_;
	protected:
		void _mark_gc(const gc_iteration &gc) const;
		void _release_resources(virtual_machine &vm);
	public:
#ifdef RCDEBUG
		exec_variable *vars;
#endif
		exec_stack_variables(unsigned int size, exec_function_ptr *own);

		exec_variable &get(unsigned int index) {
			RCASSERT(index<size_);
			exec_variable *arr=((exec_variable*)((char*)this+sizeof(*this)));
			return arr[index];
		}
		const exec_variable &get(unsigned int index) const { RCASSERT(index<size_); return ((exec_variable*)((char*)this+sizeof(*this)))[index]; }
		exec_function_ptr *owner() const { return owner_; }
		unsigned int size() const { return size_; }
		static unsigned int calculate_size(unsigned int size);
		void gc_acquire() { _gc_acquire(); }
		void gc_release(virtual_machine &vm) { _gc_release(vm); }
		void gc_mark(const gc_iteration &gi) const { _gc_mark(gi); }
	};

	class exec_stack : public exec_base {
		friend class exec_function_ptr;
		unsigned int size_;
		exec_namespace *ns;
		unsigned int *level0map;
	protected:
		void _mark_gc(const gc_iteration &gc) const;
		void _release_resources(virtual_machine &vm);
	public:
#ifdef RCDEBUG
		exec_stack_variables **stackvariables;
#endif
		exec_stack(exec_namespace *ns_, unsigned int *level0map_);
		exec_stack(exec_function_ptr *fnc);
		exec_stack(exec_stack *prev);

		exec_variable &get_local(const exec_variable_location &i) { return get_variables(i.depth())->get(i.offset()); }
		const exec_variable &get_local(const exec_variable_location &i) const  { return get_variables(i.depth())->get(i.offset()); }

		const exec_namespace *get_namespace() const { return ns; }
		exec_stack_variables *get_variables(unsigned int index) { 
			RCASSERT(index>0 && index<=size_); 
			return ((exec_stack_variables**)((char*)this+sizeof(*this)))[index-1]; 
		}
		const exec_stack_variables *get_variables(unsigned int index) const { 
			RCASSERT(index>0 && index<=size_); 
			return ((const exec_stack_variables**)((char*)this+sizeof(*this)))[index-1]; 
		}
		void set_variables(unsigned int index, exec_stack_variables *p) { 
			RCASSERT(index>0 && index<=size_); 
			((exec_stack_variables**)((char*)this+sizeof(*this)))[index-1]=p; 
		}

		exec_variable &get_0(unsigned int index);

		static unsigned int calculate_size(exec_function_ptr *fnc);
		static unsigned int calculate_size(exec_stack *prev);
		unsigned int size() const { return size_; }
		exec_variable &get_variable(const exec_variable_location &i) {
			if (i.depth()==0) return get_0(i.offset());
			return get_variables(i.depth())->get(i.offset());
		}

		void gc_acquire() { _gc_acquire(); }
		void gc_release(virtual_machine &vm) { _gc_release(vm); }
		void gc_mark(const gc_iteration &gi) const { _gc_mark(gi); }
	};

	class exec_stack_element_object : public object_base {
		vm_execution_stack_elem_base *elem;
		bool inited;
	public:
		struct valueelem {
			owca_internal_string *ident;
			exec_variable *value;
		};
		std::vector<valueelem> elements;

		int find(owca_internal_string *) const;
		void init(void);

		exec_stack_element_object(virtual_machine &, unsigned int oversize) : elem(NULL),inited(false) { }

		vm_execution_stack_elem_base *element() const { return elem; }
		void set_element(vm_execution_stack_elem_base *e);

		//void _create();
		void _marker(const gc_iteration &gc) const;
		void _destroy(virtual_machine &vm);
	};
} }
#endif
