#ifndef _RC_Y_VM_EXECUTION_STACK_ELEM_BASE_H
#define _RC_Y_VM_EXECUTION_STACK_ELEM_BASE_H

#include "exec_base.h"
#include "exec_variable.h"
#include "returnvalueflow.h"
#include "executionstackreturnvalue.h"

namespace owca {
	namespace __owca__ {
		class exec_function_ptr;
		class exec_variable;
		class exec_object;
	}
}

namespace owca {
	namespace __owca__ {

		struct vm_execution_stack_elem_base : public exec_base {
		private:
			unsigned char _options;
			enum {
				OPTION_FIRST_TIME_RUN=1,
				OPTION_CATCH_EXCEPTIONS=2,
				OPTION_SHOW_IN_EXCEPTION_STACK=4,
				OPTION_IS_DONE=8,
				OPTION_FINALIZED=16,
			};

		public:

			enum return_handling_mode_ {
				RETURN_HANDLING_NONE,
				RETURN_HANDLING_NONE_FORCE_RETURN,
				RETURN_HANDLING_OPERATOR_RETURN_INIT,
				RETURN_HANDLING_OPERATOR_RETURN_INIT_EXCEPTION,
				RETURN_HANDLING_OPERATOR_RETURN_CANT_BE_UNUSED,
				RETURN_HANDLING_OPERATOR_RETURN_UNUSED_UNSUPPORTED,
				RETURN_HANDLING_OPERATOR_RETURN_BOOL,
				RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
				RETURN_HANDLING_OPERATOR_RETURN_BOOL_UPDATE_OPER,
				RETURN_HANDLING_OPERATOR_RETURN_INT,
				RETURN_HANDLING_OPERATOR_RETURN_STR,
			};
			exec_function_ptr *fnc;
			exec_variable *return_value;
			returnvalueflow r;
			return_handling_mode_ return_handling_mode;
			exec_object *init_object;
			exec_variable *oper_2_values;
			unsigned char operator_index;
			unsigned char operator_mode;

			vm_execution_stack_elem_base() : r(returnvalueflow::FIN),return_handling_mode(RETURN_HANDLING_NONE),_options(OPTION_FIRST_TIME_RUN),
							init_object(NULL),oper_2_values(NULL),fnc(NULL) { }
			DLLEXPORT ~vm_execution_stack_elem_base();

			bool catch_exceptions() const { return (_options & OPTION_CATCH_EXCEPTIONS)!=0; }
			void catch_exceptions(bool q) { if (q) _options|=OPTION_CATCH_EXCEPTIONS; else _options&=~OPTION_CATCH_EXCEPTIONS; }
			bool first_time_run() const { return (_options & OPTION_FIRST_TIME_RUN)!=0; }
			void first_time_run(bool q) { if (q) _options|=OPTION_FIRST_TIME_RUN; else _options&=~OPTION_FIRST_TIME_RUN; }
			bool show_in_exception_stack() const { return (_options & OPTION_SHOW_IN_EXCEPTION_STACK)!=0; }
			void show_in_exception_stack(bool q) { if (q) _options|=OPTION_SHOW_IN_EXCEPTION_STACK; else _options&=~OPTION_SHOW_IN_EXCEPTION_STACK; }
			bool is_done() const { return (_options & OPTION_IS_DONE)!=0; }
			void is_done(bool q) { if (q) _options|=OPTION_IS_DONE; else _options&=~OPTION_IS_DONE; }
			bool finalized() const { return (_options & OPTION_FINALIZED)!=0; }
			void finalized(bool q) { if (q) _options|=OPTION_FINALIZED; else _options&=~OPTION_FINALIZED; }

			virtual executionstackreturnvalue first_time_execute(executionstackreturnvalue mode)=0;
			virtual executionstackreturnvalue execute(executionstackreturnvalue)=0;

			DLLEXPORT void _mark_gc(const gc_iteration &gc) const;
			DLLEXPORT void _release_resources(virtual_machine &vm);
			virtual bool internal_frame() const { return false; }
		};

	}
}

#define CASE(index) \
	case index: goto mode_ ## index; \
	mode_ ## index : ;
#define GOTO(index) do { mode=index; goto mode_ ## index ; } while(0)
#define CALC(index,boolval) \
	do { if (!(boolval)) GOTO(index); \
	mode=index; \
	return executionstackreturnvalue::FUNCTION_CALL; } while(0)
#define NEXT(index,val) \
	do { val; \
	mode=index; \
	return executionstackreturnvalue::FUNCTION_CALL; } while(0)

#endif
