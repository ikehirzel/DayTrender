#include "lexer.h"

#include "dtbuild.h"

#include <iostream>

#include <hirzel/strutil.h>

namespace dtbuild
{
	namespace lexer
	{
		std::unordered_map<std::string, short> token_types;

		void init()
		{
			token_types =
				{
					// separators
					{ "[",	LBRACK		},
					{ "]",	RBRACK		},
					{ "{",	LBRACE		},
					{ "}",	RBRACE		},
					{ "(",	LPAREN		},
					{ ")",	RPAREN		},

					// arithmetic operators
					{ "!",	NOT			},
					{ "%",	MODULUS		},
					{ "/",	SLASH		},
					{ "*",	ASTERISK	},
					{ "+",	PLUS		},
					{ "-",	MINUS		},
					{ "<",	LANGBRACK	},
					{ ">",	RANGBRACK	},
					{ ".",	PERIOD		},
					{ ";",	SEMICOLON	},
					{ "&",	AND			},
					{ "|",	OR			},
					{ "++",	INC			},
					{ "--",	DEC			},

					// asign operators
					{ "=",	EQUALS_ASGN	},
					{ "+=",	ADD_ASGN	},
					{ "-=",	SUB_ASGN	},
					{ "*=",	MUL_ASGN	},
					{ "/=",	DIV_ASGN	},
					{ "%=",	MOD_ASGN	},

					// comp operators
					{ "==",	EQUALS_COMP	},
					{ "!=",	NEQUALS_COMP},
					{ "&&",	AND_COMP	},
					{ "||",	OR_COMP		},
					{ ">=",	GTOET		},
					{ "<=",	LTOET		},

					// generic types
					{ "~",	TILDE 		},
					{ ":",	COLON		},
					{ "\'",	SQUOTE		},
					{ "\"",	DQUOTE		},
					{ ",",	COMMA		},
					{ "#",	POUND_SIGN	},
					{ "$",	DOLLAR_SIGN	},
					{ "\n",	NEW_LINE	},
					{ "\t",	TAB			},
					{ " ",	SPACE		},
					{ "^", 	XOR			},

					// comments
					{ "//",	LINE_COMMENT},
					{ "/*",	COMMENT_START},
					{ "*/",	COMMENT_END},

					// keywords
					{ "return", RETURN },
					{ "break", BREAK },
					{ "int", INT_TYPE },
					{ "double", DOUBLE_TYPE },
					{ "algorithm", ALGORITHM_TYPE },
					{ "indicator", INDICATOR_TYPE },
					{ "if", IF_KWD 		},
					{ "else", ELSE_KWD 	},
					{ "while", WHILE_KWD },
					{ "for", FOR_KWD },
					{ "#include", INCLUDE_PREPRO },
					{ "#require", REQUIRE_PREPRO },
				};
		}

