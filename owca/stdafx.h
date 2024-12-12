#ifndef _RC_Y_STDAFX_H
#define _RC_Y_STDAFX_H
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef DLLEXPORT
#define DLLEXPORT
#endif

#define _CRT_SECURE_NO_DEPRECATE

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

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
#include <stdarg.h>
#include <stdint.h>

#include "debug_assert.h"
#include "debug_execution.h"
#include "debug_gc.h"
#include "debug_gc_test.h"
#include "debug_line_numbers.h"
#include "debug_memory_blocks.h"
#include "debug_opcodes.h"
#include "debug_strings.h"

#undef VOID

namespace owca {
    namespace __owca__ {
#ifdef RCDEBUG
        DLLEXPORT void debug_printf(char *, ...);
#else
        inline void debug_printf(char *, ...) { }
#endif
    }
}

#define RCPRINTF owca::__owca__::debug_printf

#endif
