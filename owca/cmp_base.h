#ifndef _RC_Y_CMP_BASE_H
#define _RC_Y_CMP_BASE_H

#include "location.h"
#include "owca_message_type.h"
#include "tokentype.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class token;
		enum tokentype;
		class compiler;
		class error_information;
	}
}

namespace owca {
	namespace __owca__ {
		class token {
			friend class compiler;
			std::string _text;
			tokentype _type;
			unsigned int line_;
		public:
			token(tokentype type_, const std::string text_, unsigned int line__) : _type(type_),_text(text_),line_(line__) { }
			const std::string &text() const { return _text; }
			tokentype type() const { return _type; }
			unsigned int line() const { return line_; }
		};

		class error_information {
		public:
			owca_message_type mt;
			unsigned int line;
            std::string param;

			error_information(owca_message_type mt_, token *t, const std::string &param = "") : mt(mt_),line(t->line()),param(param) {
				int z;
				z=1+2;
			}
			error_information(owca_message_type mt_, token &t, const std::string &param = "") : mt(mt_),line(t.line()),param(param) {
				int z;
				z=1+2;
			}
			error_information(owca_message_type mt_, unsigned int loc, const std::string &param = "") : mt(mt_),line(loc),param(param) {
				int z;
				z=1+2;
			}
		};
	}
}
#endif
