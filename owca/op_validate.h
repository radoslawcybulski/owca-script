#ifndef _RC_Y_OP_VALIDATE_H
#define _RC_Y_OP_VALIDATE_H

#include "debug_execution.h"
#include "debug_opcodes.h"
#include "op_execute.h"
#include "vm_execution_stack_elem_internal.h"
#include "operatorcodes.h"

namespace owca {
	class gc_iteration;
	namespace __owca__ {
		//template <class A> class exec_array;
		class opcode_executer;
		class opcode_executer_jump;
		class opcode_validator;
		class exec_variable_location;
		class virtual_machine;
		class owca_internal_string;
		class store_uint_or_ptr;
	}
}

namespace owca {
	namespace __owca__ {

		// TODO: validate exec_variable_location values

		class opcode_data {
			std::vector<unsigned char> opcodes;
			std::vector<owca_internal_string*> strings;
			std::vector<vm_execution_stack_elem_internal::locationinfo> location_infos;
			std::vector<unsigned int> required_variable_name_index, not_required_variable_name_index;
            std::string file_name;
			unsigned top_level_stack_data_size, top_level_max_temporary_variables;
			friend class opcode_validator;
		public:
            opcode_data(std::string file_name) : file_name(file_name) { }

			const unsigned char *get_opcodes(void) const { return &opcodes[0]; }
			unsigned int get_top_level_stack_data_size(void) const { return top_level_stack_data_size; }
			unsigned int get_top_level_max_variable_count(void) const { return top_level_max_temporary_variables; }
			owca_internal_string *get_string_by_index(unsigned int index) const { return strings[index]; }
            std::string get_file_name() const { return file_name; }

			const std::vector<unsigned int> &get_required_variable_name_index(void) const { return required_variable_name_index; }
			const std::vector<unsigned int> &get_not_required_variable_name_index(void) const { return not_required_variable_name_index; }

			owca_location get_location_from_opcode_index(unsigned int opcode_index) const;
			void gc_mark(const gc_iteration &gi) const;
			void gc_release(virtual_machine &vm);
		};
		class opcode_validator {
			struct stacksizeinfo {
				unsigned int current,maximum;
				unsigned int tempcurrent,tempmaximumfinal,tempmaximum;
				unsigned int stackyieldcurrent,stackyieldmaximumfinal,stackyieldmaximum;
				stacksizeinfo(unsigned int mx, unsigned int tempmx, unsigned int stackyieldmx) :
							current(0),maximum(mx),
							tempcurrent(0),tempmaximumfinal(tempmx),tempmaximum(0),
							stackyieldcurrent(0),stackyieldmaximumfinal(stackyieldmx),stackyieldmaximum(0) { }
			};
			std::vector<stacksizeinfo> stacksizes;
			std::vector<unsigned char> data;
			std::vector<owca_internal_string*> strings;
			std::vector<unsigned int> indexes_1, indexes_2;
			std::vector<vm_execution_stack_elem_internal::locationinfo> location_infos;
			opcode_data *result;
			unsigned int string_count;
			unsigned int offset,begin_code_ofset;
			execopcode lastopcode;
		public:
			class boolean_result {
				bool r;
			public:
				boolean_result(bool q) : r(q) { }
				operator bool(void) const { return r; }
			};
		private:
#ifdef RCDEBUG_EXECUTION
			std::map<unsigned int,std::string> debugcodemap;

			void setdebugcodemap(unsigned int index1, unsigned int index2, const char *params, ...);
#else
			void setdebugcodemap(unsigned int index1, unsigned int index2, const char *params, ...) { }
#endif

			opcode_validator(virtual_machine &vm_, const std::vector<unsigned char> &data, opcode_data *result_);
#ifdef RCDEBUG_OPCODES
			void ensure_type(opcodestreamtype);
#else
			void ensure_type(opcodestreamtype) { }
#endif
			opcode_validator::boolean_result validate();
			owca_internal_string *check_string(void);
		public:
			virtual_machine &vm;
			bool continue_opcodes,execution_can_continue;

			//opcode_executer_jump actual_code_position() const { opcode_executer_jump j; j.offset=offset; return j; }
			//void set_code_position(const opcode_executer_jump &j) { offset=j.offset; }
			char compare(const opcode_executer_jump &oe) const {
				if (offset == oe.offset + begin_code_ofset) return 0;
				return offset < oe.offset + begin_code_ofset ? -1 : 1;
			}
			opcode_validator::boolean_result peek_exec(bool &ret);

			unsigned int stack_depth() const { return (unsigned int)stacksizes.size(); }
			void push_stack_size(unsigned int max_local_variables, unsigned int max_temporary_variables, unsigned int max_local_stack_data_size);
			boolean_result pop_stack_size();
			boolean_result push_temporary_variable();
			boolean_result pop_temporary_variable();
			boolean_result pop_temporary_variables(unsigned int cnt);
			boolean_result push_local_stack_data_size(unsigned int sz);
			boolean_result pop_local_stack_data_size(unsigned int sz);
			boolean_result check(const exec_variable_location &varloc);
			boolean_result check(unsigned int vaoff);

			boolean_result get_operands(unsigned int count); // and push return value
			boolean_result get_final_value();
			boolean_result get_no_value();
			boolean_result validate_flow();
			boolean_result validate_cont_expr();
			boolean_result validate_read_expr();
			boolean_result validate_write_expr_que();
			boolean_result validate_write_expr_assign_tuple();
			boolean_result validate_write_expr_for();
			boolean_result validate_end_of_data() const { return offset == data.size(); }
			unsigned int temporary_variables_count() const;
			static boolean_result validate(virtual_machine &vm, const std::vector<unsigned char> &opcodes, opcode_data *result = NULL);
			boolean_result get(operatorcodes &opc);
			boolean_result get(unsigned int &val);
			boolean_result get(opcode_executer_jump &val);
			boolean_result get(owca_internal_string *&);
			boolean_result get(store_uint_or_ptr &);
			boolean_result get(exec_variable_location &);
			boolean_result get(owca_int &);
			boolean_result get(owca_real &);
			boolean_result fin();

			void debug();
		};

	}
}

#endif