		tokenlist lex(const std::string& src, const std::string& filepath)
		{
			using namespace hirzel;

			std::vector<std::string> str_toks;
			tokenlist toks;

			str_toks = str::tokenize(src, " \"\'\t\n?$#(){}[]<>,.;:&+=-*/%!~", true, true);
			toks.resize(str_toks.size());

			/*******************************************
			 *   Creating list of tokens from string   *
			 *******************************************/

			long line = 1;
			int col = 1;
			// used to see what the last meaningful type is
			for (long i = 0; i < str_toks.size(); i++)
			{
				toks[i].value = str_toks[i];
				toks[i].type = token_types[toks[i].value];
				toks[i].line = line;
				toks[i].column = col;

				switch (toks[i].type)
				{
					case NEW_LINE:
						col = 0;
						line++;
					case TAB:
					case SPACE:
						col++;
						break;

					default:
						col += toks[i].value.size();
						if (toks[i].type == NO_TYPE)
						{
							if (str::is_alpha(toks[i].value[0]) || toks[i].value[0] == '_')
							{
								toks[i].type = IDENTIFIER;
								for (int c = 0; c < toks[i].value.size(); c++)
								{
									if (!str::is_digit(toks[i].value[c]) && !str::is_alpha(toks[i].value[c]) && toks[i].value[c] != '_')
									{
										std::cout << syntax_error(filepath, "invalid character in identifier name", toks[i].line, toks[i].column, toks[i].value.size());
										return {};
									}
								}
							}
							else if (str::is_digit(toks[i].value[0]))
							{
								toks[i].type = NUM_LITERAL;
								for (int c = 0; c < toks[i].value.size(); c++)
								{
									if (!str::is_digit(toks[i].value[c]))
									{
										std::cout << syntax_error(filepath, "invalid character in number literal", toks[i].line, toks[i].column, toks[i].value.size());
										return {};
									}
								}
							}
						}
						break;
				}
			}

			/*********************************
			 *   Coalescing related tokens   *
			 *********************************/

			long ai = 0;
			std::string tmp;
			short tmp_type = NO_TYPE;
			for (long ei = 0; ei < toks.size(); ei++)
			{
				toks[ai] = toks[ei];
				short tmp_type = 0, tmp_end = 0;
				switch (toks[ai].type)
				{
					case NUM_LITERAL:
						if (toks[ai - 1].type == PERIOD)
						{
							toks[ai - 1].type = NUM_LITERAL;
						}

						for (int n = 0; n < 2; n++)
						{
							if (ai < 1) break;
							if (toks[ai - 1].type == NUM_LITERAL)
							{
								ai--;
								toks[ai].value += toks[ai + 1].value;
							}
						}
						break;

					case SLASH:
						// line comment of end comment
						if (toks[ai - 1].type == SLASH || toks[ai - 1].type == ASTERISK)
						{
							ai--;
							toks[ai].value += toks[ai + 1].value;
							toks[ai].type = token_types[toks[ai].value];
						}
						break;

					case ASTERISK:
						// start comment
						if (toks[ai - 1].type == SLASH)
						{
							ai--;
							toks[ai].value += toks[ai + 1].value;
							toks[ai].type = token_types[toks[ai].value];
						}
						break;

					case SQUOTE:
						tmp_type = CHAR_LITERAL;
						tmp_end = SQUOTE;
					case DQUOTE:
						if (!tmp_type) tmp_type = STRING_LITERAL;
						if (!tmp_end) tmp_end = DQUOTE;

						tmp = toks[ei].value;

						toks[ai].type = tmp_type;
						toks[ai].value.clear();
						toks[ai].column++;

						ei++;

						while (ei < toks.size())
						{
							if (toks[ei].type == tmp_end)
							{
								break;
							}
							else if (toks[ei].type == NEW_LINE)
							{
								std::cout << syntax_error(filepath, "missing terminating " + tmp + " character", toks[ai].line,
									toks[ai].column -1, 0);
								return {};
							}

							toks[ai].value += toks[ei].value;

							ei++;
						}
						break;

					case POUND_SIGN:
						// these checks are broken up into two so that there is no accidental seg faults
						if (ei + 1 >= toks.size())
						{
							std::cout << syntax_error(filepath, "stray '#' in program", toks[ei].line, toks[ei].column, 1);
							return {};
						}
						if (toks[ei].line != toks[ei + 1].line)
						{
							std::cout << syntax_error(filepath, "stray '#' in program", toks[ei].line, toks[ei].column, 1);
							return {};
						}
						ei++;
						toks[ai].value += toks[ei].value;
						toks[ai].type = token_types[toks[ai].value];
						if (toks[ai].type == NO_TYPE)
						{
							std::cout << syntax_error(filepath, "invalid proprocessing directive '" + toks[ai].value + "'",
								toks[ai].line, toks[ai].column, toks[ai].value.size());
							return {};
						}
						break;

					// case LPAREN:
					// 	std::cout << ai << ": " << toks[ai].value << std::endl;
					// 	break;

					case RPAREN:
						if (ai < 2) break;
						if (toks[ai - 1].type == LPAREN && toks[ai - 2].type == IDENTIFIER)
						{
							tmp = toks[ai - 2].value + toks[ai - 1].value + toks[ai].value;
							tmp_type = token_types[tmp];
							if (tmp_type != NO_TYPE)
							{
								ai -= 2;
								toks[ai].type = tmp_type;
								toks[ai].value = tmp;
							}
						}
						break;

					case EQUALS_ASGN:
						if (ai < 1) break;
						tmp_type = NO_TYPE;

						switch (toks[ai - 1].type)
						{
						case EQUALS_ASGN:
						case PLUS:
						case MINUS:
						case NOT:
							tmp_type = toks[ai - 1].type;
							break;
						}

						if (tmp_type > NO_TYPE)
						{
							ai--;
							toks[ai].value += toks[ai + 1].value;
							toks[ai].type = token_types[toks[ai].value];
						}
						break;
				}
				ai++;
			}

			toks.resize(ai);

			/*****************************************
			 *   Removing blank space and comments   *
			 *****************************************/

			ai = 0;
			for (long ei = 0; ei < toks.size(); ei++)
			{
				short cmt_end_type = 0;
				toks[ai] = toks[ei];
				switch (toks[ai].type)
				{
					case SPACE:
					case TAB:
					case NEW_LINE:
						ai--;
						break;

					case LINE_COMMENT:
						cmt_end_type = NEW_LINE;
					case COMMENT_START:
						if(!cmt_end_type) cmt_end_type = COMMENT_END;
						ai--;
						while (ei < toks.size())
						{
							if (toks[ei].type == cmt_end_type)
							{
								break;
							}
							ei++;
						}
						break;
				}
				ai++;
			}

			toks.resize(ai);

			return toks;
		}
	}
}
