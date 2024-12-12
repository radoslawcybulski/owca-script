#include "stdafx.h"
#include "base.h"
#include "cmp_compiler.h"
#include "sourcefile.h"
#include "message.h"

namespace owca { namespace __owca__ {
	static std::string keywords_[]={
		"if","elif","else","finally",
		"try","except","raise","else","finally",
		"for","while","else","finally",
		"with",
		"pass",
		"return","yield",
		"break","continue","restart","finally",
		"function","class",
		"self","true","false","null",
		"lambda"
	};
	std::set<std::string> keywords;
	struct _init {
		_init() {
		for(unsigned int i=0;i<sizeof(keywords_)/sizeof(keywords_[0]);++i) {
			keywords.insert(keywords_[i]);
		}
		}
	} init_;
	static bool is_white(char t) { return t==' ' || t=='\t' || t=='\r' || t=='\f'; }
	static bool is_indent(char t) { return t==' ' || t=='\t'; }
	static bool is_alpha(char t) { return (t>='a' && t<='z') || (t>='A' && t<='Z') || t=='_'; }
	static bool is_digit(char t) { return t>='0' && t<='9'; }
	static bool is_alphanum(char t) { return is_alpha(t) || is_digit(t); }
	static bool is_digithex(char t) { return is_digit(t) || (t>='a' && t<='f') || (t>='A' && t<='F'); }
	static bool is_digitoct(char t) { return t>='0' && t<='7'; }
	static bool is_digitbin(char t) { return t>='0' && t<='1'; }

	unsigned int is_operator(const char *act, const char *end)
	{
	#define Q(a) (act[0]==a[0] && act[1]==a[1] && act[2]==a[2])
		if (act+2<end && (Q("<<=") || Q(">>="))) return 3;
	#undef Q
	#define Q(a) (act[0]==a[0] && act[1]==a[1])
		if (act+1<end && (Q("==") || Q(">=") || Q("<=") || Q("!=") || Q(">>") || Q("<<") || Q("+=") || Q("-=") || Q("*=") || Q("/=") || Q("%=") || Q("&=") || Q("|=") || Q("^=") || Q(":="))) return 2;
	#undef Q
	#define Q(a) (*act==*a)
		if (Q("=") || Q("+") || Q("-") || Q("*") || Q("/") || Q("<") || Q(">") || Q("|") || Q("&") || Q("^") || Q("~") ||
				Q("{") || Q("}") || Q("(") || Q(")") || Q("[") || Q("]") || Q(":") || Q("%") || Q(".") || Q(",") || Q("?")) return 1;
	#undef Q
		return 0;
	}

	void compiler::add_token(tokentype type, const std::string &text, unsigned int line)
	{
		tokens.push_back(token(type,text,line));
	}

    struct bracket_info {
        int line;
        char orig;
        char match;

