#ifndef _RC_Y_EXEC_COMPARE_ARRAYS_H
#define _RC_Y_EXEC_COMPARE_ARRAYS_H

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

	class exec_compare_arrays {
	public:
		virtual_machine *vm;
		const exec_variable *a1,*a2;
		unsigned int s1,s2;
		exec_variable ret/*,tmp[2]*/;

		unsigned int index;

		enum {
			LESS,EQ,MORE,CALL_1,CALL_2,START
		} result;

		exec_compare_arrays() { }

		void next(bool order);
		void _mark_gc(const gc_iteration &gc) const { }
		void _release_resources(virtual_machine &vm) { }
	};

} }

#endif
