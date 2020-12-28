#include "parser.h"

#include "dtbuild.h"
#include "symbols.h"

#include <string>
#include <iostream>

namespace dtbuild
{
	typedef std::vector<std::vector<short>> deflist;
	std::unordered_map<std::string, short> types;
	std::vector<std::string> ntnames;
	std::vector<deflist> grammar;

	void parse_init()
	{
		grammar.resize(NT_CUTOFF + 1);
		ntnames.resize(NT_CUTOFF + 1);

		#define def(t, n, ...) ntnames[t] = n; grammar[t] = { __VA_ARGS__ };
		#define list(...) { __VA_ARGS__ }

		/*
			Semantics:
				Optional and the elipses cannot be used with non-terminals
		*/

		def(PROGRAM, "program",
			list(PROG_ITEM, ELIPSIS))

		def(PROG_ITEM, "program-item",
			list(ALGO),
			list(FUNC),
			list(INDI))

		def(ALGO, "algorithm-def",
			list(ALGORITHM_TYPE, ID, LPAREN, ALGO_ARGS, RPAREN, LBRACE, STMTS, RBRACE))

		def(STMTS, "statements",
			list(STMT, ELIPSIS))

		def(INDI, "indicator-def",
			list(INDICATOR_TYPE, ID, LPAREN, ARGS, RPAREN, COMPOUND_STMT))

		def(FUNC, "function-def",
			list(DECLARATOR, LPAREN, ARGS, RPAREN, COMPOUND_STMT))
		
		def(ARGS, "argument-list",
			list(DECLARATOR, COMMA_ELIPSIS))
		
		def(ID, "identifier",
			list(IDENTIFIER))

		def(CONST, "constant",
			list(INT_LITERAL),
			list(FLOAT_LITERAL),
			list(STRING_LITERAL),
			list(CHAR_LITERAL))

		def(TYPENAME, "typename",
			list(INT_TYPE),
			list(DOUBLE_TYPE),
			list(IDENTIFIER))

		def(LIST, "value-list",
			list(EXPR, COMMA_ELIPSIS))

		def(INIT_LIST, "initializer-list",
			list(LANGBRACK, CONST, COMMA_ELIPSIS, RANGBRACK))
		
		def(INDI_INIT, "indicator-initializer",
			list(ID, INIT_LIST))

		def(ALGO_ARG, "argument",
			list(INDI_INIT, ID));

		def(ALGO_ARGS, "argument-list",
			list(ALGO_ARG, COMMA_ELIPSIS))

		
		//###########################################
		//                STATEMENTS                #
		//###########################################

		def(STMT, "statement",
			list(DECL_STMT),
			list(COMPOUND_STMT),
			list(SEL_STMT),
			list(ITER_STMT),
			list(JUMP_STMT),
			list(EXPR_STMT))

		def(COMPOUND_STMT, "compound-statement",
			list(LBRACE, STMT, ELIPSIS, RBRACE))
		
		def(SEL_STMT, "if-statement",
			list(IF_KWD, LPAREN, EXPR, RPAREN, STMT, OPTIONAL, ELSE_STMT))
		
		def(ELSE_STMT, "else-statment",
			list(ELSE_KWD, STMT))
		
		def(ITER_STMT, "iteration-statement",
			list(WHILE_STMT),
			list(FOR_STMT))

		def(EXPR_STMT, "expression-statement",
			list(OPTIONAL, EXPR, SEMICOLON))

		def(WHILE_STMT, "while-loop",
			list(WHILE_KWD, LPAREN, EXPR, RPAREN, STMT))

		def(FOR_STMT, "for-loop",
			list(FOR_KWD, LPAREN, DECL_STMT, EXPR, SEMICOLON, EXPR, RPAREN, STMT))

		def(JUMP_STMT, "jump-statement",
			list(RETURN, OPTIONAL, EXPR, SEMICOLON))

		def(DECL_STMT, "declaration-statement",
			list(DECLARATOR, OPTIONAL, INITIALIZER, SEMICOLON))

		def(DECLARATOR, "declarator",
			list(TYPENAME, ID))

		def(INITIALIZER, "initializer",
			list(EQUALS_ASGN, EXPR))


		//############################################
		//                EXPRESSIONS                #
		//############################################

		def(EXPR, "expression",
			list(ASGN_EXPR),
			list(COND_EXPR))

		def(ASGN_EXPR, "assignment-expression",
			list(UNARY_EXPR, ASGN_OP, EXPR))

		def(COND_EXPR, "condition-expression",
			list(OR_EXPR))

		def(OR_EXPR, "or-expression",
			list(AND_EXPR, OR_EXPR1, ELIPSIS))
			
		def(OR_EXPR1, "or-expression-body",
			list(OR_COMP, AND_EXPR))

		def(AND_EXPR, "and-expression",
			list(EQ_EXPR, AND_EXPR1, ELIPSIS))

		def(AND_EXPR1, "and-expression-body",
			list(AND_COMP, EQ_EXPR))

		def(EQ_EXPR, "equality-expression",
			list(REL_EXPR, EQ_EXPR1, ELIPSIS))

		def(EQ_EXPR1, "equality-expression-body",
			list(EQ_OP, REL_EXPR))

		def(REL_EXPR, "relational-expression",
			list(ADD_EXPR, REL_EXPR1, ELIPSIS))

		def(REL_EXPR1, "relational-expression-body",
			list(REL_OP, ADD_EXPR))

		def(ADD_EXPR, "additive-expression",
			list(MUL_EXPR, ADD_EXPR1, ELIPSIS))
		
		def(ADD_EXPR1, "additive-expression-body",
			list(ADD_OP, MUL_EXPR))

		def(MUL_EXPR, "multiplicitive-expression",
			list(CAST_EXPR, MUL_EXPR1, ELIPSIS))

		def(MUL_EXPR1, "multiplicitive-expression-body",
			list(MUL_OP, CAST_EXPR))

		def(CAST_EXPR, "cast-expression",
			list(OPTIONAL, CAST_OP, UNARY_EXPR))

		def(CAST_OP, "cast-operation",
			list(LPAREN, TYPENAME, RPAREN))

		def(UNARY_EXPR, "unary-expression",
			list(PRIM_EXPR, POSTFIX_EXPR),
			list(PREFIX_OP, UNARY_EXPR),
			list(UNARY_OP, CAST_EXPR))
		
		def(PRIM_EXPR, "primary-expression",
			list(ID),
			list(CONST),
			list(STRING_LITERAL),
			list(LPAREN, EXPR, RPAREN))

		def(POSTFIX_EXPR, "postfix-expression",
			list(POSTFIX_OP, ELIPSIS))



		//##########################################
		//                OPERATORS                #
		//##########################################

		def(ACCESSOR, "accessor",
			list(PERIOD, ID))

		def(PACCESSOR, "pointer-accessor",
			list(POINTER, ID))

		def(POSTFIX_OP, "postfix-operator",
			list(ACCESSOR),
			list(PACCESSOR),
			list(INDEX_OP),
			list(CALL_OP),
			list(INCREMENT),
			list(DECREMENT))

		def(PREFIX_OP, "prefix-operator",
			list(INCREMENT),
			list(DECREMENT))

		def(EQ_OP, "equality-operator",
			list(EQUALS_COMP),
			list(NEQUALS_COMP))

		def(ADD_OP, "additive-operator",
			list(PLUS),
			list(MINUS))

		def(MUL_OP, "multiplicitive-operator",
			list(ASTERISK),
			list(SLASH),
			list(MODULUS))

		def(ASGN_OP, "assignment-operator",
			list(EQUALS_ASGN),
			list(ADD_ASGN),
			list(SUB_ASGN),
			list(MUL_ASGN),
			list(DIV_ASGN),
			list(MOD_ASGN))
		
		def(UNARY_OP, "unary-operator",
			list(AND),
			list(ASTERISK),
			list(PLUS),
			list(MINUS),
			list(NOT))

		def(REL_OP, "relational-operator",
			list(RANGBRACK),
			list(LANGBRACK),
			list(LTOET),
			list(GTOET))

		def(CALL_OP, "call-operation",
			list(LPAREN, LIST, RPAREN))

		def(INDEX_OP, "index-operation",
			list(LBRACK, EXPR, RBRACK))



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

//#define DEBUG_OUTPUT

#define ELIPSIS_STATE	1
#define CELIPSIS_STATE	2
#define OPTIONAL_STATE	3

	int create_node(short def_type, Node& tree, const std::vector<Token> &toks, long index, std::vector<std::string>& errors, const std::string& filepath, int depth = 0)
	{
		int match_count = -1;
		int err_count = errors.size();
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
				int parse_state = 0;

				if (i + 1 < def.size())
				{
					if (def[i] == OPTIONAL)
					{
						parse_state = OPTIONAL_STATE;
						i++;
					}
					else if (def[i + 1] == ELIPSIS)
					{
						parse_state = ELIPSIS_STATE;
					}
					else if (def[i + 1] == COMMA_ELIPSIS)
					{
						parse_state = CELIPSIS_STATE;
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
					matches = create_node(type, tree.args.back(), toks, index, errors, filepath, depth + 1);
					
				}
				// terminal
				else
				{
					if(def.size() == 1)
					{
						tree.filepath = toks[index].filepath;
						tree.value = toks[index].value;
						tree.subtype = toks[index].type;
						tree.line = toks[index].line;
						tree.col = toks[index].col;
					}

					#ifdef DEBUG_OUTPUT
					std::cout << tabs(depth) << "\033[34mTesting type: " << def[i] << "\033[0m" << std::endl;
					#endif

					matches = (toks[index].type == type) * 2 - 1;
				}

				switch(parse_state)
				{
				case ELIPSIS_STATE:
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
					break;

				case CELIPSIS_STATE:
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
					break;

				case OPTIONAL_STATE:
					if (matches < 0)
					{
						tree.args.pop_back();
						matches = 0;
					}
					break;
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

						errors.push_back(syntax_error(toks[index].filepath, "syntax error! expected " + ntnames[def[i - offs]] +
							" before '" + toks[index].value + "' token", toks[index].line,
							toks[index].col, toks[index].value.size()));

						#ifdef DEBUG_OUTPUT
						std::cout << errors.back();
						#endif
						
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
				//std::cout << "deftype: " << def_type << std::endl;
				//std::cout << "I: " << i << std::endl;
				
				// this is necessary as elipsis matching can cause unsafe problems
				if (matches == toks.size()) break;
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
		
		if (match_count > 0)
		{
			//errors.resize(err_count);
		}

		return match_count;
	}

	Node parse(const std::vector<Token> &toks, const std::string &filepath)
	{
		Node out;
		out.type = PROGRAM;

		if(grammar.empty())
		{
			std::cout << "parser: grammar is invalid, cannot continue with parsing\n";
			return out;
		}

		std::cout << "\n\n";
		std::vector<std::string> errors;
		int matches = create_node(PROGRAM, out, toks, 0, errors, filepath);
		out.print();

		std::cout << "\n**********************************************\n";
		if (matches != toks.size())
		{
			std::cout << "FAILURE\n";
			std::cout << "toks: " << toks.size() << "\t\tmatches: " << matches << std::endl;
			out.args.clear();
			out.value.clear();
		}
		else
		{
			std::cout << "SUCCESS\n";
		}
		std::cout << "**********************************************\n\n";

		if (!errors.empty())
		{
			std::cout << errors[0] << std::endl;
		}

		return out;
	}
}