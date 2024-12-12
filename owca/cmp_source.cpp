#include "stdafx.h"
#include "base.h"
#include "cmp_source.h"
#include "message.h"

namespace owca { namespace __owca__ {

	const std::string &source::token_text() const
	{
		return a->text();
	}

	tokentype source::token_type() const
	{
		return a->type();
	}

	unsigned int source::token_location()
	{
		return a->line();
	}

	bool source::is_keyword() const
	{
		return a->type()==TOKEN_KEYWORD;
	}

	bool source::is_keyword(const std::string &k) const
	{
		return a->type()==TOKEN_KEYWORD && a->text()==k;
	}

	bool source::is_string() const
	{
		return a->type()==TOKEN_STRING;
	}

	bool source::is_int() const
	{
		return a->type()==TOKEN_INT;
	}

	bool source::is_real() const
	{
		return a->type()==TOKEN_REAL;
	}

	bool source::is_ident() const
	{
		return a->type()==TOKEN_IDENT;
	}

	bool source::is_ident(const std::string &k) const
	{
		return a->type()==TOKEN_IDENT && a->text()==k;
	}

	bool source::is_oper() const
	{
		return a->type()==TOKEN_OPER;
	}

	bool source::is_oper(const std::string &o) const
	{
		return a->type()==TOKEN_OPER && a->text()==o;
	}

	bool source::is_eo_l() const
	{
		return a->type()==TOKEN_EOL;
	}

	bool source::is_up() const
	{
		return a->type()==TOKEN_UP;
	}

	bool source::is_down() const
	{
		return a->type()==TOKEN_DOWN;
	}

	void source::assert_ident(const std::string &k)
	{
		if (!is_ident() || a->text()!=k) throw error_information(owca::YERROR_SYNTAX_ERROR,*a);
	}

	void source::assert_oper(const std::string &k)
	{
		if (!is_oper() || a->text()!=k) throw error_information(owca::YERROR_SYNTAX_ERROR,*a);
	}

	void source::assert_keyword(const std::string &k)
	{
		if (!is_keyword() || a->text()!=k) throw error_information(owca::YERROR_SYNTAX_ERROR,*a);
	}

	void source::assert_eo_l()
	{
		if (!is_eo_l()) throw error_information(owca::YERROR_SYNTAX_ERROR,*a);
	}

	void source::assert_up()
	{
		if (!is_up()) throw error_information(owca::YERROR_SYNTAX_ERROR,*a);
	}

} }












