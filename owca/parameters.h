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

		DLLEXPORT void check_parameter_count(unsigned int min_count, unsigned int max_count=MAX) const; // false if exception thrown
		void convert_parameters_impl(unsigned int) const {}
		template <typename A, typename ... B> void convert_parameters_impl(unsigned int index, A& a, B & ... b) const {
			auto val = get(index);
			val.convert(*vm->owner_vm, a, std::to_string(index + 1).c_str());
			convert_parameters_impl(index + 1, b...);
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
		DLLEXPORT void get_arguments(unsigned int *arg_count, owca_global *values, unsigned int mincount, unsigned int maxcount=MAX) const;
		DLLEXPORT void get_keyword_arguments(unsigned int *arg_count, owca_global *values, const owca_string *identificators, unsigned int count, bool *used = NULL, bool *required = NULL) const;
		//void get(std::vector<owca_global> &params, std::vector<owca_global> &keyparams, unsigned int mincount, unsigned int maxcount, const owca_string *keyidents, unsigned int keyidentscount) const;


		template <class ... ARGS> void convert_parameters(ARGS & ... a) const {
			check_parameter_count(sizeof...(a));
			convert_parameters_impl(0, a...);
		}
	};
	struct owca_call_parameters {
		std::vector<owca_global> parameters;
		owca_global list_parameter;
		owca_global map_parameter;
	};
}
#endif
