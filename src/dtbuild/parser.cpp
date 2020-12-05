#include "parser.h"

#include "dtbuild.h"
#include "symbols.h"

#include <string>
#include <iostream>

namespace dtbuild
{
	namespace parser
	{
		typedef std::vector<std::vector<short>> deflist;
		std::unordered_map<short, deflist> grammar;
		// nothing that has a null ending type needs to have a grammar name
		std::unordered_map<short, std::string> ntnames;
/*
		std::unordered_map<short, bool (*)(Node&)> grammar_funcs;

		bool match_statement(Node& tree)
		{

		}
*/

		void init()
		{
			ntnames[PROGRAM] = "program";
			grammar[PROGRAM] =
			{
				{ FUNC, ELIPSIS }
			};

			ntnames[FUNC] = "function-def";
			grammar[FUNC] =
			{
				{ DECL_ID, LPAREN, ARGS, RPAREN, FUNCBDY }
			};

			ntnames[FUNC_CALL] = "function-call";
			grammar[FUNC_CALL] =
			{
				{ ID, LPAREN, LIST, RPAREN }
			};

			ntnames[ARGS] = "argument-list";
			grammar[ARGS] =
			{
				{ DECL_ID, COMMA_ELIPSIS }
			};

			ntnames[DECL_ID] = "declaration-identifier";
			grammar[DECL_ID] =
			{
				{ TYPE_INIT, ID }
			};

			ntnames[ID] = "identifier";
			grammar[ID] =
			{
				{ IDENTIFIER }
			};

			ntnames[STMT] = "statement";
			grammar[STMT] =
			{
				{ DECL_ID, STMT1, SEMICOLON },
				{ EXPR, STMT1, SEMICOLON }

			};

			ntnames[STMT1] = "statement-body";
			grammar[STMT1] =
			{
				{ OP, EXPR },
				{}
			};

			ntnames[STMTS] = "statements";
			grammar[STMTS] =
			{
				{ STMT, ELIPSIS }
			};

			ntnames[FUNCBDY] = "function-body";
			grammar[FUNCBDY] =
			{
				{ LBRACE, STMTS, RBRACE }
			};

			ntnames[OP] = "operator";
			grammar[OP] =
			{
				{ EQUALS_ASGN },
				{ PLUS },
				{ MINUS }
			};

			ntnames[EXPR] = "expression";
			grammar[EXPR] =
			{
				{ TERM, EXPR1 }
			};

			ntnames[LP_OP] = "operator";
			grammar[LP_OP] =
			{
				{ PLUS },
				{ MINUS }
			};

			ntnames[HP_OP] = "operator";
			grammar[HP_OP] =
			{
				{ ASTERISK },
				{ SLASH }
			};

			ntnames[EXPR1] = "expression-body";
			grammar[EXPR1] =
			{
				{ LP_OP, TERM, EXPR1 },
				{}
			};

			ntnames[TERM] = "term";
			grammar[TERM] = 
			{
				{ FACTOR, TERM1 }
			};

			ntnames[TERM1] = "term suffix";
			grammar[TERM1] =
			{
				{ HP_OP, TERM },
				{}
			};

			ntnames[FACTOR] = "factor";
			grammar[FACTOR] = 
			{
				{ FUNC_CALL },
				{ ID },
				{ LPAREN, EXPR, RPAREN },
				{ CONST }
			};

			ntnames[CONST] = "constant";
			grammar[CONST] =
			{
				{ NUM_LITERAL }
			};

			ntnames[TYPENAME] = "typename";
			grammar[TYPENAME] =
			{
				{ INT_TYPE },
				{ ALGORITHM_TYPE },
				{ INDICATOR_TYPE },
				{ IDENTIFIER }
			};

			ntnames[LIST] = "value-list";
			grammar[LIST] =
			{
				{ CONST, COMMA_ELIPSIS },
				{}
			};	

			ntnames[INIT_LIST] = "initializer-list";
			grammar[INIT_LIST] = 
			{
				{ LANGBRACK, LIST, RANGBRACK },
				{}
			};

			ntnames[TYPE_INIT] = "type-initializer";
			grammar[TYPE_INIT] =
			{
				{ TYPENAME, INIT_LIST }
			};

			// validating grammar and sorting defs
			for (short type = 1; type <= NT_CUTOFF; type++)
			{
				deflist& defs = grammar[type];
				if (defs.empty())
				{
					continue;
				}
				
				// list of index to push to end
				std::vector<int> empty_def_i;
				// first type of each definition in deflist
				std::vector<short> start_types;

				for (long i = 0; i < defs.size(); i++)
				{
					std::vector<short>& curr = defs[i];
					// empty definition (pass-all case) that is not at end
					if (curr.empty())
					{
						// flag it 
						empty_def_i.push_back(i);
						continue;
					}
					start_types.push_back(curr[0]);
				}

				bool dup = false;
				for (int i = 0; i < start_types.size(); i++)
				{
					for (int j = i + 1; j < start_types.size(); j++)
					{
						if (start_types[i] == start_types[j])
						{
							std::cout << "parser: duplicate grammar start types for non-terminal: " << type << std::endl;
							dup = true;
							break;
						}
					}
				}

				// clearing grammar if duplicate types are found
				// as grammar can not be correctly tested
				if (dup)
				{
					grammar.clear();
					return;
				}

				// eraseing all empty defs except for one and pushing it to the back
				for (int i = empty_def_i.size() - 1; i >= 0; i--)
				{
					int index = empty_def_i[i];
					// last/first empty def
					if (i == 0)
					{
						defs.push_back(defs[index]);
					}
					defs.erase(defs.begin() + index);
				}
			}

			std::cout << "Statement:\n";

			deflist d = grammar[STMT];
			for (long i = 0; i < d.size(); i++)
			{
				std::cout << i << ": " << d[i].size() << std::endl;;
			}
		}

