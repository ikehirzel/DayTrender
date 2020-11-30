#pragma once

#include <vector>
#include "lexer.h"

/* BNF of DTScript:
 * 
 * PROG		::= FUNCS
 * 
 * FUNC		::= TYPE IDENT ( ARGLIST ) { STMTS }
 * FUNCS	::= FUNC | FUNC FUNCS
 * 
 * ARG		::= TYPE IDENT | TYPE < ARGLIST > IDENT
 * ARGLIST	::= ARG | ARG , ARGLIST | ""
 * 
 * STMT		::= EXPR OP EXPR ; | TYPENAME ID = EXPR ;| if ( EXPR ) STMT | { STMTLIST } | STMT STMT
 * 
 * EXPR		::= TERM +- EXPR | TERM
 * TERM		::= TERM /* FACTOR | FACTOR
 * FACTOR	::= ID | ( EXPR ) | CONST
 * CONST	::= NUM_LITERAL | CHAR_LITERAL | STRING_LITERAL
 */ 

namespace dtbuild
{
	namespace parser
	{
		enum nonterminal_type : short
		{
			PROGRAM = -6,
			FUNCTION,
			STATEMENT,
			EXPRESSION,
			TYPENAME,
			TERM,
			OPERATOR
		};

		inline int nt_to_index(short t)
		{
			return t + -PROGRAM;
		}

		typedef std::vector<std::vector<short>> deflist;
		typedef std::vector<deflist> Grammar;

		struct Node
		{
			short type;
			union value
			{
				std::vector<Node> args;
				token tok;
			};
		};

		struct Operator
		{
			static deflist defs;
		};

		struct Expression
		{
			static deflist defs;
			std::vector<token> args;
		};

		struct Statement
		{
			static deflist defs;
			std::vector<token> args;
		};

		struct Function
		{
			static deflist defs;
			std::vector<Statement> stmts;
		};

		struct Program
		{
			static deflist defs;
			std::vector<Function> funcs;
		};
		Grammar grammar;
		typedef bool(*check_func)(const tokenlist&, long);
		std::vector<check_func> checks;
		//bool is_program(const tokenlist& toks, long index);
		bool is_program(const tokenlist& toks, long index);
		bool is_statement(const tokenlist& toks, long index);
		//check_func is_statement();
		check_func is_expression();

		bool is_expression(short type);

		//deflist expression_defs, operator_defs;
		void init();
		Program parse(const tokenlist& toks, const std::string& filepath);
	}
}