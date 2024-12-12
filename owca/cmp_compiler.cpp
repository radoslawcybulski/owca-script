#include "stdafx.h"
#include "base.h"
#include "cmp_compiler.h"
#include "virtualmachine.h"
#include "cmp_build_flow.h"
#include "tree_function.h"
#include "exec_function_ptr.h"
#include "exec_string.h"
#include "exec_stack.h"
#include "tree_varspace.h"
#include "op_base.h"
#include "op_function.h"
#include "message.h"
#include "op_write.h"
#include <climits>

namespace owca {
	namespace __owca__ {

		compiler::compiler(virtual_machine &vm_, owca_message_list &errorswarnings_) : vm(vm_),errorswarnings(errorswarnings_),actual_scope(NULL),fncowner(NULL)
		{
		}

		//owca_internal_string *compiler::identificator(const std::string &ident)
		//{
		//	return vm.identificator(ident);
		//}

		void compiler::error(const error_information &e)
		{
            error(e.mt,e.line,e.param);
		}

		void compiler::error(owca_message_type yt, unsigned int line, std::string param)
		{
			errorswarnings.msg.push_back(owca_message(yt,line,param));
			errorswarnings.errors=true;
		}

		RCLMFUNCTION std::vector<unsigned char> compiler::compile(const owca_source_file &fs, const compile_visible_items &visible_names)
		{
			tokens.clear();
			tokenize(fs);
			if (errorswarnings.errors)
				return std::vector<unsigned char>();

			tree_function *tree=build_tree(this,visible_names);

			if (errorswarnings.errors) {
done:
				delete tree;
				return std::vector<unsigned char>();
			}

			RCASSERT(actual_scope==tree->scope);
			tree->compile_names(tree_expression::AT_NONE);
			if (errorswarnings.errors) goto done;

			RCASSERT(tree->scope->next==NULL);

			RCASSERT(!errorswarnings.errors);

			opcode_writer ow;
			std::vector<unsigned char> tmp;
			try {
				tmp = ow.compile(tree);
			}
			catch (error_information &e) {
				RCASSERT(0);
				error(e);
			}

			if (errorswarnings.errors) goto done;

			delete tree;

			return tmp;
		}

		owca_int compiler::get_int(unsigned int line, const std::string &s)
		{
			owca_int v;
			int base=-1;

			if (s.size()>2 && s[0]=='0') {
				switch(s[1]) {
				case 'x':
				case 'X': base=16; break;
				case 'o':
				case 'O': base=8; break;
				case 'b':
				case 'B': base=2; break;
				}
			}
			if (!to_int(v,s.c_str()+(base>=0 ? 2 : 0),(unsigned int)s.size()-(base>=0 ? 2 : 0),base>=0 ? base : 10)) {
				throw error_information(owca::YERROR_WRONG_FORMAT_OF_NUMBER,line,s);
			}

			return v;
		}

		owca_real compiler::get_real(unsigned int line, const std::string &s)
		{
			owca_real v;
			if (!to_real(v,s.c_str(),(unsigned int)s.size())) throw error_information(owca::YERROR_WRONG_FORMAT_OF_NUMBER,line,s);
			return v;
		}

		static unsigned int hex_value(char c)
		{
			if (c>='0' && c<='9') return c-'0';
			if (c>='a' && c<='f') return c-'a'+10;
			if (c>='A' && c<='F') return c-'A'+10;
			RCASSERT(0);
			return 0;
		}

		static bool is_hex_digit(char c)
		{
			return (c>='0' && c<='9') || (c>='a' && c<='f') || (c>='A' && c<='F');
		}

		static bool is_oct_digit(char c)
		{
			return (c>='0' && c<='7');
		}

		bool compiler::parse_string(std::string &res, unsigned int line, const std::string &s)
		{
			std::ostringstream o;
			unsigned int src = 0;
			unsigned char expected = 0;

			while(src<s.size()) {
				if (s[src]=='\\' && src+1<s.size()) {
					++src;
					switch(s[src]) {
					case 'n': o << '\n'; break;
					case 'r': o << '\r'; break;
					case 'b': o << '\b'; break;
					case 'v': o << '\v'; break;
					case 'a': o << '\a'; break;
					case 'f': o << '\f'; break;
					case 't': o << '\t'; break;
					case '\\': o << '\\'; break;
					case '"': o << '\"'; break;
					case '\'': o << '\''; break;
					case 'x': expected = 1; goto parse_hex;
					case 'u': expected = 2; goto parse_hex;
					case 'U': expected = 4; goto parse_hex;
parse_hex: {
						if (src + 1 + expected * 2 > s.size())
							return false;
						unsigned int w = 0;
						for (unsigned char c = 0; c < expected * 2; ++c) {
							if (!is_hex_digit(s[src + 1 + c]))
								return false;
							unsigned char q = hex_value(s[src + 1 + c]);
							w = (w << 4) | q;
						}

						// from fox-toolkit, FXString.cpp:~420
						if (w < 0x80) o << (char)w;
						else if (w < 0x800)
							o << (char)((w >> 6) | 0xc0) << (char)((w & 0x3F) | 0x80);
						else if (w < 0x10000)
							o << (char)((w >> 12) | 0xe0) << (char)(((w >> 6) & 0x3F) | 0x80) << (char)((w & 0x3F) | 0x80);
						else if (w < 0x200000)
							o << (char)((w >> 18) | 0xf0) << (char)(((w >> 12) & 0x3F) | 0x80) << (char)(((w >> 6) & 0x3F) | 0x80) << (char)((w & 0x3F) | 0x80);
						else if (w < 0x4000000)
							o << (char)((w >> 24) | 0xf8) << (char)(((w >> 18) & 0x3F) | 0x80) << (char)(((w >> 12) & 0x3F) | 0x80) << (char)(((w >> 6) & 0x3F) | 0x80) << (char)((w & 0x3F) | 0x80);
						else
							o << (char)((w >> 30) | 0xfc) << (char)(((w >> 24) & 0X3F) | 0x80) << (char)(((w >> 18) & 0X3F) | 0x80) << (char)(((w >> 12) & 0X3F) | 0x80) << (char)(((w >> 6) & 0X3F) | 0x80) << (char)((w & 0X3F) | 0x80);
						src += expected * 2;
					  break; }
					default:
						o << s[src];
					  break;
					}
					++src;
				}
				else o << s[src++];
			}
			res = o.str();
			return true;
		}

