#include "stdafx.h"
#include "base.h"
#include "class.h"
#include "exec_class_int.h"
#include "vm.h"
#include "structinfo.h"
#include "virtualmachine.h"
#include "namespace.h"
#include "returnvalue.h"
#include "exec_object.h"
#include "exec_namespace.h"

namespace owca {

	//void owca_object_base::_create(__owca__::virtual_machine &vm, unsigned int oversize)
	//{
	//	create(*vm.owner_vm,oversize);
	//}

	//void owca_object_base::_destroy(__owca__::virtual_machine &vm)
	//{
	//	destroy(*vm.owner_vm);
	//}

	//void owca_object_base::_marker(const gc_iteration &gc) const
	//{
	//	marker(gc);
	//}

	owca_class::owca_class(owca_namespace &nspace, const std::string &name_)
	{
		ic=new __owca__::internal_class(*nspace.ns,name_);
	}

	owca_class::~owca_class()
	{
		delete ic;
	}

	owca_class::obj_constructor owca_class::operator [] (const owca_string &ident)
	{
		return owca_class::obj_constructor(*ic, ident);
	}

	void owca_class::_set_struct(owca::__owca__::structinfo type)
	{
		ic->_setstructure(type);
	}

	owca_function_return_value owca_class::construct(owca_global &ret)
	{
		__owca__::vmstack _vmstack(ic->vm);
		__owca__::exec_object *o;
		std::string rt=ic->create(o);
		if (!rt.empty()) {
			ret=ic->vm.owner_vm->construct_builtin_exception(ExceptionCode::CLASS_CREATION,rt);
			return owca_function_return_value::EXCEPTION;
		}
		ret._object.gc_release(*ret._vm);
		ret._update_vm(&ic->vm);
		ret._object.set_object(o);
		return owca_function_return_value::RETURN_VALUE;
	}

	void owca_class::add_inheritance(const owca_global &g)
	{
		if (g._object.mode()!=__owca__::VAR_OBJECT || g._object.get_object()->is_type()) {
			throw owca_exception("not a type");
		}
		ic->_add_inherit(g._object.get_object());
	}
}
