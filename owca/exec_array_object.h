#ifndef _RC_Y_EXEC_ARRAY_OBJECT_H
#define _RC_Y_EXEC_ARRAY_OBJECT_H

#include "structinfo.h"

namespace owca {
	class gc_iteration;
	namespace __owca__ {
		class exec_array_object;
		class exec_variable;
		class virtual_machine;
	}
}

namespace owca { namespace __owca__ {
	class exec_array_object : public object_base {
		exec_variable *vars;
		unsigned int size_;
	public:
		exec_array_object(virtual_machine &vm, unsigned int oversize) : vars(NULL),size_(0) { }
		~exec_array_object() { RCASSERT(vars==NULL); }

		exec_variable *ptr() const { return vars; }
		DLLEXPORT exec_variable &get(unsigned int index);

		DLLEXPORT void resize(virtual_machine &vm, unsigned int newsize);
		void swap(exec_array_object *);
		unsigned int size() const { return size_; }

		void insert(virtual_machine &vm, unsigned int index, const exec_variable &v);
		void pop(exec_variable &dst, virtual_machine &vm, unsigned int index);

		static exec_variable *_allocate_table(virtual_machine &vm, unsigned int size);
		static void _release_table(virtual_machine &vm, exec_variable *table);
		exec_variable *_swap_table(exec_variable *new_table);

		void _marker(const gc_iteration &gc) const;
		void _destroy(virtual_machine &vm);
		void _release_array(virtual_machine &vm);
	};
} }
#endif
