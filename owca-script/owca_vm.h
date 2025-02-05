#ifndef RC_OWCA_SCRIPT_OWCA_VM_H
#define RC_OWCA_SCRIPT_OWCA_VM_H

#include "stdafx.h"
#include "owca_error_message.h"
#include "owca_class.h"

namespace OwcaScript {
	class OwcaCode;
	class OwcaValue;
	class OwcaFunctions;

	class GenerationGC {
		unsigned int value;
	
	public:
		explicit GenerationGC(unsigned int value) : value(value) {}

		bool operator == (GenerationGC other) const { return value == other.value; }
		bool operator != (GenerationGC other) const { return !(*this == other); }
	};

	namespace Internal {
		class VM;
	}

	class OwcaVM {
		friend class Internal::VM;

		std::shared_ptr<Internal::VM> vm_owner;
		Internal::VM *vm;

	public:
		OwcaVM();
		OwcaVM(Internal::VM *vm) : vm(std::move(vm)) {}
		~OwcaVM();

		class CompilationFailed : public std::exception {
			std::string filename_;
			std::string err_msg;
			std::vector<OwcaErrorMessage> error_messages_;

		public:
			CompilationFailed(std::string filename_, std::vector<OwcaErrorMessage> error_messages_);

			const auto& error_messages() const { return error_messages_; }
			
			const char* what() const noexcept {
				return err_msg.c_str();
			}
		};
		class FunctionToken {
			const void* tok = nullptr;
		public:
			explicit FunctionToken(const void* tok) : tok(tok) {}

			auto value() const { return tok; }

			bool operator == (FunctionToken o) const { return tok == o.tok; }
			bool operator != (FunctionToken o) const { return tok != o.tok; }
		};
		class ClassToken {
			const void* tok = nullptr;
		public:
			explicit ClassToken(const void* tok) : tok(tok) {}

			auto value() const { return tok; }

			bool operator == (ClassToken o) const { return tok == o.tok; }
			bool operator != (ClassToken o) const { return tok != o.tok; }
		};
		struct NativeCodeProvider {
			virtual ~NativeCodeProvider() = default;

			using Function = std::function<OwcaValue(OwcaVM &, std::span<OwcaValue>)>;
			virtual std::optional<Function> native_function(std::string_view full_name, FunctionToken token, std::span<const std::string_view> param_names) const { return std::nullopt; }
			virtual std::unique_ptr<OwcaClass::NativeClassInterface> native_class(std::string_view full_name, ClassToken token) const { return nullptr; }
		};

		OwcaCode compile(std::string filename, std::string content, std::unique_ptr<NativeCodeProvider> native_code_provider = nullptr);
		OwcaCode compile(std::string filename, std::string content, std::span<const std::string> additional_variables, std::unique_ptr<NativeCodeProvider> native_code_provider = nullptr);
		OwcaValue execute(const OwcaCode&);
		OwcaValue execute(const OwcaCode&, const std::unordered_map<std::string, OwcaValue>& values);
		OwcaValue execute(const OwcaCode&, OwcaValue &output_dict);
		OwcaValue execute(const OwcaCode&, const std::unordered_map<std::string, OwcaValue>& values, OwcaValue &output_dict);
		OwcaValue create_array(std::vector<OwcaValue> values) const;
		OwcaValue create_tuple(std::vector<OwcaValue> values) const;
		OwcaValue create_map(const std::vector<OwcaValue> &values) const;
		OwcaValue create_set(const std::vector<OwcaValue> &values) const;
		void run_gc();
		void gc_mark(const OwcaValue&, GenerationGC);
	};
}

#endif
