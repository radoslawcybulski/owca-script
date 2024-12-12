#ifndef _RC_Y_CMP_SOURCE_H
#define _RC_Y_CMP_SOURCE_H

namespace owca {
	class owca_location;
	namespace __owca__ {
		class source;
		class token;
	}
}

#include "cmp_base.h"

namespace owca { namespace __owca__ {
	class source {
		std::list<token>::iterator b,a,e;
	public:
		source(std::list<token>::iterator beg_, std::list<token>::iterator end_) : b(beg_),a(beg_),e(end_) { }
		void iter_actual(std::list<token>::iterator s) { a=s; }
		std::list<token>::iterator iter_begin() { return b; }
		std::list<token>::iterator iter_end() { return e; }
		std::list<token>::iterator iter_actual() { return a; }
		token *begin() { return &*b; }
		token *end() { return &*e; }
		token *actual() { return &*a; }
		//token *operator -> () { return &*a; }
		bool valid() const { return a!=e; }
		void next() {
			++a;
		}
		const std::string &token_text() const;
		tokentype token_type() const;
		unsigned int token_location();
		bool is_keyword() const;
		bool is_keyword(const std::string &k) const;
		bool is_string() const;
		bool is_int() const;
		bool is_real() const;
		bool is_ident() const;
		bool is_ident(const std::string &k) const;
		bool is_oper() const;
		bool is_oper(const std::string &o) const;
		bool is_eo_l() const;
		bool is_up() const;
		bool is_down() const;
		void assert_ident(const std::string &k);
		void assert_oper(const std::string &k);
		void assert_keyword(const std::string &k);
		void assert_eo_l();
		void assert_up();
	};
} }
#endif
