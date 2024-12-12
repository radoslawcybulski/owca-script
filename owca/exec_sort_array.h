#ifndef _RC_Y_EXEC_SORT_ARRAY_H
#define _RC_Y_EXEC_SORT_ARRAY_H

#include "exec_variable.h"
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
	}
}

namespace owca { namespace __owca__ {

	class exec_sort_array_getter_base {
	public:
		virtual const exec_variable &next(owca_int &hash)=0;
	};

	class exec_sort_array_result;
	class exec_sort_array {
		friend class exec_sort_array_result;
		struct elem {
			owca_int hash;
			const exec_variable *ptr;
			elem *next;
		};
		struct recursiveinfo {
			elem **array,*pivot;
			elem *left,*right,*left_last,*right_last,**last_ptr;
#ifdef RCDEBUG
			unsigned int count;
#endif
		};

		unsigned char push_new_sort_array(elem **array, elem **left_last_ptr);
		void prepare_compare();

		virtual_machine *vm;
		unsigned int size;
		elem *sortarray,*left_last,*right_last,*root;
		recursiveinfo *recarray,*recact,*recmax;
		exec_variable ret;

		enum mode_ {
			MODE_COMPARE,MODE_DECIDE,
		} mode;

	public:
		exec_sort_array() : sortarray(NULL) { }

		bool create(virtual_machine *, exec_sort_array_getter_base &getter, unsigned int size_);
		bool next();
		exec_sort_array_result result();
		void _mark_gc(const gc_iteration &gc) const { }
		void _release_resources(virtual_machine &vm) { delete [] sortarray; }
	};

	class exec_sort_array_result {
		exec_sort_array::elem *el,*root;
	public:
		exec_sort_array_result() : el(NULL),root(NULL) { }
		exec_sort_array_result(exec_sort_array::elem *root_, exec_sort_array::elem *el_) : root(root_),el(el_){ }

		bool valid() const { return el!=NULL; }
		const exec_variable &value() const { return *el->ptr; }
		unsigned int index() const { RCASSERT(el>=root); return (unsigned int)(el-root); }
		void next() { el=el->next; }
	};
} }

#endif
