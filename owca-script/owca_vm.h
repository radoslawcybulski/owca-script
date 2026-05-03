#ifndef RC_OWCA_SCRIPT_OWCA_VM_H
#define RC_OWCA_SCRIPT_OWCA_VM_H

#include "stdafx.h"
#include "owca_error_message.h"
#include "tokens.h"

namespace OwcaScript {
	class OwcaVM;
	class OwcaCode;
	class OwcaMap;
	class OwcaSet;
	class OwcaArray;
	class OwcaString;
	class OwcaTuple;
	class OwcaValue;
	class OwcaVariable;
	class NativeClassInterface;
	class Generator;

	namespace Internal {
		class VM;
	}
	
	class GenerationGC {
		unsigned int value;

	public:
		explicit GenerationGC(unsigned int value) : value(value) {}

		bool operator == (GenerationGC other) const { return value == other.value; }
		bool operator != (GenerationGC other) const { return !(*this == other); }
	};

	struct NativeCodeProvider {
		virtual ~NativeCodeProvider() = default;

		using Function = std::function<OwcaValue(OwcaVM, std::span<OwcaValue>)>;
		using GeneratorFunction = std::function<Generator(OwcaVM, std::span<OwcaValue>)>;
		virtual std::optional<Function> native_function(std::string_view full_name, std::optional<ClassToken> cls, FunctionToken token, std::span<const std::string_view> param_names) const { return std::nullopt; }
		virtual std::optional<GeneratorFunction> native_generator(std::string_view full_name, std::optional<ClassToken> cls, FunctionToken token, std::span<const std::string_view> param_names) const { return std::nullopt; }
		virtual std::shared_ptr<NativeClassInterface> native_class(std::string_view full_name, ClassToken token) const { return nullptr; }
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

		OwcaCode compile(std::string filename, std::string content, std::shared_ptr<NativeCodeProvider> native_code_provider, size_t first_line);
		OwcaCode compile(std::string filename, std::string content, std::shared_ptr<NativeCodeProvider> native_code_provider = nullptr);
		OwcaCode compile(std::string filename, std::string content, std::vector<std::string> additional_variables, std::shared_ptr<NativeCodeProvider> native_code_provider, size_t first_line = 1);
		OwcaCode compile(std::string filename, std::string content, std::vector<std::string> additional_variables, size_t first_line = 1);
		OwcaValue execute(const OwcaCode&);
		OwcaValue execute(const OwcaCode&, OwcaMap values);
		OwcaValue execute(const OwcaCode&, OwcaMap values, OwcaMap *output_dict);
		OwcaValue get_member(OwcaValue self, std::string_view key);
		void set_member(OwcaValue self, std::string_view key, OwcaValue value);
		OwcaValue call(OwcaValue func, std::span<OwcaValue> values);
		OwcaArray create_array() const;
		OwcaArray create_array(std::span<OwcaValue> values) const;
		OwcaArray create_array(std::deque<OwcaValue> values) const;
		OwcaTuple create_tuple(std::vector<OwcaValue> values) const;
		OwcaTuple create_tuple(std::pair<OwcaValue, OwcaValue> values) const;
		OwcaMap create_map() const;
		OwcaMap create_map(const std::span<OwcaValue> &values) const;
		OwcaMap create_map(const std::span<std::pair<OwcaValue, OwcaValue>> &values) const;
		OwcaMap create_map(const std::span<std::pair<std::string, OwcaValue>> &values) const;
		OwcaSet create_set(const std::span<OwcaValue> &values) const;
		OwcaString create_string(std::string_view) const;
		
		void run_gc();
	};
}

#endif