		std::string tabs(int amt)
		{
			std::string out;
			for (int i = 0; i < amt; i++)
			{
				out += '\t';
			}
			return out;
		}

		int val()
		{
			return 1;
		}

		/*
		defs: deflist to compare against
		toks: token list being parsed
		index: index of the current token
		*/
		int matches_definition(short def_type, Node& tree, const lexer::tokenlist &toks, long index, const std::string& filepath, int depth = 0)
		{

			1 * val();

			int match_count = -1;

			const deflist& def_list = grammar[def_type];
			// for every definition of the non-terminal

			for (int j = 0; j < def_list.size(); j++)
			{
				const std::vector<short>& def = def_list[j];

				int matches = -1;

				if (def.empty()) 
				{
					if (match_count < 0) match_count = 0;
					matches = 0;
					std::cout << tabs(depth) << "empty def: match is true" << std::endl;
				}

				for (int i = 0; i < def.size(); i++)
				{
					
					bool is_elipsis = false;
					if (def[i] == ELIPSIS)
					{
						i--;
						is_elipsis = true;
					}
					if (def[i] == COMMA_ELIPSIS)
					{
						if (toks[index].type == COMMA)
						{
							is_elipsis = true;
							if (match_count < 0) match_count = 0;
							matches = 1;
							match_count++;
							index++;
							i--;
						}
						else
						{
							if (match_count < 0) match_count = 0;
							matches = 0;
							continue;
						}
					}
					short type = def[i];
					bool is_non_terminal = type <= NT_CUTOFF && type > NO_TYPE;

					std::cout << tabs(depth) << "@ TOK: " << toks[index].value << std::endl;
					
					// non-terminal
					if (is_non_terminal)
					{
						Node n;
						n.type = type;
						n.subtype = j;
						tree.args.push_back(n);

						std::cout << tabs(depth) <<  "Testing: \033[32m" << ntnames[def[i]] << "\033[0m\n";
						matches = matches_definition(type, tree.args.back(), toks, index, filepath, depth + 1);
					}
					// terminal
					else
					{
						if(def.size() == 1)
						{
							tree.subtype = toks[index].type;
							tree.value = toks[index].value;
						}

						std::cout << tabs(depth) << "Testing type: " << def[i] << std::endl;
						matches = (toks[index].type == type) * 2 - 1;
					}

					// list did not continue but it is not an error
					if (is_elipsis && matches < 0)
					{
						tree.args.pop_back();
						matches = 0;
						i++;
					}

					// did not match
					if (matches < 0)
					{
						if (is_non_terminal)
						{
							tree.args.pop_back();
						}
						else
						{
							tree.subtype = NO_TYPE;
							tree.value.clear();
						}

						std::cout << tabs(depth) << "does not match: index: " << i << "\n";
						match_count = -1;
						// did not match first item
						if (i == 0)
						{
							// try next def
							break;
						}
						else
						{
							i--;
							if (def[i] == ELIPSIS || def[i] == COMMA_ELIPSIS)
							{
								i--;
							}
							std::cout << tabs(depth) << "\033[31;1msyntax error!\033[0m:\texpected "
								<< ntnames[def[i]] << " before '" << toks[index].value << "' token\n";
							//syntax_error(filepath, "syntax error!", toks[index].line, toks[index].column, toks[index].value.size());
							return match_count;
						}
					}
					// matched type
					else
					{
						if (match_count < 0) match_count = 0;

						index += matches;
						match_count += matches;
						std::cout << tabs(depth) << "matches!: " << matches << "\n";

						//return total_match;
					}
					if (depth == 0) std::cout << "\n\n";
				}

				if (matches >= 0)
				{
					//std::cout << tabs(depth) << "matches: " << matches << std::endl:
					break;
				}
			}

			if (!tree.args.empty())
			{
				if (tree.args.back().empty())
				{
					tree.args.pop_back();
				}
			}

			/* 
				This version will pop anything in the args that is emptybut I think that is
				unnecessary becase no middle or first node should be empty, but we shall see.

			for (int i = tree.args.size() - 1; i >= 0; i--)
			{
				if (tree.args[i].args.empty() && tree.args[i].value.empty())
				{
					//tree.args.pop_back();
					tree.args.erase(tree.args.begin() + i);
				}
			}
			*/

			std::cout << tabs(depth) << "\033[33mRETURNING: \033[0m" << match_count << std::endl;
			return match_count;
		}

		Node parse(const lexer::tokenlist &toks, const std::string &filepath)
		{
			Node out;
			out.type = PROGRAM;

			if(grammar.empty())
			{
				std::cout << "parser: grammar is invalid, cannot continue with parsing\n";
				return out;
			}

			std::cout << "\nTokens: ";
			for (long i = 0; i < toks.size(); i++)
			{
				std::cout << toks[i].type << ' ';
			}

			std::cout << "\n\n**********************************************\nStarting parsing...\n**********************************************\n\n";

			int matches = matches_definition(PROGRAM, out, toks, 0, filepath);
			//matches = matches_definition(STMT, out, toks, matches, filepath);

			std::cout << "\n**********************************************\n";
			if (matches != toks.size())
			{
				std::cout << "FAILURE\n";
				std::cout << "toks: " << toks.size() << "\t\tmatches: " << matches << std::endl;
			}
			else
			{
				std::cout << "SUCCESS\n";
			}
			std::cout << "**********************************************\n";

			
			out.print();

			generate_code(out);

			return out;
		}
	}
}