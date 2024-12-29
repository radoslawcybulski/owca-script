#ifndef _RC_Y_H
#define _RC_Y_H

#ifndef OWCA_SCRIPT_DLLEXPORT

    #if defined(_MSC_VER)
        //  Microsoft 
        #define OWCA_SCRIPT_EXPORT __declspec(dllexport)
        #define OWCA_SCRIPT_IMPORT __declspec(dllimport)
    #else
        //  GCC
        #define OWCA_SCRIPT_EXPORT __attribute__((visibility("default")))
        #define OWCA_SCRIPT_IMPORT

    #endif

    #ifdef OWCA_SCRIPT_BUILDING
        #define OWCA_SCRIPT_DLLEXPORT OWCA_SCRIPT_EXPORT
    #else
        #define OWCA_SCRIPT_DLLEXPORT OWCA_SCRIPT_IMPORT
    #endif

#endif

#include "owca/debug_assert.h"
#include "owca/debug_execution.h"
#include "owca/debug_gc.h"
#include "owca/debug_gc_test.h"
#include "owca/debug_line_numbers.h"
#include "owca/debug_memory_blocks.h"
#include "owca/debug_opcodes.h"
#include "owca/debug_strings.h"

#include "owca/returnvalue.h"
#include "owca/base.h"
#include "owca/class.h"
#include "owca/global.h"
#include "owca/parameters.h"
#include "owca/message.h"
#include "owca/namespace.h"
#include "owca/sourcefile.h"
#include "owca/exception.h"
#include "owca/vm.h"
#include "owca/string.h"
#include "owca/list.h"
#include "owca/tuple.h"
#include "owca/map.h"
#include "owca/location.h"

#endif
