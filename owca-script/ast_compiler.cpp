#include "stdafx.h"
#include "ast_compiler.h"
#include "code_buffer.h"
#include "ast_block.h"
#include "ast_expr_as_stat.h"
#include "ast_expr_oper_2.h"
#include "ast_expr_oper_1.h"
#include "ast_expr_compare.h"
#include "ast_function.h"
#include "ast_expr_oper_x.h"
#include "ast_expr_member.h"
#include "ast_expr_constant.h"
#include "ast_expr_identifier.h"
#include "ast_return.h"
#include "ast_yield.h"
#include "ast_class.h"
#include "ast_if.h"
#include "ast_while.h"
#include "ast_for.h"
#include "ast_try.h"
#include "ast_throw.h"
#include "ast_loop_control.h"
#include "vm.h"

namespace OwcaScript::Internal {
	static std::unordered_set<std::string_view> keywords = { {
		"true", "false", "nul", "if", "else", "elif", "for", "while", "return", "function", "class", "throw", "try", "catch", "and", "or", "not", "native"
	} };
	static std::string_view operators_2[] = {
			">=", "<=", "=>", "==", "!="
	};
	static std::string_view operators_1 = {
			"+-*/%&|^='\";[](){}<>:,."
	};
	struct CompilationError {};

	static bool is_digit(char c, unsigned int base = 10) {
		if (base == 2)
			return (c >= '0' && c <= '1');
		if (base == 8)
			return (c >= '0' && c <= '7');
		if (base == 10)
			return (c >= '0' && c <= '9');
		if (base == 16)
			return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
		return false;
	}
	static bool is_alpha(char c) {
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
	}
	static bool is_alpha_or_underscore(char c) {
		return is_alpha(c) || c == '_';
	}
	static bool is_alpha_or_digit(char c) {
		return is_alpha(c) || is_digit(c);
	}
	static bool is_alpha_or_underscore_or_digit(char c) {
		return is_alpha_or_underscore(c) || is_digit(c);
	}

	bool AstCompiler::eof()
	{
		return content_offset >= content.size();
	}
	Line AstCompiler::skip_ws()
	{
		while (content_offset < content.size() && content[content_offset] <= ' ') {
			if (content[content_offset] == '\n') {
				content_line.line++;
			}
			++content_offset;
		}
		return content_line;
	}
	std::string_view AstCompiler::preview_string()
	{
		auto start = content_offset;
		auto first = content[content_offset];
		++content_offset;
		while (!eof()) {
			auto d = content[content_offset];
			if (d == '\n') {
				continue_ = false;
				add_error_and_throw(OwcaErrorKind::StringContainsEndOfLineCharacter, filename_, content_line, "string contains end of line character");
			}
			if (d == first) break;
			++content_offset;
		}
		++content_offset;
		return std::string_view{ content }.substr(start, content_offset - start);
	}
	std::string_view AstCompiler::preview_number()
	{
		auto start = content_offset;
		while (!eof() && is_alpha_or_digit(content[content_offset])) ++content_offset;
		if (!eof() && content[content_offset] == '.') {
			++content_offset;
			while (!eof() && is_alpha_or_digit(content[content_offset])) ++content_offset;
		}
		return std::string_view{ content }.substr(start, content_offset - start);
	}
	std::string_view AstCompiler::preview_ident()
	{
		auto start = content_offset;
		++content_offset;
		while (!eof() && is_alpha_or_underscore_or_digit(content[content_offset])) ++content_offset;
		return std::string_view{ content }.substr(start, content_offset - start);
	}
	std::string_view AstCompiler::preview_oper()
	{
		auto start = content_offset;
		auto c = content[content_offset];
		auto o2 = std::string_view{ content }.substr(start, 2);
		for (auto q : operators_2) {
			if (q == o2) {
				content_offset += o2.size();
				return o2;
			}
		}
		if (operators_1.find(content[content_offset]) != std::string::npos) {
			content_offset += 1;
			return std::string_view{ content }.substr(start, 1);
		}
		continue_ = false;
		if (c >= 0 && c < ' ')
			add_error_and_throw(OwcaErrorKind::SyntaxError, filename_, content_line, std::format("unexpected character 0x{:#04x}", (int)c));
		add_error_and_throw(OwcaErrorKind::SyntaxError, filename_, content_line, std::format("unexpected character `{}`", c));
	}

