#include "stdafx.h"
#include "base.h"
#include "exec_function_stack_data.h"
#include "op_while.h"
#include "op_with.h"
#include "op_try.h"
#include "op_if.h"
#include "op_for.h"

namespace owca {
	namespace __owca__ {

		op_flow_data_object *exec_function_stack_data::peek()
		{
			RCASSERT(actpos<totalsize);
			return ptr(actpos);
		}

		void exec_function_stack_data::pop(virtual_machine &vm)
		{
			op_flow_data_object *p=peek();
			p->_release_resources(vm);
			unsigned int sz=p->size();
			RCASSERT(actpos+sz<=totalsize);
			actpos+=sz;
		}

		void exec_function_stack_data::_mark_gc(const gc_iteration &gc) const
		{
			unsigned int p=actpos;
			while(p<totalsize) {
				gc_iteration::debug_info _d("exec_function_stack_data: at pos %d",p);
				const op_flow_data_object *a=ptr(p);
				a->_mark_gc(gc);
				unsigned int sz=a->size();
				RCASSERT(p+sz<=totalsize);
				p+=sz;
			}
		}

		void exec_function_stack_data::_release_resources(virtual_machine &vm)
		{
			while (actpos < totalsize) {
				pop(vm);
			}
		}

		void exec_function_stack_data::_link_to_destroy(virtual_machine &vm)
		{
			_delay_destruction(vm);
			_release_resources(vm);
		}
	}
}

