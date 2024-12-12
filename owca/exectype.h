#ifndef _RC_Y_EXEC_TYPE_H
#define _RC_Y_EXEC_TYPE_H

namespace owca { namespace __owca__ {

	enum exectype {
		VAR_NULL,
		VAR_INT,
		VAR_REAL,
		VAR_BOOL,
		VAR_STRING,
		VAR_GENERATOR,
		VAR_PROPERTY,
		VAR_FUNCTION,
		VAR_FUNCTION_FAST,
		VAR_OBJECT,
		VAR_NAMESPACE,
		VAR_WEAK_REF,
		VAR_COUNT,
		VAR_HASH_DELETED,
		VAR_HASH_EMPTY,
		VAR_NO_DEF_VALUE,
		VAR_NO_PARAM_GIVEN,
		VAR_UNDEFINED
	};  

}}

#endif