        bracket_info() { }
        bracket_info(int line, char orig) : line(line),orig(orig) {
            if (orig == '(') match = ')';
            else if (orig == '[') match = ']';
            else if (orig == '{') match = '}';
            else RCASSERT(0);
        }
    };
	RCLMFUNCTION void compiler::tokenize(const owca_source_file &fs)
	{
		unsigned int line=0;
		int len=0;
		const char *begin,*act,*end;
		char indent_char=0;
		std::list<int> indents;
		std::list<bracket_info> brackets;

		add_token(TOKEN_UP,"",0);
		indents.push_back(0);
		while((len=fs.next_line(begin))>=0) {
			++line;
			act=begin;
			while(act<begin+len && (is_white(*act) || *act=='#')) ++act;
			if (*act=='#' || act==begin+len) continue;
			act=begin;
			//end=begin+len;

			end=begin;
			while(end<begin+len && *end!='#') ++end;

			if (brackets.empty()) {
				int ind=0;
				while(act<end && is_indent(*act)) {
					if (indent_char) {
						if (indent_char!=*act) { error(owca::YERROR_MIXED_CHARS_IN_INDENT,line,""); return; }
					}
					else indent_char=*act;
					++act;
					++ind;
				}
				if (act==end) continue;

				RCASSERT(!is_indent(*act));

				if (ind>indents.back()) {
					add_token(TOKEN_UP,"",line);
					indents.push_back(ind);
				}
				else if (ind<indents.back()) {
					while(ind<indents.back()) {
						add_token(TOKEN_DOWN,"",line);
						indents.pop_back();
					}
					if (ind!=indents.back()) { error(owca::YERROR_INCORRECT_INDENT,line,""); return; }
				}

				if (is_white(*act)) {
					unsigned int i;
					for(i=0;act[i];++i) {
						if (!is_white(act[i])) break;
					}
					if (act[i]) {
						error(owca::YERROR_UNEXPECTED_CHARACTER,line,std::string(&act[i],1));
					}
					continue;
				}
			}
			else {
				while(act<end && is_white(*act)) ++act;
			}

			while(act<end) {
				const char *b=act;

				if (is_alpha(*act) || *act=='$') { // ident
					bool keyw=(*act!='$');
					++act;
					while(act<end && is_alphanum(*act)) ++act;

					add_token(TOKEN_IDENT,std::string(b,act-b),line);
					if (keyw && keywords.find(tokens.back().text())!=keywords.end()) {
						tokens.back()._type=TOKEN_KEYWORD;
					}
				}
				else if (*act=='\'' || *act=='\"') {
					++act;
					while (act < end) {
						if (*act == *b) break;
						if (*act == '\\' && act[1] != 0) ++act;
						++act;
					}
					if (act==end) { error(owca::YERROR_UNFINISHED_STRING_CONSTANT,line,""); return; }
					++b;
					add_token(TOKEN_STRING,std::string(b,(unsigned int)(act-b)),line);
					++act;
				}
				else if (is_digit(*act) || (*act=='.' && act+1<end && is_digit(act[1]))) {
					if (*act=='0' && act+1<end && (act[1]=='x' || act[1]=='X')) {
						act+=2;
						while(act<end && is_digithex(*act)) ++act;
						if ((act-b)==2) error(owca::YERROR_WRONG_FORMAT_OF_NUMBER,line,std::string(b,2));
						add_token(TOKEN_INT,std::string(b,(unsigned int)(act-b)),line);
					}
					else if (*act=='0' && act+1<end && (act[1]=='o' || act[1]=='O')) {
						act+=2;
						while(act<end && is_digitoct(*act)) ++act;
						if ((act-b)==2) error(owca::YERROR_WRONG_FORMAT_OF_NUMBER,line,std::string(b,2));
						add_token(TOKEN_INT,std::string(b,(unsigned int)(act-b)),line);
					}
					else if (*act=='0' && act+1<end && (act[1]=='b' || act[1]=='B')) {
						act+=2;
						while(act<end && is_digitbin(*act)) ++act;
						if ((act-b)==2) error(owca::YERROR_WRONG_FORMAT_OF_NUMBER,line,std::string(b,2));
						add_token(TOKEN_INT,std::string(b,(unsigned int)(act-b)),line);
					}
					else {
						bool real=false;

						while(act<end && is_digit(*act)) ++act;
						if (act<end && *act=='.') {
							real=true;
							++act;
							while(act<end && is_digit(*act)) ++act;
						}
						if (act<end && (*act=='e' || *act=='E')) {
							real=true;
							++act;
							if (act<end && (*act=='+' || *act=='-')) ++act;
							if (act==end || !is_digit(*act)) error(owca::YERROR_WRONG_FORMAT_OF_NUMBER,line,std::string(b,(unsigned int)(act-b)));
							else while(act<end && is_digit(*act)) ++act;
						}
						add_token(real ? TOKEN_REAL : TOKEN_INT,std::string(b,(unsigned int)(act-b)),line);
					}
					if (act<end && is_alphanum(*act)) error(owca::YERROR_WRONG_FORMAT_OF_NUMBER,line,std::string(b,(unsigned int)(act+1-b)));
				}
				else if (is_white(*act)) {
					while(act<end && is_white(*act)) ++act;
				}
				else {
					unsigned int cc=is_operator(act,end);
					if (cc==0) {
						error(owca::YERROR_UNEXPECTED_CHARACTER,line,std::string(act,1));
						return;
					}
					if (cc==1) {
						switch(*act) {
						case '(':
						case '[':
                        case '{': brackets.push_back(bracket_info(line,*act)); break;
						case ')':
						case ']':
						case '}':
							if (brackets.empty()) break;
                            if (brackets.back().match!=*act) {
                                error(owca::YERROR_UNMATCHED_BRACKET,brackets.back().line,std::string(&brackets.back().orig,1));
								return;
							}
							brackets.pop_back();
							break;
						}
					}
					add_token(TOKEN_OPER,std::string(act,cc),line);
					act+=cc;
				}
			}
			if (brackets.empty()) add_token(TOKEN_EOL,"",line);
		}
		if (!brackets.empty()) add_token(TOKEN_EOL,"",line);

		while(!indents.empty()) {
			add_token(TOKEN_DOWN,"",line+1);
			indents.pop_back();
		}
	}
} }












