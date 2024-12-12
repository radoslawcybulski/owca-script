#include "global.h"

#ifndef _RC_Y_PARAMETERS_H
#define _RC_Y_PARAMETERS_H

#include "exec_map_object_iterator.h"
#include "virtualmachine.h"

namespace owca {
	class owca_global;
	class owca_local;
	class owca_parameters;
	class owca_string;
	namespace __owca__ {
		struct callparams;
		class virtual_machine;
		class obj_constructor_base;
		DLLEXPORT owca_parameters _generate_parameters(virtual_machine &vm, const __owca__::callparams &ci);
	}
	class owca_user_function_base_object;
}

namespace owca {
	class owca_parameters {
		friend owca_parameters __owca__::_generate_parameters(__owca__::virtual_machine &vm, const __owca__::callparams &ci);
		friend class owca_user_function_base_object;
		friend class owca_local;
		const __owca__::callparams *ci;
		__owca__::virtual_machine *vm;

		void _init(__owca__::virtual_machine &vm_, const __owca__::callparams &ci_) {
			vm=&vm_;
			ci=&ci_;
		}
	public:
		class map_iterator {
			friend class owca_parameters;
			__owca__::exec_map_object_iterator iter;
		public:
		};

		static const unsigned int MAX=0xffffffff;

		DLLEXPORT unsigned int parameters_count() const;
		DLLEXPORT unsigned int list_parameters_count() const;
		DLLEXPORT owca_global parameter(unsigned int index) const;
		DLLEXPORT owca_global list_parameter(unsigned int index) const;
		DLLEXPORT bool map_parameter(map_iterator &mi, owca_global &key, owca_global &value) const;

		DLLEXPORT unsigned int count() const;
		DLLEXPORT owca_global get(unsigned int index) const;
		DLLEXPORT bool get_arguments(owca_global &exception_object, unsigned int *arg_count, owca_global *values, unsigned int mincount, unsigned int maxcount=MAX) const;
		DLLEXPORT bool get_keyword_arguments(owca_global &exception_object, unsigned int *arg_count, owca_global *values, const owca_string *identificators, unsigned int count, bool *used = NULL, bool *required = NULL) const;
		//void get(std::vector<owca_global> &params, std::vector<owca_global> &keyparams, unsigned int mincount, unsigned int maxcount, const owca_string *keyidents, unsigned int keyidentscount) const;


		DLLEXPORT bool check_parameter_count(owca_global &exception_object, unsigned int min_count, unsigned int max_count=MAX) const; // false if exception thrown
		template <class A> bool convert_parameters(owca_global &exception_object, A &a) const {
			if (!check_parameter_count(exception_object, 1) || !get_keyword_arguments(exception_object, NULL, NULL, NULL, 0)) return false;
			return get(0).convert(exception_object,*vm->owner_vm,a,"1");
		}
		template <class A, class B> bool convert_parameters(owca_global &exception_object, A &a, B &b) const {
			if (!check_parameter_count(exception_object, 2) || !get_keyword_arguments(exception_object, NULL, NULL, NULL, 0)) return false;
			return get(0).convert(exception_object,*vm->owner_vm,a,"1") &&
				get(1).convert(exception_object,*vm->owner_vm,b,"2");
		}
		template <class A, class B, class C> bool convert_parameters(owca_global &exception_object, A &a, B &b, C &c) const {
			if (!check_parameter_count(exception_object, 3) || !get_keyword_arguments(exception_object, NULL, NULL, NULL, 0)) return false;
			return get(0).convert(exception_object,*vm->owner_vm,a,"1") &&
				get(1).convert(exception_object,*vm->owner_vm,b,"2") &&
				get(2).convert(exception_object,*vm->owner_vm,c,"3");
		}
		template <class A, class B, class C, class D> bool convert_parameters(owca_global &exception_object, A &a, B &b, C &c, D &d) const {
			if (!check_parameter_count(exception_object, 4) || !get_keyword_arguments(exception_object, NULL, NULL, NULL, 0)) return false;
			return get(0).convert(exception_object,*vm->owner_vm,a,"1") &&
				get(1).convert(exception_object,*vm->owner_vm,b,"2") &&
				get(2).convert(exception_object,*vm->owner_vm,c,"3") &&
				get(3).convert(exception_object,*vm->owner_vm,d,"4");
		}
	};
	struct owca_call_parameters {
		std::vector<owca_global> parameters;
		owca_global list_parameter;
		owca_global map_parameter;
	};
}
#endif
