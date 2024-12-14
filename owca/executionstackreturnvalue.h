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
			};
		private:
			type_ tp;
		public:
			executionstackreturnvalue(type_ tp_) : tp(tp_) { }

			type_ type() const { return tp; }
		};

		inline std::string to_string(executionstackreturnvalue e) {
			switch (e.type()) {
			case executionstackreturnvalue::OK: return "OK";
			case executionstackreturnvalue::EXCEPTION: return "EXCEPTION";
			case executionstackreturnvalue::RETURN: return "RETURN";
			case executionstackreturnvalue::RETURN_NO_VALUE: return "RETURN_NO_VALUE";
			case executionstackreturnvalue::FUNCTION_CALL: return "FUNCTION_CALL";
			case executionstackreturnvalue::CREATE_GENERATOR: return "CREATE_GENERATOR";
			case executionstackreturnvalue::CO_START: return "CO_START";
			case executionstackreturnvalue::CO_STOP: return "CO_STOP";
			}
			return "<unknown value executionstackreturnvalue>";
		}
	}
}

#endif