	std::pair<Line, std::string_view> AstCompiler::preview()
	{
		skip_ws();
		if (eof())
			add_error_and_throw(OwcaErrorKind::UnexpectedEndOfFile, filename_, content_line, "unexpected end of file");
		auto start = content_offset;
		auto c = content[content_offset];
		auto line = content_line;
		auto text = [&]() {
			if (is_alpha_or_underscore(c)) return preview_ident();
			if (is_digit(c)) return preview_number();
			if (c == '"' || c == '\'') return preview_string();
			if (c == '.' && content_offset + 1 < content.size() && content[content_offset + 1] >= '0' && content[content_offset + 1] <= '9') return preview_number();
			return preview_oper();
			}();
		content_line = line;
		content_offset = start;
		return{ line, text };
	}
	std::pair<Line, std::string_view> AstCompiler::consume()
	{
		auto [line, tok] = preview();
		content_offset += tok.size();
		skip_ws();
		return { line, tok };
	}
	Line AstCompiler::consume(std::string_view txt)
	{
		auto [line, tok] = preview();
		if (tok != txt) {
			add_error_and_throw(OwcaErrorKind::SyntaxError, filename_, line, std::format("unexpected token `{}`, expected `{}`", tok, txt));
		}
		content_offset += tok.size();
		skip_ws();
		return line;
	}
	bool AstCompiler::is_keyword(std::string_view txt) const
	{
		return keywords.find(txt) != keywords.end();
	}
	bool AstCompiler::is_identifier(std::string_view txt) const
	{
		assert(!txt.empty());
		if (!is_alpha_or_underscore(txt[0])) return false;
		for(auto i = 1u; i < txt.size(); ++i) {
			if (!is_alpha_or_underscore_or_digit(txt[i])) return false;
		}
		return true;
	}
	bool AstCompiler::is_operator(std::string_view txt) const
	{
		if (txt.size() == 2) {
			for (auto q : operators_2) {
				if (q == txt) return true;
			}
			return false;
		}
		if (txt.size() == 1) {
			return operators_1.find(txt[0]) != std::string::npos;
		}
		return false;
	}
	bool AstCompiler::is_valid_number(std::string_view txt) const
	{
		unsigned int base = 0;

		if (txt.substr(0, 2) == "0x" || txt.substr(0, 2) == "0X") {
			base = 16;
			txt = txt.substr(2);
		}
		else if (txt.substr(0, 2) == "0o" || txt.substr(0, 2) == "0O") {
			base = 8;
			txt = txt.substr(2);
		}
		else if (txt.substr(0, 2) == "0b" || txt.substr(0, 2) == "0B") {
			base = 2;
			txt = txt.substr(2);
		}
		if (base) {
			for (auto q : txt) {
				if (!is_digit(q, base)) return false;
			}
			return true;
		}

		auto i = 0u;
		while (i < txt.size() && is_digit(txt[i])) ++i;
		if (i < txt.size() && txt[i] == '.') {
			++i;
			while (i < txt.size() && is_digit(txt[i])) ++i;
			if (i == 1)
				return false;
		}
		if (i < txt.size() && (txt[i] == 'e' || txt[i] == 'E')) {
			++i;
			bool succ = false;
			if (i < txt.size() && (txt[i] == '-' || txt[i] == '+'))
				++i;
			if (i < txt.size() && is_digit(txt[i]))
				succ = true;
			if (!succ) 
				return false;
			while (i < txt.size() && is_digit(txt[i])) ++i;
		}
		return i == txt.size();
	}
	void AstCompiler::add_error(OwcaErrorKind kind_, std::string file_, Line line_, std::string message_) {
		error_messages_.push_back({ kind_, std::move(file_), line_.line, std::move(message_) });
	}

	void AstCompiler::add_error_and_throw(OwcaErrorKind kind_, std::string file_, Line line_, std::string message_) {
		add_error(kind_, std::move(file_), line_, std::move(message_));
		throw CompilationError{};
	}

	std::unique_ptr<AstExpr> AstCompiler::compile_parse_constant_number(Line line, std::string_view text)
	{
		auto orig_text = text;
		unsigned int base = 0;

		if (text.size() > 1) {
			if (text.substr(0, 2) == "0x" || text.substr(0, 2) == "0X") {
				base = 16;
				text = text.substr(2);
			}
			else if (text.substr(0, 2) == "0b" || text.substr(0, 2) == "0B") {
				base = 2;
				text = text.substr(2);
			}
			else if (text.substr(0, 2) == "0o" || text.substr(0, 2) == "0O") {
				base = 8;
				text = text.substr(2);
			}
		}
		if (base == 0) {
			bool all = true;
			for (auto& q : text) {
				if (q < '0' || q > '9') {
					all = false;
					break;
				}
			}
			if (all) base = 10;
		}

		if (base > 0) {
			OwcaIntInternal value = 0;
			auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value, base);

			if (ec == std::errc() && ptr == text.data() + text.size()) {
				return std::make_unique<AstExprConstant>(line, OwcaInt{ value });
			}
			else if (ec == std::errc() || ec == std::errc::invalid_argument) {
				add_error_and_throw(OwcaErrorKind::InvalidNumber, filename_, line, std::format("`{}` is not a valid number", orig_text));
			}
			else if (ec == std::errc::result_out_of_range) {
				add_error_and_throw(OwcaErrorKind::InvalidNumber, filename_, line, std::format("`{}` doesn't fit in range of allowed values ({} -> {}) for given OwcaIntInternal type",
					orig_text, std::numeric_limits<OwcaIntInternal>::min(), std::numeric_limits<OwcaIntInternal>::max()));
			}
			else {
				assert(false);
			}
		}
		else {
			OwcaFloatInternal value = 0;
			auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);

