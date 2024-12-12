#ifndef _RC_Y_EXECUTION_STACK_RETURN_VALUE_H
#define _RC_Y_EXECUTION_STACK_RETURN_VALUE_H

namespace owca {
	namespace __owca__ {

		class executionstackreturnvalue {
		public:
			enum type_ {
				OK,
				EXCEPTION,
				RETURN,
				RETURN_NO_VALUE,
				FUNCTION_CALL,
				CREATE_GENERATOR,
				CO_START,
				CO_STOP,
				DEBUG_BREAK,
			};
		private:
			type_ tp;
		public:
			executionstackreturnvalue(type_ tp_) : tp(tp_) { }

			type_ type() const { return tp; }
		};

	}
}

#endif
