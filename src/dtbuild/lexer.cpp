#include "lexer.h"

#include "dtbuild.h"

#include <iostream>

#include <hirzel/strutil.h>


/*
	Giga chad version:

	all algorithms and indicators are compiled and they do not directly interact with each other
	TradeAlgorithms will handle the calls ( get the data from the indicators, pass the dataset to the algo)
	this saves on space a bunch and compile time because indicators do not get recompiled every time

	parsing will still happen for convenience they will all be able to be written like scripts but 
	c++ will handle the "int main (const indicator_dataset& dataset, algorithm_data& out)" stuff so that
	it doesn't get in the way of the main point: writing an algorithm

	trade algorithm 
	tradealgorithm passes candles into 


	indicator EMA (double ratio)
	algorithm Simple_MA (EMA<0.5> short, EMA<0.8> long)

*/


namespace dtbuild
{
	std::unordered_map<std::string, short> token_types;

	enum lex_states: short
	{
		NORMAL_MODE,
		IS_LCOMMENT,
		IS_BCOMMENT,
		IS_STR_LIT,
		IS_CHAR_LIT
	};

	tokenlist lex(const std::string& src, const std::string& filepath)
	{
		// TODO HANDLE LEADER KEY
		if (token_types.empty())
		{
			token_types =
			{
				{ "~",	TILDE 		},
				{ "!",	NOT			},
				{ "%",	MODULUS		},
				{ "/",	SLASH		},
				{ "*",	ASTERISK	},
				{ "+",	PLUS		},
				{ "-",	MINUS		},
				{ "<",	LANGBRACK	},
				{ ">",	RANGBRACK	},
				{ "[",	LBRACK		},
				{ "]",	RBRACK		},
				{ "{",	LBRACE		},
				{ "}",	RBRACE		},
				{ "(",	LPAREN		},
				{ ")",	RPAREN		},
				{ "\'",	SQUOTE		},
				{ "\"",	DQUOTE		},
				{ ",",	COMMA		},
				{ ".",	PERIOD		},
				{ ";",	SEMICOLON	},
				{ ":",	COLON		},
				{ "&",	AND			},
				{ "=",	EQUALS		},
				{ "|",	OR			},
				{ "#",	POUND_SIGN	},
				{ "$",	DOLLAR_SIGN	},
				{ "\n",	NEW_LINE	},
				{ "\t",	TAB			},
				{ " ",	SPACE		},
				{ "//",	LINE_COMMENT},
				{ "/*",	COMMENT_START},
				{ "*/",	COMMENT_END},
				{ "#include", INCLUDE_PREPRO },
				{ "#require", REQUIRE_PREPRO },
				{ "buy()", BUY_CALL },
				{ "sell()", SELL_CALL },
				{ "do_nothing()", NOTHING_CALL }
			};
		}

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
									toks[i].type = ERROR_TYPE;
									break;
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
									toks[i].type = ERROR_TYPE;
									break;
								}
							}
						}
					}
					break;
			}
		}

		long cur = 0;
		for (long i = 0; i < toks.size(); i++)
		{
			if (toks[i].line > cur)
			{
				//std::cout << '\n';
				cur = toks[i].line;
			}
			//std::cout << "[" << toks[i].line << "]: " <<  toks[i].value << ", col: " << toks[i].column << std::endl;
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
			bool error = false;
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
							syntax_error(filepath, "missing terminating " + tmp + " character", toks[ai].line,
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
						syntax_error(filepath, "stray '#' in program", toks[ei].line, toks[ei].column, 1);
						return {};
					}
					if (toks[ei].line != toks[ei + 1].line)
					{
						syntax_error(filepath, "stray '#' in program", toks[ei].line, toks[ei].column, 1);
						return {};
					}
					toks[ai].value += toks[ei + 1].value;
					std::cout << "toks ai : " << toks[ai].value << std::endl;
					toks[ai].type = token_types[toks[ai].value];
					std::cout << "toks ait: " << toks[ai].type << std::endl;
					if (toks[ai].type == NO_TYPE)
					{
						syntax_error(filepath, "invalid proprocessing directive '" + toks[ai].value + "'",
							toks[ai].line, toks[ai].column, toks[ai].value.size());
						return {};
					}
					ei++;
					break;

				case RPAREN:
					if (ai < 2) break;
					if (toks[ai - 1].type = LPAREN && toks[ai - 2].type == IDENTIFIER)
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

		std::cout << "\n*********************************\nBasic Reconstruction\n*********************************\n\n";

		short tabsLevel = 0;
		bool newline = false;
		long currline = 0;

		for (long i = 0; i < toks.size(); i++)
		{
			const std::string& t = toks[i].value;
			if (toks[i].line > currline)
			{
				std::cout << '\n';
				currline = toks[i].line;
			}
			
			std::cout << t << ' ';
			switch (toks[i].type)
			{
				case LBRACE:
					//std::cout << '\n';
					tabsLevel++;
					for (int i = 0; i < tabsLevel; i++) std::cout << '\t';
					break;

				case RBRACE:
					//std::cout << '\n';
					tabsLevel--;
					for (int i = 0; i < tabsLevel; i++) std::cout << '\t';
					break;

				default:
					break;
			}
		}

		std::cout << "\n\n*********************************\nEnd lexing phase...\n*********************************\n\n";

		return toks;
	}
}