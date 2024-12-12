#ifndef _RC_Y_EXEC_CALL_PARAMS_H
#define _RC_Y_EXEC_CALL_PARAMS_H

namespace owca {
	namespace __owca__ {
		class exec_variable;
		class exec_map_object;
		class virtual_machine;
		class owca_internal_string;
	}
}

namespace owca {
	namespace __owca__ {

		struct callparams {
			const exec_variable *normal_params;
			unsigned int normal_params_count;

			const exec_variable *list_params;
			unsigned int list_params_count;

			exec_map_object *map;

			void set_simple() {
				map=NULL;
				list_params_count=0;
			}
		};

	}
}

#endif