			if (ec == std::errc() && ptr == text.data() + text.size()) {
				return std::make_unique<AstExprConstant>(line, OwcaFloat{ value });
			}
			else if (ec == std::errc() || ec == std::errc::invalid_argument) {
				add_error_and_throw(OwcaErrorKind::InvalidNumber, filename_, line, std::format("`{}` is not a valid number", orig_text));
			}
			else if (ec == std::errc::result_out_of_range) {
				add_error_and_throw(OwcaErrorKind::InvalidNumber, filename_, line, std::format("`{}` doesn't fit in range of allowed values for given OwcaFloatInternal type", orig_text));
			}
			else {
				assert(false);
			}
		}
		return nullptr;
	}

	std::unique_ptr<AstExpr> AstCompiler::compile_expr_value()
	{
		auto [line, tok] = consume();
		if (tok == "true") return std::make_unique<AstExprConstant>(line, OwcaBool{ true });
		if (tok == "false") return std::make_unique<AstExprConstant>(line, OwcaBool{ false });
		if (tok == "nul") return std::make_unique<AstExprConstant>(line, OwcaEmpty{});
		if (is_digit(tok[0]) || tok[0] == '.') return compile_parse_constant_number(line, tok);
		
		if (is_alpha_or_underscore(tok[0])) {
			if (is_keyword(tok)) {
				add_error_and_throw(OwcaErrorKind::ExpectedIdentifier, filename_, line, std::format("unexpected keyword `{}`", tok));
			}
			return std::make_unique<AstExprIdentifier>(line, tok);
		}
		if (tok[0] == '\'' || tok[0] == '"') {
			return std::make_unique<AstExprConstant>(line, std::string{ tok.substr(1, tok.size() - 2) });
		}
		if (tok == "(") {
			AllowRangeSet allow_range_set{ allow_range, true };
			auto v = compile_expression_no_assign();
			if (preview().second == ")") {
				consume(")");
				return v;
			}
			std::vector<std::unique_ptr<AstExpr>> values;
			values.push_back(std::move(v));
			while (preview().second != ")") {
				consume(",");
				if (preview().second == ")") break;
				values.push_back(compile_expression_no_assign());
			}
			consume(")");
			return std::make_unique<AstExprOperX>(line, AstExprOperX::Kind::CreateTuple, std::move(values));
		}
		if (tok == "[") {
			AllowRangeSet allow_range_set{ allow_range, true };
			std::vector<std::unique_ptr<AstExpr>> values;

			while (preview().second != "]") {
				if (!values.empty()) {
					consume(",");
				}
				if (!values.empty() && preview().second == "]") break;
				values.push_back(compile_expression_no_assign());
			}
			consume("]");
			return std::make_unique<AstExprOperX>(line, AstExprOperX::Kind::CreateArray, std::move(values));
		}
		if (tok == "{") {
			AllowRangeSet allow_range_set{ allow_range, false };
			std::vector<std::unique_ptr<AstExpr>> values;
			bool map = true;

			while (preview().second != "}") {
				if (!values.empty()) {
					consume(",");
				}
				if (!values.empty() && preview().second == "}") break;
				auto v = compile_expression_no_assign(false);
				values.push_back(std::move(v));
				if (values.size() == 1) {
					if (preview().second == ":") {
						map = true;
					}
					else {
						map = false;
					}
				}
				if (map) {
					consume(":");
					AllowRangeSet allow_range_set{ allow_range, true };
					values.push_back(compile_expression_no_assign(false));
				}
			}
			consume("}");
			return std::make_unique<AstExprOperX>(line, map ? AstExprOperX::Kind::CreateMap : AstExprOperX::Kind::CreateSet, std::move(values));
		}
		add_error_and_throw(OwcaErrorKind::ExpectedValue, filename_, line, 
			std::format("unexpected token `{}`, expected value, which is true, false, nul, number, string, identifier, expression in parenthesis, array construction object or set / map construction object", tok)
		);
	}
	std::unique_ptr<AstExpr> AstCompiler::compile_expr_postfix()
	{
		auto left = compile_expr_value();
		while (true) {
			auto tok = preview().second;

			if (tok == "(") {
				AllowRangeSet allow_range_set{ allow_range, true };
				std::vector<std::unique_ptr<AstExpr>> args;
				args.push_back(std::move(left));
				auto line = consume().first;

				if (preview().second != ")") {
					while (true) {
						if (args.size() > 1) {
							consume(",");
						}
						args.push_back(compile_expression_no_assign());
						if (preview().second == ")") break;
					}
				}
				consume(")");
				left = std::make_unique<AstExprOperX>(line, AstExprOperX::Kind::Call, std::move(args));
			}
			else if (tok == "[") {
				AllowRangeSet allow_range_set{ allow_range, true };
				auto line = consume().first;
				auto key = compile_expression_no_assign();
				consume("]");
				left = std::make_unique<AstExprOper2>(line, AstExprOper2::Kind::IndexRead, std::move(left), std::move(key));
			}
			else if (tok == ".") {
				auto line = consume().first;
				auto [line2, tok] = consume();
				if (is_alpha_or_underscore(tok[0])) {
					if (is_keyword(tok)) {
						add_error_and_throw(OwcaErrorKind::ExpectedIdentifier, filename_, line2, std::format("expected identifier, got keyword `{}`", tok));
					}
					left = std::make_unique<AstExprMember>(line, std::move(left), std::string{ tok });
				}
				else {
					add_error_and_throw(OwcaErrorKind::ExpectedIdentifier, filename_, line2, std::format("expected identifier, got `{}`", tok));
				}
			}
			else {
				break;
			}
		}
		return left;
	}
	std::unique_ptr<AstExpr> AstCompiler::compile_expr_prefix()
	{
		auto tok = preview().second;
		std::optional<AstExprOper1::Kind> kind;

		if (tok == "~") kind = AstExprOper1::Kind::BinNeg;
		else if (tok == "not") kind = AstExprOper1::Kind::LogNot;
		else if (tok == "-") kind = AstExprOper1::Kind::Negate;

		if (kind) {
			auto [ line, tok ] = consume();
			auto left = compile_expr_postfix();
			return std::make_unique<AstExprOper1>(line, *kind, std::move(left));
		}
		return compile_expr_postfix();
	}
	std::unique_ptr<AstExpr> AstCompiler::compile_expr_mul_div_mod()
	{
		auto left = compile_expr_prefix();
		auto tok = preview().second;
		std::optional<AstExprOper2::Kind> kind;

		if (tok == "*") kind = AstExprOper2::Kind::Mul;
		else if (tok == "/") kind = AstExprOper2::Kind::Div;
		else if (tok == "%") kind = AstExprOper2::Kind::Mod;

		if (kind) {
			auto [ line, tok ] = consume();
			auto right = compile_expr_mul_div_mod();
			left = std::make_unique<AstExprOper2>(line, *kind, std::move(left), std::move(right));
		}
		return left;
	}
	std::unique_ptr<AstExpr> AstCompiler::compile_expr_add_sub()
	{
		auto left = compile_expr_mul_div_mod();
		auto tok = preview().second;
		std::optional<AstExprOper2::Kind> kind;

		if (tok == "+") kind = AstExprOper2::Kind::Add;
		else if (tok == "-") kind = AstExprOper2::Kind::Sub;

		if (kind) {
			auto [ line, tok ] = consume();
			auto right = compile_expr_add_sub();
			left = std::make_unique<AstExprOper2>(line, *kind, std::move(left), std::move(right));
		}
		return left;
	}
	std::unique_ptr<AstExpr> AstCompiler::compile_expr_bitwise()
	{
		auto left = compile_expr_add_sub();
		auto tok = preview().second;
		std::optional<AstExprOper2::Kind> kind;

		if (tok == "|") kind = AstExprOper2::Kind::BinOr;
		else if (tok == "^") kind = AstExprOper2::Kind::BinXor;
		else if (tok == "&") kind = AstExprOper2::Kind::BinAnd;
		else if (tok == "<<") kind = AstExprOper2::Kind::BinLShift;
		else if (tok == ">>") kind = AstExprOper2::Kind::BinRShift;

		if (kind) {
			auto [ line, tok ] = consume();
			auto right = compile_expr_bitwise();
			left = std::make_unique<AstExprOper2>(line, *kind, std::move(left), std::move(right));
		}
		return left;
	}
	std::unique_ptr<AstExpr> AstCompiler::compile_expr_range()
	{
		std::unique_ptr<AstExpr> left, right;

		if (!allow_range || preview().second != ":")
			left = compile_expr_bitwise();
		if (allow_range) {
			auto tok = preview().second;
			if (tok == ":") {
				auto [line, tok] = consume();
				tok = preview().second;
				if (tok == "-" || tok == "~" || !is_operator(tok))
					right = compile_expr_bitwise();
				left = std::make_unique<AstExprOper2>(line, AstExprOper2::Kind::MakeRange, std::move(left), std::move(right));
			}
		}
		return left;
	}
	std::unique_ptr<AstExpr> AstCompiler::compile_expr_compare()
	{
		auto left = compile_expr_range();
		std::vector<std::tuple<CompareKind, Line, std::unique_ptr<AstExpr>>> nexts;
		while (true) {
			auto tok = preview().second;
			std::optional<CompareKind> kind;
			if (tok == "==") kind = CompareKind::Eq;
			else if (tok == "!=") kind = CompareKind::NotEq;
			else if (tok == ">=") kind = CompareKind::MoreEq;
			else if (tok == "<=") kind = CompareKind::LessEq;
			else if (tok == ">") kind = CompareKind::More;
			else if (tok == "<") kind = CompareKind::Less;
			else if (tok == "is") kind = CompareKind::Is;
			if (!kind) break;

			auto [line, tok2] = consume();
			auto right = compile_expr_bitwise();
			nexts.push_back({ *kind, line, std::move(right) });
		}

		if (!nexts.empty()) {
			left = std::make_unique<AstExprCompare>(std::get<1>(nexts[0]), std::move(left), std::move(nexts));
		}
		return left;
	}
	std::unique_ptr<AstExpr> AstCompiler::compile_expr_log_and()
	{
		auto left = compile_expr_compare();
		if (preview().second == "and") {
			auto [ line, tok ] = consume();
			auto right = compile_expr_log_and();
			left = std::make_unique<AstExprOper2>(line, AstExprOper2::Kind::LogAnd, std::move(left), std::move(right));
		}
		return left;
	}
	std::unique_ptr<AstExpr> AstCompiler::compile_expr_log_or()
	{
		auto left = compile_expr_log_and();
		if (preview().second == "or") {
			auto [ line, tok ] = consume();
			auto right = compile_expr_log_or();
			left = std::make_unique<AstExprOper2>(line, AstExprOper2::Kind::LogOr, std::move(left), std::move(right));
		}
		return left;
	}

	std::unique_ptr<AstExpr> AstCompiler::compile_expression_no_assign(bool allow_ranges_val)
	{
		AllowRangeSet allow_range_set{ allow_ranges_val, false };
		return compile_expr_log_or();
	}

	struct AstCompiler::RewriteAsWrite : public AstVisitor {
		AstCompiler* compiler;
		std::unique_ptr<AstExpr> right;
		std::unique_ptr<AstExpr> res;

		RewriteAsWrite(AstCompiler* compiler, std::unique_ptr<AstExpr> right) : compiler(compiler), right(std::move(right)) {}

		void apply(AstBase &o) override {
			compiler->add_error_and_throw(OwcaErrorKind::NotALValue, compiler->filename_, o.line, std::format("not a l-value"));
		}
		void apply(AstExprIdentifier &o) override {
			o.update_value_to_write(std::move(right));
		}
		void apply(AstExprMember &o) override {
			o.update_value_to_write(std::move(right));
		}
		void apply(AstExprOper2 &o) override {
			if (o.kind() == AstExprOper2::Kind::IndexRead) {
				o.update_value_to_write(AstExprOper2::Kind::IndexWrite, std::move(right));
			}
			else {
				apply(static_cast<AstExpr&>(o));
			}
		}
	};

	std::unique_ptr<AstExpr> AstCompiler::compile_expression()
	{
		auto left = compile_expression_no_assign();
		if (preview().second == "=") {
			auto [ line, tok ] = consume();
			auto right = compile_expression();
			auto vis = RewriteAsWrite{ this, std::move(right) };
			left->visit(vis);
			if (vis.res) left = std::move(vis.res);
		}
		return left;
	}

	std::unique_ptr<AstStat> AstCompiler::compile_expression_as_stat()
	{
		auto e = compile_expression();
		consume(";");
		auto line = e->line;
		return std::make_unique<AstExprAsStat>(line, std::move(e));
	}

	std::unique_ptr<AstStat> AstCompiler::compile_block()
	{
		const auto start_line = consume("{");
		std::vector<std::unique_ptr<AstStat>> temp;

		try {
			while (!eof()) {
				auto [line, tok] = preview();
				if (tok == "}") break;

				temp.push_back(compile_stat());
			}
			consume("}");
		}
		catch (CompilationError) {
			if (!continue_) throw;
			unsigned int cnt = 1;

			while (!eof()) {
				auto [line, tok] = consume();
				if (tok == "}") {
					--cnt;
					if (cnt == 0) break;
				}
				else if (tok == "{") ++cnt;
			}
			if (cnt > 0) {
				add_error_and_throw(OwcaErrorKind::UnexpectedEndOfFile, filename_, content_line, std::format("unexpected end of file, when trying to find a closing bracket for opened in line {}", start_line));
			}
		}

		return std::make_unique<AstBlock>(start_line, std::move(temp));
	}

	std::unique_ptr<AstClass> AstCompiler::compile_class_raw()
	{
		auto line = consume("class");
		auto [line2, class_name] = consume();
		bool use_native = false;
		if (class_name == "native" && preview().second != "(") {
			auto [l, f] = consume();
			line2 = l;
			class_name = f;
			use_native = true;
		}
		if (!is_identifier(class_name)) {
			add_error_and_throw(OwcaErrorKind::ExpectedIdentifier, filename_, line2, std::format("expected class' name, got `{}`", class_name));
		}
		std::vector<std::unique_ptr<AstExpr>> base_classes;
		std::vector<std::unique_ptr<AstFunction>> members;

		if (preview().second == "(") {
			consume("(");
			while (true) {
				if (!base_classes.empty()) {
					consume(",");
				}

				base_classes.push_back(compile_expression_no_assign());
				if (preview().second == ")") break;
			}
			consume(")");
		}
		auto full_name_updater = FullNameUpdater{ full_name, class_name };
		consume("{");
		while (preview().second != "}") {
			auto f = compile_function_raw();
			members.push_back(std::move(f));
		}
		consume("}");

		auto c = std::make_unique<AstClass>(line, std::string{ class_name }, std::string{ full_name }, std::move(base_classes), std::move(members), use_native);
		return c;
	}

	std::unique_ptr<AstFunction> AstCompiler::compile_function_raw()
	{
		auto function_updater = FunctionUpdater{ *this };
		auto line = consume("function");
		auto [line2, func_name] = consume();
		bool use_native = false, is_generator = false;

		while(preview().second != "(") {
			if (func_name == "native") {
				if (use_native)
					add_error_and_throw(OwcaErrorKind::InvalidIdentifier, filename_, line2, "reused function's `native` specifier");
				auto [ l, f] = consume();
				line2 = l;
				func_name = f;
				use_native = true;
			}
			else if (func_name == "generator") {
				if (is_generator)
					add_error_and_throw(OwcaErrorKind::InvalidIdentifier, filename_, line2, "reused function's `generator` specifier");
				auto [ l, f] = consume();
				line2 = l;
				func_name = f;
				is_generator = true;
			}
		}
		if (!is_identifier(func_name)) {
			add_error_and_throw(OwcaErrorKind::ExpectedIdentifier, filename_, line2, std::format("expected identifier for function name, got `{}`", func_name));
		}
		std::vector<std::string> param_names;

		consume("(");
		if (preview().second != ")") {
			std::unordered_set<std::string> idents;

			while (true) {
				if (!param_names.empty()) {
					consume(",");
				}
				auto [line3, p_name] = consume();
				if (!is_identifier(p_name)) {
					add_error_and_throw(OwcaErrorKind::ExpectedIdentifier, filename_, line3, std::format("expected identifier for function's parameter name, got `{}`", p_name));
				}
				param_names.push_back(std::string{ p_name });
				if (!idents.insert(std::string{ p_name }).second) {
					add_error_and_throw(OwcaErrorKind::InvalidIdentifier, filename_, line3, std::format("reused function's parameter name `{}`", p_name));
				}
				if (preview().second == ")") break;
			}
			for (auto i = 1u; i < param_names.size(); ++i) {
				if (param_names[i] == "self") {
					add_error_and_throw(OwcaErrorKind::InvalidIdentifier, filename_, line2, std::format("only first argument might be `self`, not {}", i));
				}
			}
		}
		consume(")");
		auto full_name_updater = FullNameUpdater{ full_name, func_name };
		auto f = std::make_unique<AstFunction>(line, std::string{ func_name }, full_name, std::move(param_names), use_native ? AstFunction::Native::Yes : AstFunction::Native::No, is_generator ? AstFunction::Generator::Yes : AstFunction::Generator::No);
		if (use_native) {
			auto [txt_line, txt] = preview();
			if (txt == "{") {
				add_error_and_throw(OwcaErrorKind::SyntaxError, filename_, txt_line, "native function can't have body");
			}
			consume(";");
		}
		else {
			struct PopOnExit {
				AstCompiler* self;
				~PopOnExit() {
					self->functions_stack.pop_back();
				}
			};
			functions_stack.push_back(f.get());
			PopOnExit pop_on_exit{ this };

			auto body = compile_block();
			f->update_body(std::move(body));
		}
		return f;
	}
	
	bool AstCompiler::preview_is_loop_identifier()
	{
		auto start = content_offset;
		auto line = content_line;
		auto tok = preview().second;
		bool is_loop_identifier = false;

		if (is_identifier(tok)) {
			consume(tok);
			if (preview().second == ":") 
				is_loop_identifier = true;
		}
		content_offset = start;
		content_line = line;
		return is_loop_identifier;
	}

	std::unique_ptr<AstStat> AstCompiler::compile_break_or_continue()
	{
		auto [ line, tok ] = consume();
		assert(tok == "break" || tok == "continue");
		if (loop_control_depth == 0) {
			add_error(OwcaErrorKind::LoopControlError, filename_, line, std::format("`{}` outside loop", tok));
		}
		unsigned int depth = loop_control_depth - 1;
		if (preview().second != ";") {
			auto ident = consume().second;
			auto it = loop_control_identifiers.find(ident);
			if (it == loop_control_identifiers.end()) {
				add_error(OwcaErrorKind::LoopControlError, filename_, line, std::format("no loop with `{}` identifier in current function", ident));
			}
			else {
				depth = it->second.second;
			}
		}
		consume(";");
		if (tok == "break") {
			return std::make_unique<AstLoopControl>(line, AstLoopControl::Mode::Break, depth);
		}
		if (tok == "continue") {
			return std::make_unique<AstLoopControl>(line, AstLoopControl::Mode::Continue, depth);
		}
		assert(false);
		throw 1;
	}

	std::unique_ptr<AstStat> AstCompiler::compile_while(std::string_view loop_ident)
	{
		auto line = consume("while");
		consume("(");
		std::unique_ptr<AstExpr> val = compile_expression_no_assign();
		consume(")");
		auto control_depth = loop_control_depth;
		auto lcu = LoopControlUpdater{ *this, line, loop_ident };
		auto body = compile_stat();
		return std::make_unique<AstWhile>(line, control_depth, loop_ident, std::move(val), std::move(body));
	}

	std::unique_ptr<AstStat> AstCompiler::compile_for(std::string_view loop_ident)
	{
		auto line = consume("for");
		consume("(");
		auto [ text_line, text ] = consume();
		if (!is_identifier(text))
			add_error_and_throw(OwcaErrorKind::ExpectedIdentifier, filename_, text_line, std::format("expected identifier for iteration's value, got `{}`", text));
		consume("=");
		std::unique_ptr<AstExpr> iterator = compile_expression_no_assign();
		consume(")");
		auto control_depth = loop_control_depth;
		auto lcu = LoopControlUpdater{ *this, line, loop_ident };
		auto body = compile_stat();
		return std::make_unique<AstFor>(line, control_depth, loop_ident, text, std::move(iterator), std::move(body));
	}

	std::unique_ptr<AstStat> AstCompiler::compile_return()
	{
		auto line = consume("return");
		std::unique_ptr<AstExpr> val;
		if (!eof() && preview().second != ";") {
			if (!functions_stack.empty() && functions_stack.back()->is_generator()) {
				add_error_and_throw(OwcaErrorKind::SyntaxError, filename_, line, "can't use return statement with value in generator function");
			}
			val = compile_expression_no_assign();
		}
		consume(";");
		return std::make_unique<AstReturn>(line, std::move(val));
	}

	std::unique_ptr<AstStat> AstCompiler::compile_yield()
	{
		auto line = consume("yield");
		if (functions_stack.empty()) {
			add_error_and_throw(OwcaErrorKind::SyntaxError, filename_, line, "can't use yield statement in global scope");
		}
		if (functions_stack.empty() || !functions_stack.back()->is_generator()) {
			add_error_and_throw(OwcaErrorKind::SyntaxError, filename_, line, std::format("function `{}` is not marked as generator, can't use yield statement", functions_stack.back()->name()));
		}
		auto val = compile_expression_no_assign();
		consume(";");
		return std::make_unique<AstYield>(line, std::move(val));
	}

	std::unique_ptr<AstStat> AstCompiler::compile_try()
	{
		auto line = consume("try");
		auto body = compile_block();

		std::vector<std::tuple<std::string_view, unsigned int, std::vector<std::unique_ptr<AstExpr>>, std::unique_ptr<AstStat>>> catches;

		while(catches.empty() || preview().second == "catch") {
			consume("catch");
			consume("(");
			std::string_view identifier;
			if (preview_is_loop_identifier()) {
				auto [ line, txt ] = consume();
				if (!is_identifier(txt)) add_error_and_throw(OwcaErrorKind::ExpectedIdentifier, filename_, line, std::format("expected identifier, got `{}`", txt));
				identifier = txt;
				consume(":");
			}
			std::vector<std::unique_ptr<AstExpr>> types;
			while(true) {
				if (!types.empty()) {
					if (preview().second != "|") break;
					consume("|");
				}
				types.push_back(compile_expression_no_assign());
			}
			consume(")");
			auto body2 = compile_block();
			catches.push_back({ identifier, std::numeric_limits<unsigned int>::max(), std::move(types), std::move(body2) });
		}
		return std::make_unique<AstTry>(line, std::move(body), std::move(catches));
	}
	
	std::unique_ptr<AstStat> AstCompiler::compile_throw()
	{
		auto line = consume("throw");
		auto val = compile_expression_no_assign();
		consume(";");
		return std::make_unique<AstThrow>(line, std::move(val));
	}
	
	std::unique_ptr<AstStat> AstCompiler::compile_if(bool elif)
	{
		auto line = consume(elif ? "elif" : "if");
		consume("(");
		std::unique_ptr<AstExpr> val = compile_expression_no_assign();
		consume(")");
		auto if_true = compile_stat();
		auto if_false = [&]() -> std::unique_ptr<AstStat> {
			auto tok = preview().second;
			if (tok == "elif") return compile_if(true);
			if (tok == "else") {
				consume("else");
				return compile_stat();
			}
			return nullptr;
		}();
		return std::make_unique<AstIf>(line, std::move(val), std::move(if_true), std::move(if_false));
	}

	std::unique_ptr<AstStat> AstCompiler::compile_function()
	{
		auto f = compile_function_raw();
		auto line = f->line;
		auto fi = std::make_unique<AstExprIdentifier>(line, f->name());
		fi->update_value_to_write(std::move(f));
		fi->set_function_write();
		auto f2 = std::make_unique<AstExprAsStat>(line, std::move(fi));
		return f2;
	}

	std::unique_ptr<AstStat> AstCompiler::compile_class()
	{
		auto f = compile_class_raw();
		auto line = f->line;
		auto fi = std::make_unique<AstExprIdentifier>(line, f->name());
		fi->update_value_to_write(std::move(f));
		auto f2 = std::make_unique<AstExprAsStat>(line, std::move(fi));
		return f2;
	}

	std::unique_ptr<AstStat> AstCompiler::compile_stat()
	{
		if (eof())
			add_error_and_throw(OwcaErrorKind::UnexpectedEndOfFile, filename_, content_line, "unexpected end of file");

		std::string_view ident;
		if (preview_is_loop_identifier()) {
			ident = consume().second;
			consume(":");
		}
		auto [ line, tok ] = preview();
		if (tok == "while") return compile_while(ident);
		if (tok == "for") return compile_for(ident);
		if (!ident.empty())
			add_error_and_throw(OwcaErrorKind::SyntaxError, filename_, line, std::format("unexpected token `{}`, expected while because loop identifier `{}` is present", tok, ident));
		if (tok == "function") return compile_function();
		if (tok == "class") return compile_class();
		if (tok == "return") return compile_return();
		if (tok == "yield") return compile_yield();
		if (tok == "if") return compile_if();
		if (tok == "try") return compile_try();
		if (tok == "throw") return compile_throw();
		if (tok == "break" || tok == "continue") return compile_break_or_continue();
		if (tok == "{") return compile_block();
		return compile_expression_as_stat();
	}

	std::unique_ptr<AstFunction> AstCompiler::compile_main_block()
	{
		functions_stack.clear();
		auto mb = std::format("<main-block {}>", filename_);
		auto f = std::make_unique<AstFunction>(Line{ 1 }, mb, mb, std::vector<std::string>{}, AstFunction::Native::No, AstFunction::Generator::No);
		functions_stack.push_back(f.get());
		
		try {
			std::vector<std::unique_ptr<AstStat>> temp;

			while (!eof()) {
				temp.push_back(compile_stat());
			}

			functions_stack.pop_back();
			f->update_body(std::make_unique<AstBlock>(Line{ 1 }, std::move(temp)));

			return f;
		}
		catch (CompilationError) {
			return nullptr;
		}
	}

	struct AstCompiler::Phase2 : public AstVisitor {
		struct Stack {
			enum class IdentifierState {
				Unknown, Writable, ReadOnly
			};
			std::unordered_map<std::string_view, std::pair<unsigned int, IdentifierState>> identifiers;
			std::vector<std::string_view> identifier_names;
			using CopyFromParent = AstFunction::CopyFromParent;
			std::vector<CopyFromParent> copy_from_parents;

			AstFunction* owner;
			Stack* parent = nullptr;

			auto define_identifier(std::string_view name) {
				auto it = identifiers.insert({ name, { (unsigned int)identifiers.size(), IdentifierState::Unknown } });
				if (it.second) {
					identifier_names.push_back(it.first->first);
				}
				return it.first;
			}
			std::optional<std::pair<unsigned int, bool>> lookup_identifier(std::string_view name) {
				auto it = identifiers.find(name);
				if (it != identifiers.end()) {
					bool wa;
					switch(it->second.second) {
					case IdentifierState::Writable: wa = true; break;
					case IdentifierState::ReadOnly: wa = false; break;
					case IdentifierState::Unknown:
						if (parent) {
							auto p = parent->lookup_identifier(name);
							wa = !p;
						}
						else wa = true;
						it->second.second = wa ? IdentifierState::Writable : IdentifierState::ReadOnly;
						break;
					}
					return std::pair<unsigned int, bool>{ it->second.first, wa };
				}

				if (parent == nullptr)
					return std::nullopt;

				auto pp = parent->lookup_identifier(name);
				if (!pp)
					return std::nullopt;
				auto [ p, _ ] = *pp;

				auto it2 = define_identifier(name);
				copy_from_parents.push_back({ p, it2->second.first });
				it2->second.second = IdentifierState::ReadOnly;

				return std::pair<unsigned int, bool>{ it2->second.first, false };
			}
			unsigned int ensure_writable_identifier(AstCompiler *comp, Line line, std::string_view name) {
				auto index_pp = lookup_identifier(name);
				assert(index_pp);
				auto [ index, writeable ] = *index_pp;
				if (!writeable) comp->add_error(OwcaErrorKind::VariableIsConstant, comp->filename_, line, std::format("variable `{}` is constant - it has been copied from parent function's stack", name));
				return index;
			}
		};
		std::unordered_map<AstFunction*, Stack> stacks;
		std::span<const std::string> additional_variables;
		Stack* current_stack = nullptr;
		bool first_run = true;
		AstCompiler* compiler;

		Phase2(AstCompiler* compiler, std::span<const std::string> additional_variables) : additional_variables(additional_variables), compiler(compiler) {}

		void add_error(OwcaErrorKind kind, Line line, std::string msg) {
			compiler->add_error(kind, compiler->filename_, line, std::move(msg));
		}
		using AstVisitor::apply;
		void apply(AstBase &o) override {
			o.visit_children(*this);
		}
		void apply(AstWhile &o) override {
			if (first_run) {
				if (!o.get_loop_identifier().empty()) {
					current_stack->define_identifier(o.get_loop_identifier());
				}
			}
			else {
				if (!o.get_loop_identifier().empty()) {
					auto index = current_stack->ensure_writable_identifier(compiler, o.line, o.get_loop_identifier());
					o.update_loop_ident_index(index);
				}
			}
			apply(static_cast<AstStat&>(o));
		}
		void apply(AstFor &o) override {
			if (first_run) {
				if (!o.get_loop_identifier().empty()) {
					current_stack->define_identifier(o.get_loop_identifier());
				}
				current_stack->define_identifier(o.get_value());
			}
			else {
				if (!o.get_loop_identifier().empty()) {
					auto index = current_stack->ensure_writable_identifier(compiler, o.line, o.get_loop_identifier());
					o.update_loop_ident_index(index);
				}
				auto index = current_stack->ensure_writable_identifier(compiler, o.line, o.get_value());
				o.update_value_index(index);
			}
			apply(static_cast<AstStat&>(o));
		}
		void apply(AstTry &o) override {
			if (first_run) {
				for(auto i = 0u; i < o.catch_count(); ++i) {
					auto ident = o.catch_identifier(i);
					if (!ident.empty())
						current_stack->define_identifier(ident);
				}
			}
			else {
				for(auto i = 0u; i < o.catch_count(); ++i) {
					auto ident = o.catch_identifier(i);
					if (!ident.empty()) {
						auto index = current_stack->ensure_writable_identifier(compiler, o.catch_line(i), ident);
						o.update_catch_index(i, index);
					}
				}
			}
			apply(static_cast<AstStat&>(o));
		}
		void apply(AstExprIdentifier &o) override {
			if (first_run) {
				if (o.write()) {
					current_stack->define_identifier(o.identifier());
				}
			}
			else {
				auto index_pp = current_stack->lookup_identifier(o.identifier());
				if (!index_pp) {
					add_error(OwcaErrorKind::UndefinedIdentifier, o.line, std::format("variable `{}` has not been written", o.identifier()));
				}
				else {
					auto [ index, writable ] = *index_pp;
					if (o.write()) {
						if (!writable) 
							add_error(OwcaErrorKind::VariableIsConstant, o.line, std::format("variable `{}` is constant - it has been copied from parent function's stack", o.identifier()));
					}
					o.update_index(index);
				}
			}
			apply(static_cast<AstExpr&>(o));
		}
		void apply(AstFunction &o) override {
			auto &st = stacks[&o];
			st.owner = &o;
			st.parent = current_stack;
			current_stack = &st;

			if (first_run) {
				if (current_stack->parent == nullptr) {
					for(auto &it : compiler->vm.get_builtin_objects()) {
						current_stack->define_identifier(it.first);
					}
					for (auto& ident : additional_variables)
						current_stack->define_identifier(ident);
				}
				for (auto& p : o.parameters()) {
					current_stack->define_identifier(p);
				}
			}

			apply(static_cast<AstExpr&>(o));
			
			if (!first_run) {
				o.update_copy_from_parents(std::move(st.copy_from_parents));
				o.update_identifier_names(std::move(st.identifier_names));
			}

			current_stack = st.parent;
		}
	};

	void AstCompiler::compile_phase_2(AstFunction &root, std::span<const std::string> additional_variables)
	{
		auto p = Phase2{ this, additional_variables };
		p.first_run = true;
		root.visit(p);
		p.first_run = false;
		root.visit(p);
	}

	std::shared_ptr<CodeBuffer> AstCompiler::compile(std::span<const std::string> additional_variables)
	{
		auto root = compile_main_block();
		if (!error_messages_.empty()) {
			assert(!error_messages_.empty());
			return nullptr;
		}
		assert(root);

		compile_phase_2(*root, additional_variables);
		if (!error_messages_.empty()) {
			return nullptr;
		}

		auto code_buffer_size_calc = CodeBufferSizeCalculator{};
		root->calculate_size(code_buffer_size_calc);

		auto emit_info = AstBase::EmitInfo{ CodeBuffer{ filename(), code_buffer_size_calc.get_total_size(), std::move(native_code_provider) } };
		root->emit(emit_info);

		emit_info.code_buffer.validate_size(code_buffer_size_calc.get_total_size());

		return std::make_shared<CodeBuffer>(std::move(emit_info.code_buffer));
	}
}