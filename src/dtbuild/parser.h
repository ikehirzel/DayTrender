#pragma once

#include <vector>

namespace dtbuild
{
	typedef std::vector<std::vector<short>> deflist;
	struct Operator
	{
		short type;
	};

	struct Expression
	{
		short type;
	};

	struct Statement
	{

	};

	struct Program
	{
		std::vector<Statement> stmts;
	};

	struct AST
	{
		Program prog;
	};

	bool is_expression(short type);
	bool is_statement(short type);

	deflist expression_defs, operator_defs;

	AST parse(const tokenlist& toks);
}