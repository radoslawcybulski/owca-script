#include <stdio.h>

#define RCDEBUG_IN_VS
#define RCDEBUG

#ifndef RCDEBUG_ASSERT
#ifdef RCDEBUG

	#define RCDEBUG_ASSERT

#else

	//#define RCDEBUG_ASSERT

#endif
#endif

#ifdef RCDEBUG_ASSERT
	#ifndef RCASSERT
		inline void __rcassert(const char *file, unsigned int line)
		{
			printf("ASSERTION FAILED in %s:%d\n",file,line);
			throw 1.0f;
		}

		#define RCASSERT(a) do { if (!(a)) { __rcassert(__FILE__,__LINE__); } } while(0)
	#endif
#else
	#ifndef RCASSERT
		#define RCASSERT(a) do { if (!(a)) { } } while(0)
	#endif
#endif
