#include "lexer.h"

#include <hirzel/strutil.h>

#include <iostream>

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


namespace daytrender
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

	tokenlist lex(const std::string& src)
	{
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
			};
		}

		using namespace hirzel;

		long offset = 0;
		long line = 1;
		int col = 1;

		long tmp_line = 0;
		int tmp_col = 0;
		std::string tmp;
		std::vector<std::string> str_toks;
		tokenlist toks;

		str_toks = str::tokenize(src, " \"\'\t\n?$#(){}[]<>,.;:&+=-*/%!~", true, true);
		toks.resize(str_toks.size());

		/*******************************************
		 *   Creating list of tokens from string   *
		 *******************************************/
		int state = NORMAL_MODE;
		short quote_type;
		for (long i = 0; i < toks.size(); i++)
		{
			const std::string& s = str_toks[i];

			toks[i].value = s;
			toks[i].type = token_types[s];
			toks[i].column = col;
			toks[i].line = line;

			toks[i - offset] = toks[i];

			// handling the state specific behaviours
			switch (state)
			{
				case IS_LCOMMENT:
					// this is the correct ordering
					if (toks[i].type == SPACE || toks[i].type == TAB) offset--;
					offset ++;
					if (toks[i].type == NEW_LINE)
					//if (toks[i].line > toks[i - 1].line)
					{
						state = NORMAL_MODE;
					}
					break;

				case IS_BCOMMENT:
					// this is the correct ordering
					if (toks[i].type == SPACE || toks[i].type == TAB || toks[i].type == NEW_LINE)
					{
						offset--;
					}
					offset ++;
					if (toks[i].type == SLASH && toks[i - 1].type == ASTERISK)
					{
						offset++;
						state = NORMAL_MODE;
					}
					break;

				// does not handle \" or \'
				case IS_STR_LIT:
					if (!tmp_col) tmp_col = toks[i].column;
					if (!tmp_line) tmp_line = toks[i].line;
					if (toks[i].type == quote_type)
					{
						offset++;
						std::cout << "VAL: " << tmp << std::endl;
						state = NORMAL_MODE;
						toks[i - offset].value = tmp;
						toks[i - offset].type = STRING_LITERAL;
						toks[i - offset].column = tmp_col;
						toks[i - offset].line = tmp_line;
						tmp_col = 0;
						tmp_line = 0;
						tmp.clear();
						break;
					}
					if (toks[i].type == NEW_LINE)
					{
						state = NORMAL_MODE;
						tmp_col = 0;
						tmp_line = 0;
						tmp.clear();
						break;
					}
					tmp += toks[i].value;
					if (toks[i].type == SPACE || toks[i].type == TAB)
					{
						 offset--;
					}
					offset++;
					break;
			}

			switch (toks[i].type)
			{
				case NEW_LINE:
					col = 0;
					line++;
				case TAB:
				case SPACE:
					col++;
					offset++;
					break;

				default:
					col += toks[i].value.size();
					break;
			}

			// handling token type WARNING DO NOT USE CONTINUE
			switch (toks[i].type)
			{
				case NO_TYPE:
					// testing for identifier
					if (str::is_alpha(s[0]) || s[0] == '_')
					{
						toks[i].type = IDENTIFIER;
						for (int c = 0; c < s.size(); c++)
						{
							if (!str::is_digit(s[c]) && !str::is_alpha(s[c]) && s[c] != '_')
							{
								toks[i].type = ERROR_TYPE;
								break;
							}
						}
					}
					else if (str::is_digit(s[0]))
					{
						toks[i].type = NUM_LITERAL;
						for (int c = 0; c < s.size(); c++)
						{
							if (!str::is_digit(s[c]))
							{
								toks[i].type = ERROR_TYPE;
								break;
							}
						}
						
						if (i < 1) break;
						if (toks[i - 1].type == PERIOD)
						{
							std::cout << "Should combine " << toks[i].value << " with '.'\n";
							offset++;
							toks[i].line = toks[i - 1].line;
							toks[i].column = toks[i - 1].column;
							toks[i].value.insert(0, 1, '.');
						}

						if (i < 2) break;
						if (toks[i - 2].type == NUM_LITERAL)
						{
							offset++;
							toks[i].line = toks[i - 2].line;
							toks[i].column = toks[i - 2].column;
							toks[i].value.insert(0, toks[i - 2].value);
						}
						toks[i - offset] = toks[i];
					}
					break;

				case SQUOTE:
				case DQUOTE:
					quote_type = toks[i].type;
					state = IS_STR_LIT;
					offset--;
					tmp.clear();
					break;

				case SLASH:
					if (i < 1) break;
					if (toks[i - 1].type == SLASH)
					{
						state = IS_LCOMMENT;
						offset ++;
					}
					break;
				
				case ASTERISK:
					if (i < 1) break;
					if (toks[i - 1].type == SLASH)
					{
						offset ++;
						state = IS_BCOMMENT;
					}
					break;

				case AND:
					if (i < 1) break;
					if (toks[i - 1].type == AND)
					{
						toks[i].type = AND_COMP;
						toks[i].value += toks[i - 1].value;
						toks[i].line = toks[i - 1].line;
						toks[i].column = toks[i - 1].column;
						offset++;
						toks[i - offset] = toks[i];
					}
				break;
			}
		}
		std::cout << "\n\n";
		toks.resize(toks.size() - offset);
		long currline = 0;
		
		long samt = 12;
		for (long i = 0; i < toks.size(); i++)
		{
			if (toks[i].line > currline)
			{
				std::cout << "\n";
				currline = toks[i].line;
			}
			std::cout << "[" << toks[i].line << "]: " << toks[i].value;
			for (int a = 0; a < 10 - toks[i].value.size(); a++)
			{
				std::cout << ' ';
			}
			std::cout << "col: " << toks[i].column << std::endl;
		}
		

		std::cout << "Basic reconstruction:\n\n";
		short tabsLevel = 0;
		bool newline = false;
		currline = 1;
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

		std::cout << "\n\nEnd lexing phase...\n";

		return toks;
	}
}