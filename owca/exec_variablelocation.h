#ifndef _RC_Y_EXEC_VARIABLELOCATION_H
#define _RC_Y_EXEC_VARIABLELOCATION_H

namespace owca {
	namespace __owca__ {

		class exec_variable_location {
			unsigned short depth_;
			unsigned short offset_;
		protected:
			bool boolean_test() const { return valid(); }
		public:
			enum { _INVALID=(1<<(sizeof(unsigned short)*8))-1 };

			exec_variable_location(unsigned int offset__, unsigned int depth__) : offset_(offset__),depth_(depth__) { }
			exec_variable_location() : depth_(_INVALID),offset_(_INVALID) { }
			unsigned int offset() const { return offset_; }
			unsigned int depth() const { return depth_; }
			bool valid() const { return depth_!=_INVALID; }
			bool operator == (const exec_variable_location &l) const { return offset_==l.offset_ && depth_==l.depth_; }

			static exec_variable_location invalid;
		};

	}
}

#endif
