#ifndef _RC_Y_DEFVAL_H
#define _RC_Y_DEFVAL_H

namespace owca {
	class owca_global;
	namespace __owca__ {
		struct defval;
		class exec_variable;
		class owca_internal_string_nongc;
	}
}

namespace owca {
	namespace __owca__ {
		struct defval {
			enum type_ {
				INT,REAL,STRING,NULL_,NONE,BOOL,NO_PARAM_GIVEN
			} type;
			union {
				owca_int i;
				owca_real r;
				bool b;
				owca_internal_string_nongc *s;
			} data;
			defval(type_ t) : type(t) { }
			defval(int ii) : type(INT) { data.i=ii; }
			defval(long int ii) : type(INT) { data.i=ii; }
			defval(long long int ii) : type(INT) { data.i=ii; }
			defval(double rr) : type(REAL) { data.r=rr; }
			defval(long double rr) : type(REAL) { data.r=rr; }
			defval(bool b) : type(BOOL) { data.b=b; }
			DLLEXPORT defval(char *c);
			defval() : type(NONE) { }
			DLLEXPORT ~defval();
			DLLEXPORT void get(exec_variable &r) const;
		};
		extern defval dv_null,dv_unused,dv_no_param_given;
	}
}
#endif
