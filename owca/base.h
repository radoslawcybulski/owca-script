#ifndef _RC_Y_BASE_H
#define _RC_Y_BASE_H

#include <string>
#include <iostream>
#include <sstream>
#include <string>
#include <ios>
#include <new>
#include <list>
#include <map>
#include <string>
#include <vector>
#include <set>
#include <limits>
#include <typeinfo>
#include <algorithm>
#include <stdint.h>
#include <cstdio>

#ifdef RCDEBUG_TIME_IT
namespace owca {
	namespace __owca__ {
		class timeit;
		class timer {
			friend class timeit;
			std::string ident;
			unsigned __int64 value;
			unsigned int count;
			timer *next;
		public:
			timer(const std::string &ident_);
		static void print_timers();
		};
		class timeit {
			timer &t;
		public:
			timeit(timer &);
			~timeit();
		};
	}
}
#define TIMEITQ(a) a ## __LINE__
#define TIMER(name,ident) static owca::__owca__::timer name(ident)
#define TIMEIT(timer) owca::__owca__::timeit TIMEITQ(__L__) (timer)
#else
#define TIMER(name,ident) do { } while(0)
#define TIMEIT(timer) do { } while(0)
#endif

void debugprint(const char *, ...);

#ifdef RCDEBUG
namespace owca { namespace __owca__ {
	class exec_variable;

	void _rc_linemarker_mark(unsigned int);
	void _rc_linemarker_print();
} }

#endif


#ifdef RCDEBUG_LINE_NUMBERS
	#define RCLM1(q) do { _rc_linemarker_mark(q); } while(0)
#else
	#define RCLM1(q) do { } while(0)
#endif

namespace owca {
	typedef int64_t owca_int;
	typedef uint64_t owca_int_unsigned;
	typedef double owca_real;

	class owca_string;
	class owca_local;
	class owca_location;
	class owca_parameters;

	namespace __owca__ {
		class virtual_machine;
		class exec_variable;
		class owca_internal_string;
		struct callparams;
	}

	DLLEXPORT std::string ptr_to_string(void *ptr);
	DLLEXPORT std::string int_to_string(owca_int l, unsigned int base=10, bool upper=false);
	DLLEXPORT std::string real_to_string(owca_real l);
	DLLEXPORT bool to_int(owca_int &retval, const char *txt, unsigned int size, unsigned int base);
	DLLEXPORT bool to_real(owca_real &retval, const char *txt, unsigned int size);
}

namespace owca {
	namespace __owca__ {
		DLLEXPORT std::string __exception_format(const char *msg);
		DLLEXPORT std::string __exception_format1(const char *msg, const std::string &p1);
		DLLEXPORT std::string __exception_format2(const char *msg, const std::string &p1, const std::string &p2);

		void debug_check_memory(void);
	}
}

#define RCLMFUNCTION
#define RCLMBLOCK
#define OWCA_ERROR_FORMAT owca::__owca__::__exception_format
#define OWCA_ERROR_FORMAT1 owca::__owca__::__exception_format1
#define OWCA_ERROR_FORMAT2 owca::__owca__::__exception_format2

#endif
