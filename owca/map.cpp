#include "stdafx.h"
#include "base.h"
#include "exec_map_object.h"
#include "exec_variable.h"
#include "virtualmachine.h"
#include "map.h"
#include "returnvalue.h"
#include "exception.h"
#include "global.h"
#include "exec_string.h"
#include "exec_object.h"
#include "exec_class_object.h"

namespace owca {
	using namespace __owca__;

	unsigned int owca_map::size() const
	{
		return mo->map.size();
	}

	void owca_map::clear()
	{
		mo->map.clear(*vm);
	}

	owca_global owca_map::clone() const
	{
		exec_map_object *oo;
		exec_object *o=vm->allocate_map(oo);

		oo->map.copy_from(*vm,mo->map);

		owca_global ret(*vm);
		ret._object.set_object(o);
		return ret;
	}

	bool owca_map::next(owca_map_iterator &mi, owca_global &key, owca_global &value) const
	{
		if (!mo->map.next(mi.iter)) return false;
		key._object.gc_release(*key._vm);
		value._object.gc_release(*value._vm);
		key._update_vm(vm);
		value._update_vm(vm);
		(key._object=mo->map.getkey(mi.iter).k).gc_acquire();
		(value._object=mo->map.getval(mi.iter)).gc_acquire();
		return true;
	}
}

