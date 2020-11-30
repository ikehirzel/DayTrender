#pragma once

#include <vector>
#include <string>
#include <unordered_map>

namespace dtbuild
{
	enum token_type: short
	{
		NO_TYPE,
		IDENTIFIER,
		NUM_LITERAL,
		STRING_LITERAL,
		CHAR_LITERAL,
		POUND_SIGN,
		DOLLAR_SIGN,
		QUESTION_MARK,
		TAB,
		SPACE,
		NEW_LINE,
		SQUOTE,
		DQUOTE,
		LPAREN,
		RPAREN,
		LBRACE,
		RBRACE,
		LBRACK,
		RBRACK,
		INT_TYPE,
		DOUBLE_TYPE,
		ALGORITHM_TYPE,
		INDICATOR_TYPE,
		RETURN,
		LANGBRACK,
		RANGBRACK,
		LINE_COMMENT,
		COMMENT_START,
		COMMENT_END,
		COMMA,
		PERIOD,
		SEMICOLON,
		COLON,
		AND,
		AND_COMP,
		OR,
		OR_COMP,
		PLUS,
		EQUALS_ASGN,
		EQUALS_COMP,
		NEQUALS_COMP,
		MINUS,
		ASTERISK,
		SLASH,
		MODULUS,
		NOT,
		TILDE,
		REQUIRE_PREPRO,
		INCLUDE_PREPRO,
		BUY_CALL,
		SELL_CALL,
		NOTHING_CALL
	};

	extern std::unordered_map<std::string, short> token_types;

	struct token
	{
		std::string value;
		short type = NO_TYPE;
		long line = 0;
		int column = 0;

		token() = default;
		token(const std::string& _value, short _type)
		{
			value = _value;
			type = _type;
		}
	};

	typedef std::vector<token> tokenlist;

	tokenlist lex(const std::string& src, const std::string& filepath);
}