#include "stdafx.h"
#include "base.h"
#include "virtualmachine.h"
#include "namespace.h"
#include "exec_namespace.h"
#include "vm.h"
#include "exec_string.h"
#include "string.h"
#include "vm_execution_stack_elem_external.h"
#include "exec_map_object.h"
#include "exec_class_object.h"
#include "exec_function_ptr.h"
#include "exec_object.h"
#include "exec_coroutine.h"
#include "exec_string.h"
#include "exec_class_int.h"
#include "op_validate.h"

namespace owca {
	using namespace __owca__;

	bool owca_namespace::validate_code(const std::vector<unsigned char> &data) const
	{
		return ns->validate_code_and_prepare_for_execution(data, NULL);
	}

    std::string owca_namespace::get_file_name() const
    {
        return ns->get_file_name()->str();
    }

	owca_function_return_value owca_namespace::apply_code(owca_global &result, const std::vector<unsigned char> &data)
	{
        opcode_data *opc = new opcode_data(get_file_name());
		if (!ns->validate_code_and_prepare_for_execution(data, opc)) {
			delete opc;
			throw owca_exception(ExceptionCode::CODE_FAILED_TO_VALIDATE, "input code failed to validate");
		}

		result._object.gc_release(*result._vm);
		result._object.reset();
		result._update_vm(&ns->vm);
		ns->vm.register_opcode_data(opc);
		return ns->apply_code(result._object, opc);
	}

	void owca_namespace::clear()
	{
		ns->clear();
	}

    owca_global owca_namespace::copy(std::string file_name)
    {
        owca_global result;
        result._update_vm(&ns->vm);
        owca_internal_string *name = ns->vm.allocate_string(file_name);
        result._object.set_namespace(ns->copy(name));
        name->gc_release(ns->vm);
        return result;
    }

	owca_namespace::obj_constructor::obj_constructor(owca_namespace &ns_, __owca__::owca_internal_string *name_) : obj_constructor_function(ns_.ns,name_)
	{
		_update_vm(g);
	}

	void owca_namespace::obj_constructor::_write(const exec_variable &val)
	{
		//owca_internal_string_nongc *n=owca_internal_string_nongc::allocate_nongc(name.c_str(),(unsigned int)name.size());
		int z=nspace->insert_variable(name,val);
		RCASSERT(z>=0);
	}

	void owca_namespace::obj_constructor::_read(exec_variable &val) const
	{
		//owca_internal_string_nongc *n=owca_internal_string_nongc::allocate_nongc(name.c_str(),(unsigned int)name.size());
		bool b=nspace->get_variable(name,val);
		//n->gc_release(*(virtual_machine*)NULL);

		if (!b) throw owca_exception(ExceptionCode::MISSING_MEMBER, OWCA_ERROR_FORMAT1("namespace object is missing member %1",name->str()));
	}

	owca_namespace::obj_constructor owca_namespace::operator [] (const owca_string &s)
	{
		if (internal_class::_check_name(s.data(), s.data_size())) return obj_constructor(*this, s._ss);
		throw owca_exception(ExceptionCode::INVALID_IDENT, OWCA_ERROR_FORMAT("invalid identificator"));
	}

	owca_vm &owca_namespace::vm()
	{
		return *ns->vm.owner_vm;
	}

}
