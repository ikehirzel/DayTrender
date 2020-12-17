#pragma once

#include "lexer.h"

#include <vector>
#include <unordered_map>
#include <iostream>

/* 
 * BNF of DTScript:
 * 
 * semantics of the grammar:
 * definitions in the same list cannot start with the same type ( TERM + EXPR | TERM - EXPR )
 * as this will make for indeterminate behaviour that will cause premature aborts
 * comparisons must follow an equivalent till proven not style to allow for empty defs to work
 * 
 * PROG			::= FUNCS
 * 
 * FUNC			::= TYPE IDENT ( ARGLIST ) STMTS
 * FUNCS		::= FUNC ...
 * 
 * ARG			::= TYPE IDENT
 * ARGS			::= ARG , ...
 * 
 * EXPRLIST		::= EXPR , ...
 * 
 * DECL_STMT	::= TYPENAME IDENTIFIER DECL_CLAUSE
 * DECL_CLAUSE	::= '=' DEFINITION
 * DEFINITION	::= { }
 * 
 * IF_STMT		::=	if ( EXPR ) STMTBDY ELSE
 * ELSE			::= else ELSEBDY | ""
 * ELSEBDY		::= IFSTMT | STMTBDY
 * 	
 * STMT			::= DECL_STMT ; | | CALL_STMT ; | IF_STMT
 * ASGN_STMT	::= INIT_ID 
 * STMT1		::= OP EXPR | ""
 * STMTS		::= STMT STMTS1
 * STMTS1		::= STMTS | ""
 * STMTBDY		::= { STMTS } | STMT
 * 
 * OP			::= EQUALS_ASGN
 * 
 * EXPR			::= TERM EXPR1
 * EXPR1		::= + TERM EXPR1 | - TERM EXPR1 | ""
 * TERM			::= FACTOR TERM1
 * TERM1		::= * FACTOR TERM1 | / FACTOR TERM1 | ""
 * FACTOR		::= SIGN ID | ( EXPR ) | CONST
 * SIGN			::= + | - | ""
 * CONST		::= NUM_LITERAL | CHAR_LITERAL | STRING_LITERAL
 * 
 */


namespace dtbuild
{
	namespace parser
	{
		extern std::vector<std::string> ntnames;

		struct Node
		{
			short type = NO_TYPE;  
			short subtype = NO_TYPE;
			std::string value;
			std::vector<Node> args;

			void print(int depth = 0)
			{
				for (int i = 0; i < depth; i++) std::cout << "|\t";
				std::cout << ntnames[type];

				if (!value.empty())
				{
					std::cout << ": '" << value << "'";
				}
				std::cout << "\n";

				for (int j = 0; j < args.size(); j++)
				{
					args[j].print(depth + 1);
				}
			}

			bool empty() const
			{
				return value.empty() && args.empty();
			}
		};

		void init();
		Node parse(const lexer::tokenlist& toks, const std::string& filepath);
	}
}