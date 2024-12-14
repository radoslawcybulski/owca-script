#include "stdafx.h"
#include "base.h"
#include "parameters.h"
#include "global.h"
#include "vm.h"
#include "virtualmachine.h"
#include "exec_map_object.h"
#include "exec_variable.h"
#include "exception.h"
#include "string.h"
#include "exec_string.h"
#include "exec_callparams.h"

namespace owca {
	using namespace __owca__;

	unsigned int owca_parameters::count() const
	{
		return ci->normal_params_count+ci->list_params_count;
	}

	bool owca_parameters::check_parameter_count(owca_global &exception_object, unsigned int pcountmin, unsigned int pcountmax) const
	{
		if (pcountmax==MAX) pcountmax=pcountmin;
		if (count()<pcountmin) {
			vm->owner_vm->construct_builtin_exception(ExceptionCode::NOT_ENOUGH_PARAMETERS,OWCA_ERROR_FORMAT("not enough parameters given"));
			return false;
		}
		if (count()>pcountmax) {
			vm->owner_vm->construct_builtin_exception(ExceptionCode::TOO_MANY_PARAMETERS,OWCA_ERROR_FORMAT("too many parameters given"));
			return false;
		}
		return true;
	}

	owca_global owca_parameters::get(unsigned int index) const
	{
		const exec_variable *v;

		if (index<ci->normal_params_count) v=&ci->normal_params[index];
		else {
			index-=ci->normal_params_count;
			if (index<ci->list_params_count) v=&ci->list_params[index];
			else throw owca_exception(ExceptionCode::INVALID_PARAM_TYPE, OWCA_ERROR_FORMAT2("index %1 is too large - there's only %2 indexable parameters", int_to_string(index + ci->normal_params_count), int_to_string(ci->normal_params_count + ci->list_params_count)));
		}
		owca_global g;
		g._update_vm(vm);
		g._object=*v;
		g._object.gc_acquire();
		return g;
	}

	unsigned int owca_parameters::parameters_count() const
	{
		return ci->normal_params_count;
	}

	unsigned int owca_parameters::list_parameters_count() const
	{
		return ci->list_params_count;
	}

	owca_global owca_parameters::parameter(unsigned int index) const
	{
		if (index>=ci->normal_params_count) throw owca_exception(ExceptionCode::INVALID_PARAM_TYPE, OWCA_ERROR_FORMAT2("parameter index %1 is too large - there's only %2 parameters",int_to_string(index), int_to_string(ci->normal_params_count)));
		owca_global g(*vm,ci->normal_params[index]);
		g._object.gc_acquire();
		return g;
	}

	owca_global owca_parameters::list_parameter(unsigned int index) const
	{
		if (index>=ci->list_params_count) throw owca_exception(ExceptionCode::INVALID_PARAM_TYPE, OWCA_ERROR_FORMAT2("parameter list index %1 is too large - there's only %2 parameters",int_to_string(index), int_to_string(ci->list_params_count)));
		owca_global g(*vm,ci->list_params[index]);
		g._object.gc_acquire();
		return g;
	}

	bool owca_parameters::map_parameter(map_iterator &mi, owca_global &key, owca_global &value) const
	{
		if (ci->map && ci->map->map.next(mi.iter)) {
			owca_global k(*vm->owner_vm);
			k._object=ci->map->map.getkey(mi.iter).k;
			k._object.gc_acquire();
			owca_global v(*vm,ci->map->map.getval(mi.iter));
			v._object.gc_acquire();
			key=k;
			value=v;
			return true;
		}
		return false;
	}

	bool owca_parameters::get_keyword_arguments(owca_global &exception_object, unsigned int *arg_count, owca_global *values, const owca_string *identificators, unsigned int count, bool *used, bool *required) const
	{
		if (used) for(unsigned int i=0;i<count;++i) used[i]=false;

		if (ci->map==NULL) {
			if (required) {
				for(unsigned int i=0;i<count;++i) {
					if (required[i]) goto required_;
				}
			}
			else {
				if (count>0) {
required_:
					exception_object=vm->owner_vm->construct_builtin_exception(ExceptionCode::INVALID_PARAM_TYPE,
							OWCA_ERROR_FORMAT1("missing keyword parameter %1",identificators[0].str()));
					return false;
				}
			}
		}
		else {
			unsigned int usedcount=0;

			for(unsigned int i=0;i<count;++i) {
				exec_map_object mi(*vm,0);

				exec_variable *v=ci->map->ident_get(identificators[i]._ss);
				if (v==NULL) {
					if (!required || required[i]) {
						exception_object=vm->owner_vm->construct_builtin_exception(ExceptionCode::INVALID_PARAM_TYPE,OWCA_ERROR_FORMAT1("missing keyword parameter %1",identificators[i].str()));
						return false;
					}
					if (used) used[i]=false;
				}
				else {
					if (used) used[i]=true;
					values[i]._object.gc_release(*values[i]._vm);
					values[i]._object=*v;
					values[i]._object.gc_acquire();
					values[i]._update_vm(vm);
					++usedcount;
				}
			}
			if (usedcount!=ci->map->map.size()) {
				exec_map_object_iterator mi;
				while(ci->map->map.next(mi)) {
					exec_variable &k=ci->map->map.getkey(mi).k;
					if (k.mode()==VAR_STRING) {
						for(unsigned int i=0;i<count;++i) {
							if (identificators[i]._ss->equal(k.get_string())) {
								goto cont;
							}
						}
						exception_object=vm->owner_vm->construct_builtin_exception(ExceptionCode::INVALID_PARAM_TYPE,OWCA_ERROR_FORMAT1("unused keyword parameter %1",k.get_string()->str()));
						return false;
					}
					else {
						exception_object=vm->owner_vm->construct_builtin_exception(ExceptionCode::INVALID_PARAM_TYPE,OWCA_ERROR_FORMAT("keyword parameter is not a string"));
						return false;
					}
cont: ;
				}
			}
			if (arg_count != NULL)
				*arg_count = usedcount;
		}

		return true;
	}

	bool owca_parameters::get_arguments(owca_global &exception_object, unsigned int *arg_count, owca_global *params, unsigned int mincount, unsigned int maxcount) const
	{
		if (maxcount==MAX) maxcount=mincount;

		if (!check_parameter_count(exception_object, mincount, maxcount))
			return false;

		unsigned int index = 0;
		for(index=0;index<ci->normal_params_count;++index) {
			params[index]._object.gc_release(*params[index]._vm);
			params[index]._object=ci->normal_params[index];
			params[index]._object.gc_acquire();
			params[index]._update_vm(vm);
		}
		for(;index<ci->list_params_count+ci->normal_params_count;++index) {
			params[index]._object.gc_release(*params[index]._vm);
			params[index]._object=ci->list_params[index-ci->normal_params_count];
			params[index]._object.gc_acquire();
			params[index]._update_vm(vm);
		}
		RCASSERT(index == count());

		if (arg_count != NULL)
			*arg_count = index;
		return true;
	}
}

