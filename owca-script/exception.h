#ifndef RC_OWCA_SCRIPT_EXCEPTION_H
#define RC_OWCA_SCRIPT_EXCEPTION_H

#include "stdafx.h"
#include "allocation_base.h"
#include "owca_exception.h"

namespace OwcaScript {
	class OwcaVM;

	namespace Internal {
        class CodeBuffer;

		struct Exception  {
			struct Frame {
				std::shared_ptr<CodeBuffer> code;
				unsigned int line;
				std::string_view function;
			};
			std::vector<Frame> frames;
            std::string message;
			std::optional<OwcaException> parent_exception;

			std::string_view type() const {
				return "Exception";
			}
			std::string to_string() const;
			
			
			friend void gc_mark_value(OwcaVM vm, GenerationGC gc, const Exception &);
		};
	}
}

#endif
