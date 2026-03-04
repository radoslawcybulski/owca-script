#ifndef RC_OWCA_SCRIPT_OWCA_VM_H
#define RC_OWCA_SCRIPT_OWCA_VM_H

#include "stdafx.h"
#include "owca_error_message.h"
#include "tokens.h"

namespace OwcaScript {
	class OwcaVM;
	class OwcaCode;
	class OwcaValue;
	class OwcaVariable;
	class OwcaVariableSet;
	class NativeClassInterface;
	class Generator;

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

	struct NativeCodeProvider {
		virtual ~NativeCodeProvider() = default;

		using Function = std::function<OwcaValue(OwcaVM, std::span<OwcaValue>)>;
		using GeneratorFunction = std::function<Generator(OwcaVM, OwcaVariableSet &, std::span<OwcaValue>)>;
		virtual std::optional<Function> native_function(std::string_view full_name, FunctionToken token, std::span<const std::string_view> param_names) const { return std::nullopt; }
		virtual std::optional<GeneratorFunction> native_generator(std::string_view full_name, FunctionToken token, std::span<const std::string_view> param_names) const { return std::nullopt; }
		virtual std::unique_ptr<NativeClassInterface> native_class(std::string_view full_name, ClassToken token) const { return nullptr; }
	};

	class OwcaVM {
		friend class Internal::VM;
		friend class OwcaVariable;

		std::shared_ptr<Internal::VM> vm_owner;
		Internal::VM *vm;

	public:
		OwcaVM();
		OwcaVM(Internal::VM *vm) : vm(std::move(vm)) {}
		~OwcaVM();

		class SerializationFailed : public std::exception {
			std::string msg;
		public:
			SerializationFailed(std::string msg) : msg(std::move(msg)) {}
			const char *what() const noexcept {
				return msg.c_str();
			}
		};
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

		OwcaCode load(std::string filename, std::span<unsigned char> binary_content, std::unique_ptr<NativeCodeProvider> native_code_provider = nullptr);
		OwcaCode compile(std::string filename, std::string content, std::unique_ptr<NativeCodeProvider> native_code_provider = nullptr);
		OwcaCode compile(std::string filename, std::string content, std::span<const std::string> additional_variables, std::unique_ptr<NativeCodeProvider> native_code_provider = nullptr);
		OwcaValue execute(const OwcaCode&);
		OwcaValue execute(const OwcaCode&, OwcaValue values);
		OwcaValue execute(const OwcaCode&, OwcaValue values, OwcaValue *output_dict);
		OwcaValue create_array() const;
		OwcaValue create_array(std::span<OwcaValue> values) const;
		OwcaValue create_array(std::deque<OwcaValue> values) const;
		OwcaValue create_tuple(std::vector<OwcaValue> values) const;
		OwcaValue create_map() const;
		OwcaValue create_map(const std::span<OwcaValue> &values) const;
		OwcaValue create_map(const std::span<std::pair<OwcaValue, OwcaValue>> &values) const;
		OwcaValue create_map(const std::span<std::pair<std::string, OwcaValue>> &values) const;
		OwcaValue create_set(const std::span<OwcaValue> &values) const;
		OwcaValue create_string(std::string) const;
		OwcaValue create_string_from_view(std::string_view) const;
		OwcaValue create_string(OwcaValue str, size_t start, size_t end) const;
		OwcaValue create_string(OwcaValue str, size_t count) const;
		OwcaValue create_string(OwcaValue left, OwcaValue right) const;
		
		void run_gc();
		void gc_mark(OwcaValue, GenerationGC);
	};
}

#endif
