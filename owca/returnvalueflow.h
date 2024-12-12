#ifndef _RC_Y_RETURNVALUEFLOW_H
#define _RC_Y_RETURNVALUEFLOW_H

namespace owca {
	namespace __owca__ {

		class returnvalueflow {
		public:
			enum mode_ {
				LP_BREAK,LP_CONTINUE,LP_RESTART,LP_FINALLY
			};
			enum type_ {
				FIN=0,CONTINUE_OPCODES,LOOP_CONTROL,EXCEPTION,RETURN,RETURN_NO_VALUE,YIELD,CALL_FUNCTION_CONTINUE_OPCODES,CALL_FUNCTION_CONTINUE_FIN/*,CALL_FUNCTION_EXCEPTION not very useful*/
			};
		private:
			unsigned short dt;
			unsigned char tp,md;
		public:
			returnvalueflow(type_ tp_) : tp(tp_) { }
			returnvalueflow(type_ tp_, mode_ md_, unsigned short dt_) : tp(tp_),md(md_),dt(dt_) { }
			type_ type() const { return (type_)tp; }
			mode_ mode() const { return (mode_)md; }
			unsigned short data() const { return dt; }
		};

	}
}

#endif
