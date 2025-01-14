#ifndef RC_OWCA_SCRIPT_OWCA_VM_H
#define RC_OWCA_SCRIPT_OWCA_VM_H

#include "stdafx.h"
#include "owca_error_message.h"

namespace OwcaScript {
	class OwcaCode;
	class OwcaValue;
	class OwcaFunctions;
	namespace Internal {
		class VM;
		class ImplExprDiv;
		class ImplExprMod;
		class ImplExprCompare;
		class ImplExprNegate;
		class ImplExprCall;
		class ImplExprIndex;
		class ImplExprMember;
		class ImplExprIdentifier;
		class ImplExprCreateMap;
		class ImplExprCreateSet;
		class ImplExprCreateArray;
		class ImplExprFunction;
		class ImplExprMul;
	}

	class OwcaVM {
		friend class Internal::VM;
		friend class OwcaValue;
		friend class Internal::ImplExprDiv;
		friend class Internal::ImplExprMod;
		friend class Internal::ImplExprCompare;
		friend class Internal::ImplExprNegate;
		friend class Internal::ImplExprCall;
		friend class Internal::ImplExprIndex;
		friend class Internal::ImplExprMember;
		friend class Internal::ImplExprIdentifier;
		friend class Internal::ImplExprCreateMap;
		friend class Internal::ImplExprCreateSet;
		friend class Internal::ImplExprCreateArray;
		friend class Internal::ImplExprFunction;
		friend class Internal::ImplExprMul;
		friend class OwcaFunctions;

		std::shared_ptr<Internal::VM> vm;

		OwcaVM(std::shared_ptr<Internal::VM> vm) : vm(std::move(vm)) {}
	public:
		OwcaVM();
		OwcaVM(const OwcaVM&) = delete;
		OwcaVM(OwcaVM&&);
		~OwcaVM();

		OwcaVM& operator = (const OwcaVM&) = delete;
		OwcaVM& operator = (OwcaVM&&);

		class CompilationFailed : public std::exception {
			std::string filename_;
			std::string err_msg;
			std::vector<OwcaErrorMessage> error_messages_;

		public:
			CompilationFailed(std::string filename_, std::vector<OwcaErrorMessage> error_messages_) : filename_(std::move(filename_)), error_messages_(std::move(error_messages_)) {
				err_msg = "compilation of file `" + this->filename_ + "` failed";
			}

			const auto& error_messages() const { return error_messages_; }
			
			const char* what() const noexcept {
				return err_msg.c_str();
			}
		};
		OwcaCode compile(std::string filename, std::string content);
		OwcaValue execute(const OwcaCode&, const std::unordered_map<std::string, OwcaValue>& values = {});
		void run_gc();
	};
}

#endif
