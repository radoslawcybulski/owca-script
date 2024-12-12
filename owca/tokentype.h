#ifndef _RC_Y_TOKEN_TYPE_H
#define _RC_Y_TOKEN_TYPE_H

namespace owca {
	namespace __owca__ {
		enum tokentype {
			TOKEN_STRING,
			TOKEN_INT,
			TOKEN_REAL,
			TOKEN_IDENT,
			TOKEN_KEYWORD,
			TOKEN_OPER,
			TOKEN_EOL,
			TOKEN_UP,
			TOKEN_DOWN
		};
	}
}

#endif