	//class std_output : public conv_output {
	//public:
	//	std::string tmp;

	//	void size(unsigned int s) { tmp.reserve(s); }
	//	void put(char c) { tmp.append(1,c); }
	//};

	//bool convert_to_string(conv_output &out, owca_real r, unsigned int min_width, unsigned int max_width, unsigned int prec, conv_flags flags)
	//{
	//	RCASSERT(min_width<=max_width);

	//	std::ostringstream o;
	//	o.width(min_width);
	//	if (prec>0) o.precision(prec);

	//	unsigned int flg=0;

	//	switch(flags&REAL_PRINTMODE_MASK) {
	//	case REAL_NORMAL: flg|=std::ios_base::fixed; break;
	//	case REAL_EXP: flg|=std::ios_base::scientific; break;
	//	case REAL_SCIENT:
	//		RCASSERT(0);
	//	case REAL_SHORTEST:
	//		RCASSERT(0);
	//	}
	//	if (flags&REAL_FORCE_DECIMAL_POINT) flg|=std::ios_base::showpoint;
	//	if (flags&UPPER_CASE) flg|=std::ios_base::uppercase;
	//	if (flags&ADJUST_LEFT) flg|=std::ios_base::left;
	//	else flg|=std::ios_base::right;
	//	if (flags&PAD_ZEROES) {
	//		o.fill('0');
	//		if (flags&ADJUST_LEFT) flg|=std::ios_base::showpoint;
	//	}
	//	if ((flags&SIGN_FORCE) || (flags&SIGN_SPACE)) flg|=std::ios_base::showpos;
	//	o.flags(flg);

	//	if (!(o << r)) return false;

	//	std::string s=o.str();
	//	if (flags&SIGN_SPACE) {
	//		RCASSERT(s[0]=='+' || s[0]=='-');
	//		if (s[0]=='+') s[0]=' ';
	//	}
	//	if (!(flags&PAD_ZEROES)) {
	//		while(s.length()>(min_width==0 ? 1 : min_width) && s[s.length()-1]=='0' && s[s.length()-2]>='0' && s[s.length()-2]<='9') s.resize(s.length()-1);
	//	}
	//	out.size((unsigned int)std::min(s.size(),max_width));
	//	for(unsigned int i=0;i<std::min(s.size(),max_width);++i) out.put(s[i]);
	//	return true;
	//}

	//bool convert_to_string(conv_output &out, owca_int ii, unsigned int min_width, unsigned int max_width, conv_flags flags)
	//{
	//	RCASSERT(min_width<=max_width);

	//	std::ostringstream o;
	//	o.width(min_width);

	//	if ((flags&INT_PRINTMODE_MASK)==INT_BIN) {
	//		std::string s;
	//		int beg;
	//		bool neg=false;
	//		beg=sizeof(owca_int)*8+4;

	//		if (ii<0) { neg=true; ii=-ii; }
	//
	//		s.resize(beg);
	//		do {
	//			s[--beg]=(ii&1) ? '1' : '0';
	//		} while((ii>>1)!=0);
	//		if (neg || (flags&SIGN_FORCE) || (flags&SIGN_SPACE)) {
	//			s[--beg]=neg ? '-' : '+';
	//		}
	//		if (flags&INT_FORCE_PREFIX) {
	//			s[--beg]=(flags&UPPER_CASE) ? 'B' : 'b';
	//			s[--beg]='0';
	//		}
	//		RCASSERT(beg>=0);
	//		if (flags&ADJUST_LEFT) o.flags(std::ios_base::left);
	//		else o.flags(std::ios_base::right);
	//		if (!(o << s.substr(beg,s.size()-beg))) return false;
	//	}
	//	else {
	//		unsigned int flg=0;

