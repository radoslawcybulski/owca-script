#include "stdafx.h"
#include "base.h"
#include "exec_tuple_object.h"
#include "exec_variable.h"
#include "tuple.h"
#include "global.h"
#include "exception.h"
#include "virtualmachine.h"
#include "exec_tuple_object.h"
#include "exec_object.h"
#include "exec_string.h"
#include "exec_class_object.h"

namespace owca {
	using namespace __owca__;

	unsigned int owca_tuple::_update_index(owca_int index) const
	{
		if (index<0) index+=to->size();
		if (index<0 || index>=to->size()) throw owca_exception(OWCA_ERROR_FORMAT2("index %1 is invalid for tuple of size %2",int_to_string(index),int_to_string(to->size())));
		return (unsigned int)index;
	}

	owca_global owca_tuple::operator [] (owca_int index) const
	{
		owca_global g;
		g._object=to->get(_update_index(index));
		g._object.gc_acquire();
		g._update_vm(vm);
		return g;
	}

	unsigned int owca_tuple::size() const
	{
		return to->size();
	}

	owca_global owca_tuple::clone() const
	{
		exec_tuple_object *oo;
		exec_object *o=vm->allocate_tuple(oo,to->size());

		for(unsigned int i=0;i<oo->size();++i) {
			(oo->get(i)=to->get(i)).gc_acquire();
		}

		owca_global ret(*vm);
		ret._object.set_object(o);
		return ret;
	}

	owca_global owca_tuple::get(owca_int index) const
	{
		owca_global g;
		g._object=to->get(_update_index(index));
		g._object.gc_acquire();
		g._update_vm(vm);
		return g;
	}

	namespace __owca__ {
		void update_2index(owca_int &i1, owca_int &i2, owca_int size);
	}

	owca_global owca_tuple::get(owca_int from, owca_int to_) const
	{
		exec_tuple_object *oo;
		__owca__::update_2index(from,to_,to->size());
		RCASSERT(to_>=from);
		RCASSERT(from>=0);
		RCASSERT(to_<=to->size());

		exec_object *o=vm->allocate_tuple(oo,(unsigned int)(to_-from));

		for(unsigned int i=0;i<oo->size();++i) {
			(oo->get(i)=to->get((unsigned int)(from+i))).gc_acquire();
		}

		owca_global ret(*vm);
		ret._object.set_object(o);
		return ret;
	}
}

