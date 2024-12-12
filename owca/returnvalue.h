#ifndef _RC_Y_RETURNVALUE_H
#define _RC_Y_RETURNVALUE_H

namespace owca {

	class returnvalue {
	public:
		enum type_ {
			EXCEPTION=0,
			VALUE,
			NO_VALUE,
		};
		type_ t;
	public:
		returnvalue(type_ tt=NO_VALUE) : t(tt) {
#ifdef RCDEBUG
			int zz;
			switch(tt) {
			case VALUE:
				zz=0;
				break;
			case NO_VALUE:
				zz=0;
				break;
			case EXCEPTION:
				zz=0;
				break;
			}
#endif
		}
		bool operator == (type_ tt) const { return t==tt; }
		bool operator != (type_ tt) const { return t!=tt; }

		type_ type() const { return t; }
		bool ok() const { return t!=EXCEPTION; }
		bool exception() const { return t==EXCEPTION; }
		bool ok_value() const { return t==VALUE; }
		bool ok_novalue() const { return t==NO_VALUE; }
	};
}
#endif