	//		switch(flags&INT_PRINTMODE_MASK) {
	//		case INT_HEX: flg|=std::ios_base::hex; break;
	//		case INT_DEC: flg|=std::ios_base::dec; break;
	//		case INT_OCT: flg|=std::ios_base::oct; break;
	//		}
	//		if (flags&UPPER_CASE) flg|=std::ios_base::uppercase;
	//		if (flags&ADJUST_LEFT) flg|=std::ios_base::left;
	//		if ((flags&SIGN_FORCE) || (flags&SIGN_SPACE)) flg|=std::ios_base::showpos;
	//		if (flags&INT_FORCE_PREFIX) flg|=std::ios_base::showbase;
	//		o.flags(flg);

	//		if (!(o << ii)) return false;
	//	}
	//
	//	std::string s=o.str();
	//	if (flags&SIGN_SPACE) {
	//		RCASSERT(s[0]=='+' || s[0]=='-');
	//		if (s[0]=='+') s[0]=' ';
	//	}
	//	out.size((unsigned int)std::min(s.size(),max_width));
	//	for(unsigned int i=0;i<std::min(s.size(),max_width);++i) out.put(s[i]);
	//	return true;
	//}
	}

	bool to_int(owca_int &retval, const char *txt, unsigned int size, unsigned int base)
	{
		char *end;
		long  l;
		errno = 0;
		l = strtol(txt, &end, base);
		if ((errno == ERANGE && l == LONG_MAX) || l > INT_MAX) {
			return false;
		}
		if ((errno == ERANGE && l == LONG_MIN) || l < INT_MIN) {
			return false;
		}
		while(*end != 0 && (*end == ' ' || *end == '\t')) ++end;
		if (*txt == '\0' || *end != '\0') {
			return false;
		}
		retval = l;
		return true;
		//unsigned int i=0;
		//bool used=false;

		//while(i<size && isspace(txt[i])) ++i;
		//if (i==size) return false;
		//RCASSERT(base>1 && (int)base<=10+'z'-'a');

		//retval=0;
		//while(i<size) {
		//	int diff;

		//	if (txt[i]>='0' && txt[i]<='9') diff=txt[i]-'0';
		//	else if (txt[i]>='a' && txt[i]<'a'+(int)base-10) diff=txt[i]-'a'+10;
		//	else if (txt[i]>='A' && txt[i]<'A'+(int)base-10) diff=txt[i]-'A'+10;
		//	else if (isspace(txt[i])) break;
		//	else if (txt[i]=='.') {
		//		if (base!=10) return false;
		//		bool zeroes=false;
		//		++i;
		//		while(i<size && txt[i]=='0') {
		//			++i;
		//			zeroes=true;
		//		}
		//		if (!used && !zeroes) return false;
		//		break;
		//	}
		//	else return false;

		//	owca_int q=retval*base+diff;
		//	if ((q-diff)/base!=retval) return false;
		//	retval=q;
		//	used=true;
		//	++i;
		//}
		//if (!used) return false;
		//while(i<size && isspace(txt[i])) ++i;
		//return i==size;
	}

	bool to_real(owca_real &retval, const char *txt, unsigned int size)
	{
		std::istringstream ii(std::string(txt,size));
		char c;

		ii >> retval;
		if (ii.fail()) return false;
		for(;;) {
			if (!ii.get(c)) return true;
			if (!isspace(c)) return false;
		}
	}

	// TODO: fix it
	std::string real_to_string(owca_real v)
	{
		std::ostringstream oo;

		oo << std::showpoint << v;
		std::string z=oo.str();
		for(size_t q=0;q<z.size();++q) if (z[q]=='.') {
			q+=2;
			size_t p=z.size();
			while(p>q && z[p-1]=='0') --p;
			return z.substr(0,p);
		}
		return z;
	}

	std::string int_to_string(owca_int v, unsigned int base, bool upper)
	{
		char buf[sizeof(v)*8+2],*p=buf+sizeof(v)*8+1;
		bool neg=false;
		if (v<0) {
			neg=true;
			v=-v;
		}
		*p=0;
		do {
			--p;
			unsigned int m=(unsigned int)(v%base);
			if (m>9 && upper) *p='A'+(char)(m-10);
			else if (m>9 && !upper) *p='a'+(char)(m-10);
			else *p='0'+(char)m;
			v=v/base;
		} while(v>0);
		if (neg) *--p='-';
		return p;
	}

	std::string ptr_to_string(void *ptr)
	{
		std::string ss;
		ss.resize(sizeof(void*)*2);
		unsigned long long dd=reinterpret_cast<unsigned long long>(ptr);
		for(int i=sizeof(void*)*2-1;i>=0;--i) {
			ss[i]=(dd&0xf)>=10 ? 'A'+(unsigned int)(dd&0xf)-10 : '0'+(unsigned int)(dd&0xf);
			dd>>=4;
		}
		return ss;
	}

	//std::string to_string(const owca_location &l)
	//{
	//	return l.filename()+(":"+to_string((owca_int)l.line()));
	//}
}











