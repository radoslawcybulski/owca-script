#ifndef _RC_Y_CMP_COMPILER_H
#define _RC_Y_CMP_COMPILER_H

#include "cmp_base.h"

namespace owca {
	class owca_location;
	class owca_message_list;
	class owca_source_file;
	namespace __owca__ {
		class error_information;
		class token;
		class compiler;
		class tree_function;
		class tree_executable;
		class tree_varspace;
		class virtual_machine;
		class owca_internal_string;
		class compile_visible_items;
	}
}

namespace owca {
	namespace __owca__ {
		class compiler {
			owca_message_list &errorswarnings;
			void add_token(tokentype type, const std::string &text, unsigned int line);

			void tokenize(const owca_source_file &fs);
		public:
			std::list<token> tokens;
			virtual_machine &vm;
			tree_varspace *actual_scope;
			tree_function *fncowner;
			std::list<bool> in_exception_handler;

			compiler(virtual_machine &vm_, owca_message_list &errorswarnings_);

			void error(const error_information &);
			void error(owca_message_type yt, unsigned int line, std::string param);
			void error(owca_message_type yt, const token *t, std::string param) { error(yt,t->line(),param); }
			std::vector<unsigned char> compile(const owca_source_file &fs, const compile_visible_items &visible_names);

			owca_int get_int(unsigned int line, const std::string &);
			owca_real get_real(unsigned int line, const std::string &);
			bool parse_string(std::string &res, unsigned int line, const std::string &);

			//owca_internal_string *identificator(const std::string &ident);
			//owca_internal_string *get_string(const owca_location &l, const std::string &);
		};
} }
#endif
