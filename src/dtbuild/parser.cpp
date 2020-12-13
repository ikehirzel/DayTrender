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
		std::unordered_map<std::string, short> types;
		std::vector<std::string> ntnames;
		std::vector<deflist> grammar;

		void init()
		{
			grammar.resize(NT_CUTOFF + 1);
			ntnames.resize(NT_CUTOFF + 1);

			#define def(t, n, ...) ntnames[t] = n; grammar[t] = { __VA_ARGS__ };
			#define list(...) { __VA_ARGS__ }
			#define nodef {}

			def(PROGRAM, "program",
				list(PROG_ITEM, ELIPSIS))

			def(PROG_ITEM, "program-item",
				list(ALGO),
				list(FUNC),
				list(INDI))

			def(ALGO, "algorithm-def",
				list(ALGORITHM_TYPE, ID, LPAREN, ALGO_ARGS, RPAREN, FUNCBDY))

			def(INDI, "indicator-def",
				list(INDICATOR_TYPE, ID, LPAREN, ARGS, RPAREN, FUNCBDY))

			def(FUNC, "function-def",
				list(DECL_ID, LPAREN, ARGS, RPAREN, FUNCBDY))

			def(FUNCBDY, "function-body",
				list(LBRACE, STMT, ELIPSIS, RBRACE))
			
			def(FUNC_CALL, "function-call",
				list(ID, LPAREN, LIST, RPAREN))
			
			def(ARGS, "argument-list",
				list(DECL_ID, COMMA_ELIPSIS))
			
			def(DECL_ID, "identifier-declaration",
				list(TYPENAME, ID))
			
			def(ID, "identifier",
				list(IDENTIFIER))

			def(COMPOUND_STMT, "compound-statement",
				list(LBRACE, STMT, ELIPSIS, RBRACE))

			def(STMT, "statement",
				list(DECLARATION),
				list(EXPR_STMT),
				list(IF_STMT),
				list(COMPOUND_STMT))

			def(STMT1, "statement-body",
				list(OP, EXPR),
				nodef)
			
			def(EXPR_STMT, "expression-statement",
				list(EXPR, STMT1, SEMICOLON))

			def(IF_STMT, "if-statement",
				list(IF_KWD, LPAREN, EXPR, RPAREN, STMT))

			def(FOR_STMT, "for-loop",
				list(FOR_KWD, LPAREN, EXPR, RPAREN, STMT))

			def(WHILE_STMT, "while-loop",
				list(WHILE_KWD, LPAREN, EXPR, RPAREN, STMT))

			def(OP, "operator",
				list(EQUALS_ASGN),
				list(PLUS),
				list(MINUS))

			def(EXPR, "epxression",
				list(TERM, EXPR1))
			
			def(DECLARATION, "declaration",
				list(DECL_ID, INITIALIZER, SEMICOLON))
			
			def(INITIALIZER, "initializer",
				list(EQUALS_ASGN, EXPR),
				nodef)

			def(LP_OP, "low-priority-operator",
				list(PLUS),
				list(MINUS))

			def(HP_OP, "high-priority-operator",
				list(ASTERISK),
				list(SLASH))
			
			def(EXPR1, "expression-body",
				list(LP_OP, TERM, EXPR1),
				nodef)

			def(TERM, "term",
				list(FACTOR, TERM1))

			def(TERM1, "term-body",
				list(HP_OP, TERM),
				nodef)

			def(FACTOR, "factor",
				list(ID, FACTOR1),
				list(LPAREN, EXPR, RPAREN),
				list(CONST),
				list(FUNC_CALL))

			def(FACTOR1, "factor-body",
				list(INDEX),
				list(PERIOD, FUNC_CALL),
				nodef)

			def(CONST, "constant",
				list(NUM_LITERAL))

			def(TYPENAME, "typename",
				list(INT_TYPE),
				list(IDENTIFIER))

			def(LIST, "value-list",
				list(CONST, COMMA_ELIPSIS),
				nodef)

			def(INIT_LIST, "initializer-list",
				list(LANGBRACK, LIST, RANGBRACK),
				nodef)
			
			def(INDI_INIT, "indicator-initializer",
				list(ID, INIT_LIST))

			def(ALGO_ARG, "argument",
				list(INDI_INIT, ID));
				//list(ALGO_ARG, COMMA_ELIPSIS))

			def(ALGO_ARGS, "argument-list",
				list(ALGO_ARG, COMMA_ELIPSIS))

			def(INDEX, "index-operation",
				list(LBRACK, EXPR, RBRACK))

			def(THIS_STMT, "this-statement",
				list(INDEX))

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

//#define DEBUG_OUTPUT

		int create_node(short def_type, Node& tree, const lexer::tokenlist &toks, long index, const std::string& filepath, int depth = 0)
		{
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
					#ifdef DEBUG_OUTPUT
					std::cout << tabs(depth) << "empty def: match is true" << std::endl;
					#endif
				}

				for (int i = 0; i < def.size(); i++)
				{
					
					int is_elipsis = 0;
					if (i + 1 < def.size())
					{
						if (def[i + 1] == ELIPSIS)
						{
							is_elipsis = 1;
						}
						else if (def[i + 1] == COMMA_ELIPSIS)
						{
							is_elipsis = 2;
						}
					}

					short type = def[i];
					bool is_non_terminal = type <= NT_CUTOFF && type > NO_TYPE;
					
					#ifdef DEBUG_OUTPUT
					std::cout << tabs(depth) << "@ TOK: " << toks[index].value << std::endl;
					#endif
					// non-terminal
					if (is_non_terminal)
					{
						Node n;
						n.type = type;
						n.subtype = j;
						tree.args.push_back(n);

						#ifdef DEBUG_OUTPUT
						std::cout << tabs(depth) <<  "[" << type << "] Testing: \033[32m" << ntnames[def[i]] << "\033[0m\n";
						#endif
						matches = create_node(type, tree.args.back(), toks, index, filepath, depth + 1);
					}
					// terminal
					else
					{
						if(def.size() == 1)
						{
							tree.subtype = toks[index].type;
							tree.value = toks[index].value;
						}

						#ifdef DEBUG_OUTPUT
						std::cout << tabs(depth) << "Testing type: " << def[i] << std::endl;
						#endif

						matches = (toks[index].type == type) * 2 - 1;
					}

					// list did not continue but it is not an error
					if (is_elipsis == 1)
					{
						if (matches < 0)
						{
							tree.args.pop_back();
							matches = 0;
							i++;
						}
						else
						{
							i--;
						}

					}
					else if (is_elipsis == 2)
					{
						if (matches < 0)
						{
							tree.args.pop_back();
							matches = 0;
							i++;
						}
						// matched def
						else if (matches > 0)
						{
							if (toks[index + matches].type == COMMA)
							{
								matches++;
								//test same def again
								i--;
							}
							else
							{
								// test next def
								i++;
							}
						}
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

						#ifdef DEBUG_OUTPUT
						std::cout << tabs(depth) << "does not match: index: " << i << "\n";
						#endif

						match_count = -1;

						// did not match first item
						if (i == 0)
						{
							// try next def
							break;
						}
						// did not match second or after element
						else
						{
							int offs = 0;
							if (def[i - offs] > NT_CUTOFF)
							{
								offs++;
								if (def[i -offs] == ELIPSIS || def[i -offs] == COMMA_ELIPSIS)
								{
									offs++;
								}
							}
							std::cout << "Def Type: " << def_type << std::endl;
							syntax_error(filepath, "syntax error! expected " + ntnames[def[i - offs]] +
								" before '" + toks[index].value + "' token", toks[index].line,
								toks[index].column, toks[index].value.size());

							//return match_count;
							return -1;
						}
					}
					// matched type
					else
					{
						if (match_count < 0) match_count = 0;
						index += matches;
						match_count += matches;

						#ifdef DEBUG_OUTPUT
						std::cout << tabs(depth) << "matches!: " << matches << "\n";
						#endif
					}
					#ifdef DEBUG_OUTPUT
					if (depth == 0) std::cout << "\n\n";
					#endif
				}

				if (matches >= 0)
				{
					#ifdef DEBUG_OUTPUT
					std::cout << tabs(depth) << "matches: " << matches << std::endl;
					#endif
					break;
				}
			}
			#ifdef DEBUG_OUTPUT
			std::cout << tabs(depth) << "\033[33mRETURNING: \033[0m" << match_count << std::endl;
			#endif
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

			std::cout << "Tokens: ";
			for (long i = 0; i < toks.size(); i++)
			{
				std::cout << toks[i].type << ' ';
			}

			std::cout << "\n\n";

			int matches = create_node(PROGRAM, out, toks, 0, filepath);
			//matches = create_node(STMT, out, toks, matches, filepath);

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
			std::cout << "**********************************************\n\n";

			
			out.print();

			return out;
		}
	}
}