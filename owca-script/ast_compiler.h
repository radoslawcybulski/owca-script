#ifndef RC_OWCA_SCRIPT_AST_COMPILER_H
#define RC_OWCA_SCRIPT_AST_COMPILER_H

#include "stdafx.h"
#include "owca_error_message.h"
#include "ast_base.h"
#include "line.h"
#include "owca_vm.h"

namespace OwcaScript {
	namespace Internal {
		class CodeBuffer;
		class AstFunction;

		class AstCompiler {
			std::string filename_;
			std::string content;
			std::string full_name;
			size_t content_offset = 0;
			Line content_line = Line{ 1 };
			std::vector<OwcaErrorMessage> error_messages_;
			std::vector<AstFunction*> functions_stack;
			std::unique_ptr<OwcaVM::NativeCodeProvider> native_code_provider;
			std::unordered_map<std::string_view, std::pair<Line, unsigned int>> loop_control_identifiers;
			unsigned int loop_control_depth = 0;
			VM &vm;

			bool continue_ = true;
			bool allow_range = true;
			struct AllowRangeSet {
				bool& allow_range;
				bool old_value;

				AllowRangeSet(bool& allow_range, bool new_val) : allow_range(allow_range) {
					old_value = allow_range;
					allow_range = new_val;
				}
				~AllowRangeSet() {
					allow_range = old_value;
				}
			};
			struct FunctionUpdater {
				AstCompiler &compiler;
				std::unordered_map<std::string_view, std::pair<Line, unsigned int>> loop_control_identifiers;
				unsigned int loop_control_depth;

				FunctionUpdater(AstCompiler &compiler) : compiler(compiler) {
					loop_control_identifiers = std::move(compiler.loop_control_identifiers);
					loop_control_depth = compiler.loop_control_depth;

					compiler.loop_control_identifiers.clear();
					compiler.loop_control_depth = 0;
				}
				~FunctionUpdater() {
					compiler.loop_control_identifiers = std::move(loop_control_identifiers);
					compiler.loop_control_depth = loop_control_depth;
				}
			};
			struct LoopControlUpdater {
				AstCompiler &compiler;
				std::string_view loop_ident;

				LoopControlUpdater(AstCompiler &compiler, Line line, std::string_view loop_ident) : compiler(compiler), loop_ident(loop_ident) {
					if (!loop_ident.empty()) {
						auto it = compiler.loop_control_identifiers.insert({ loop_ident, { line, compiler.loop_control_depth } });
						if (!it.second) {
							compiler.add_error(OwcaErrorKind::InvalidIdentifier, compiler.filename_, line, std::format("reused loop identifier `{}` - previous used in line {}", loop_ident, it.first->second.first));
							this->loop_ident = {};
						}
					}
					++compiler.loop_control_depth;
				}
				~LoopControlUpdater() {
					if (!loop_ident.empty()) {
						compiler.loop_control_identifiers.erase(loop_ident);
					}
					--compiler.loop_control_depth;
				}
			};
			struct FullNameUpdater {
				std::string& full_name;
				size_t size;

				FullNameUpdater(std::string& full_name, std::string_view name) : full_name(full_name) {
					size = full_name.size();
					if (!full_name.empty()) full_name += ".";
					full_name += name;
				}
				~FullNameUpdater() {
					full_name.resize(size);
				}
			};
			void add_error(OwcaErrorKind kind_, std::string file_, Line line_, std::string message_);
			[[noreturn]] void add_error_and_throw(OwcaErrorKind kind_, std::string file_, Line line_, std::string message_);

			bool eof();
			Line skip_ws();
			std::string_view preview_string();
			std::string_view preview_ident();
			std::string_view preview_number();
			std::string_view preview_oper();
			std::pair<Line, std::string_view> preview();
			std::pair<Line, std::string_view> consume();
			Line consume(std::string_view);
			bool is_keyword(std::string_view) const;
			bool is_identifier(std::string_view) const;
			bool is_operator(std::string_view) const;
			bool is_valid_number(std::string_view) const;

			std::unique_ptr<AstExpr> compile_parse_constant_number(Line line, std::string_view text);

			bool preview_is_loop_identifier();
			std::unique_ptr<AstExpr> compile_expr_value();
			std::unique_ptr<AstExpr> compile_expr_postfix();
			std::unique_ptr<AstExpr> compile_expr_prefix();
			std::unique_ptr<AstExpr> compile_expr_mul_div_mod();
			std::unique_ptr<AstExpr> compile_expr_add_sub();
			std::unique_ptr<AstExpr> compile_expr_bitwise();
			std::unique_ptr<AstExpr> compile_expr_range();
			std::unique_ptr<AstExpr> compile_expr_compare();
			std::unique_ptr<AstExpr> compile_expr_log_and();
			std::unique_ptr<AstExpr> compile_expr_log_or();
			std::unique_ptr<AstExpr> compile_expression_no_assign(bool allow_ranges_val = true);
			std::unique_ptr<AstExpr> compile_expression();
			std::unique_ptr<AstStat> compile_expression_as_stat();
			std::unique_ptr<AstStat> compile_block();
			std::unique_ptr<AstFunction> compile_function_raw();
			std::unique_ptr<AstClass> compile_class_raw();
			std::unique_ptr<AstStat> compile_function();
			std::unique_ptr<AstStat> compile_class();
			std::unique_ptr<AstStat> compile_return();
			std::unique_ptr<AstStat> compile_try();
			std::unique_ptr<AstStat> compile_throw();
			std::unique_ptr<AstStat> compile_for(std::string_view loop_ident);
			std::unique_ptr<AstStat> compile_if(bool elif = false);
			std::unique_ptr<AstStat> compile_while(std::string_view loop_ident);
			std::unique_ptr<AstStat> compile_break_or_continue();
			std::unique_ptr<AstStat> compile_stat();
			std::unique_ptr<AstFunction> compile_main_block();

			struct Phase2;
			struct RewriteAsWrite;
			void compile_phase_2(AstFunction& root, std::span<const std::string> additional_variables);
		public:
			AstCompiler(VM &vm, std::string filename_, std::string content, std::unique_ptr<OwcaVM::NativeCodeProvider> native_code_provider) : filename_(std::move(filename_)), content(std::move(content)), native_code_provider(std::move(native_code_provider)), vm(vm) {}

			const auto& filename() const { return filename_; }
			std::shared_ptr<CodeBuffer> compile(std::span<const std::string> additional_variables = {});
			auto take_error_messages() const { return std::move(error_messages_); }
		};
	}
}

#endif
