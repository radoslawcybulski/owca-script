#ifndef _RC_Y_MESSAGE_H
#define _RC_Y_MESSAGE_H

#include "location.h"
#include "owca_message_type.h"

namespace owca {
	class owca_location;
	class owca_message;
	class owca_message_list;
	namespace __owca__ {
		class compiler;
	}
}

namespace owca {
	DLLEXPORT const char *owca_message_type_text(owca_message_type mt);

	class owca_message {
		owca_message_type mt;
		unsigned int line_;
        std::string param_;
	public:
		owca_message(owca_message_type mt_, unsigned int line__, std::string param_) : mt(mt_),line_(line__),param_(param_) {
			int v;
			v=0;
		}

		unsigned int line() const { return line_; }
		owca_message_type type() const { return mt; }
        const std::string &param() const { return param_; }
		DLLEXPORT std::string text() const;
		bool is_error() const { return mt>YMESSAGE_ERROR_BEGIN && mt<YMESSAGE_ERROR_END; }
		bool is_warning() const { return mt>YMESSAGE_WARNING_BEGIN && mt<YMESSAGE_WARNING_END; }
	};
	class owca_message_list {
		friend class __owca__::compiler;
		std::list<owca_message> msg;
		bool errors,warnings;
	public:
		typedef std::list<owca_message>::const_iterator T;
		owca_message_list() : errors(false),warnings(false) { }
		bool has_errors() const { return errors; }
		bool has_warnings() const { return warnings; }
		T begin() const { return msg.begin(); }
		T end() const { return msg.end(); }
		const std::list<owca_message> &messages() const { return msg; }
	};
}
#endif
