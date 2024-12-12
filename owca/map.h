#include "obj_constructor_base.h"

#ifndef _RC_Y_MAP_H
#define _RC_Y_MAP_H

#include "exec_map_object_iterator.h"

namespace owca {
	class owca_local;
	class owca_global;
	class owca_map;
	namespace __owca__ {
		class obj_constructor_base;
		class virtual_machine;
		class exec_map_object;
		class exec_object;
	}
}

namespace owca {
	class owca_map;

	class owca_map_iterator {
		friend class owca_map;
		__owca__::exec_map_object_iterator iter;
	public:
	};

	class owca_map {
		friend class owca_local;
		friend class __owca__::obj_constructor_base;
		__owca__::exec_map_object *mo;
		__owca__::exec_object *obj;
		__owca__::virtual_machine *vm;
	public:
		owca_map() : vm(NULL),obj(NULL),mo(NULL) { }
		bool not_bound() const { return mo==NULL; }

		bool empty(void) const { return size() == 0; }

		DLLEXPORT unsigned int size() const;
		DLLEXPORT void clear();

		DLLEXPORT owca_global clone() const;

		DLLEXPORT bool next(owca_map_iterator &, owca_global &key, owca_global &value) const;
	};

}

#endif
