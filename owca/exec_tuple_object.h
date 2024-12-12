#ifndef _RC_Y_EXEC_TUPLE_OBJECT_H
#define _RC_Y_EXEC_TUPLE_OBJECT_H

#include "structinfo.h"

namespace owca {
	class gc_iteration;
	namespace __owca__ {
		class exec_tuple_object;
		class exec_variable;
		class virtual_machine;
	}
}

namespace owca { namespace __owca__ {
	class exec_tuple_object : public object_base {
		unsigned int size_;
	public:
		exec_tuple_object(virtual_machine &, unsigned int oversize);

		unsigned int size() const { return size_; }
		const exec_variable *ptr() const;
		DLLEXPORT exec_variable &get(unsigned int index);
		const exec_variable &get(unsigned int index) const;

		//void _create(virtual_machine &, unsigned int oversize);
		void _marker(const gc_iteration &gc) const;
		void _destroy(virtual_machine &vm);
	};
} }
#endif
