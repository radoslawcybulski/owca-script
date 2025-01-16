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
	}

	class OwcaVM {
		friend class Internal::VM;

		std::shared_ptr<Internal::VM> vm;

		OwcaVM(std::shared_ptr<Internal::VM> vm) : vm(std::move(vm)) {}
	public:
		OwcaVM();
		~OwcaVM();

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
		struct NativeCodeProvider {
			virtual ~NativeCodeProvider() = default;

			using Function = std::function<OwcaValue(OwcaVM &, std::span<OwcaValue>)>;
			virtual std::optional<Function> native_function(std::string_view name, std::span<const std::string> param_names) const { return std::nullopt; }
		};

		OwcaCode compile(std::string filename, std::string content, const NativeCodeProvider& native_code_provider = NativeCodeProvider{});
		OwcaValue execute(const OwcaCode&, const std::unordered_map<std::string, OwcaValue>& values = {});
		void run_gc();
	};
}

#endif
