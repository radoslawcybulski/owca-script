#include "obj_constructor_base.h"

#ifndef _RC_Y_TUPLE_H
#define _RC_Y_TUPLE_H

namespace owca {
	class owca_global;
	class owca_local;
	class owca_tuple;
	namespace __owca__ {
		class virtual_machine;
		class exec_tuple_object;
		class exec_object;
		class obj_constructor_base;
	}
}

namespace owca {

	class owca_tuple {
		friend class owca_local;
		friend class __owca__::obj_constructor_base;
		__owca__::exec_tuple_object *to;
		__owca__::exec_object *obj;
		__owca__::virtual_machine *vm;

		DLLEXPORT unsigned int _update_index(owca_int index) const;
	public:
		owca_tuple() : vm(NULL),obj(NULL),to(NULL) { }
		bool not_bound() const { return to==NULL; }
		bool empty(void) const { return size() == 0; }

		DLLEXPORT unsigned int size() const;
		DLLEXPORT owca_global clone() const;

		DLLEXPORT owca_global get(owca_int index) const;
		DLLEXPORT owca_global get(owca_int from, owca_int to) const;

		DLLEXPORT owca_global operator [] (owca_int index) const;
	};
}

#endif
