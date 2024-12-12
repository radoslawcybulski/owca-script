#ifndef _RC_Y_OPERATORCODES_H
#define _RC_Y_OPERATORCODES_H

namespace owca {
	namespace __owca__ {

		enum operatorcodes {
			E_EQ,
			E_NOTEQ,
			E_LESSEQ,
			E_MOREEQ,
			E_LESS,
			E_MORE,
			E_EQ_R,
			E_NOTEQ_R,
			E_LESSEQ_R,
			E_MOREEQ_R,
			E_LESS_R,
			E_MORE_R,
			E_ADD,
			E_SUB,
			E_MUL,
			E_DIV,
			E_MOD,
			E_LSHIFT,
			E_RSHIFT,
			E_BIN_AND,
			E_BIN_OR,
			E_BIN_XOR,
			E_ADD_R,
			E_SUB_R,
			E_MUL_R,
			E_DIV_R,
			E_MOD_R,
			E_LSHIFT_R,
			E_RSHIFT_R,
			E_BIN_AND_R,
			E_BIN_OR_R,
			E_BIN_XOR_R,
			E_IN,
			E_ADD_SELF,
			E_SUB_SELF,
			E_MUL_SELF,
			E_DIV_SELF,
			E_MOD_SELF,
			E_LSHIFT_SELF,
			E_RSHIFT_SELF,
			E_BIN_AND_SELF,
			E_BIN_OR_SELF,
			E_BIN_XOR_SELF,
			E_ACCESS_1_READ,
			E_ACCESS_1_WRITE,
			E_ACCESS_2_READ,
			E_ACCESS_2_WRITE,
			E_BIN_NOT,
			E_BOOL,
			E_SIGN_CHANGE,
			E_CALL,
			E_GENERATOR,
			E_INIT,
			E_NEW,
			E_STR,
			E_HASH,
			E_WITH_EXIT,
			E_WITH_ENTER,
			E_COUNT,
		};

		const char *operatorcodes_name(operatorcodes oper);
	}
}

#endif
