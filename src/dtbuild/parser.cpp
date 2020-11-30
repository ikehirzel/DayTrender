#include "parser.h"

#include "dtbuild.h"

#include <string>

namespace dtbuild
{
	namespace parser
	{
		void init()
		{
			grammar.resize(-PROGRAM);
			grammar[nt_to_index(STATEMENT)] =
			{
				{ TYPENAME, IDENTIFIER, OPERATOR, NUM_LITERAL, SEMICOLON }
			};

			grammar[nt_to_index(TYPENAME)] =
			{
				{ INT_TYPE }, 
			};

			grammar[nt_to_index(OPERATOR)] =
			{
				{ EQUALS_ASGN }
			};
		}

		/*
		defs: deflist to compare against
		toks: token list being parsed
		index: index of the current token
		*/
		bool match_deflist(const deflist &defs, const tokenlist &toks, long index)
		{
			// for every definition in the list
			for (long i = 0; i < defs.size(); i++)
			{
				// check if it matches the first term
				short type = defs[i][0];
				bool m = false;
				if (type >= 0)
				{
					m = match_deflist(grammar[nt_to_index(type)], toks, index);
				}
				else
				{
					m = (type == toks[index].type);
				}

				if (m)
				{
					if (defs[i].size() > 1)
					{
						for (int j = 1; j < defs[i].size(); j++)
						{
							if (!match_deflist(defs[i][j], toks[index]);
						}
					}
					else
					{
						return true;
					}
				}
				else
				{
					
				}
			}
			return false;
		}

		Program parse(const tokenlist &toks, const std::string &filepath)
		{
			Program out;

			for (long i = 0; i < toks.size(); i++)
			{
				int match = match_deflist(Statement::defs, toks, 0);
				if (match >= 0)
				{
				}
				else
				{
					syntax_error(filepath, "syntax error", toks[i].line, toks[i].column, 0);
				}
			}

			return out;
		}
	}
}