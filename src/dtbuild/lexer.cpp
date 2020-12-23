#include "lexer.h"

#include "dtbuild.h"

#include <iostream>

#include <hirzel/strutil.h>

namespace dtbuild
{
	namespace lexer
	{
		char char_types[256];

		std::unordered_map<std::string, short> token_types;

		void init()
		{
			// setting name chars
			char_types['_'] = CHAR_NAME;
			for (int i = 'A'; i <= 'Z'; i++) char_types[i] = CHAR_NAME;
			for (int i = 'a'; i <= 'z'; i++) char_types[i] = CHAR_NAME;
			
			
			// setting digit chars

			for (int i = '0'; i <= '9'; i++) char_types[i] = CHAR_DIGIT;

			// setting separator types

			char_types['('] = CHAR_SEP;
			char_types[')'] = CHAR_SEP;
			char_types['['] = CHAR_SEP;
			char_types[']'] = CHAR_SEP;
			char_types['{'] = CHAR_SEP;
			char_types['}'] = CHAR_SEP;
			char_types[';'] = CHAR_SEP;
			char_types[','] = CHAR_SEP;
			char_types['.'] = CHAR_SEP;

			// setting operator types
			
			char_types['!'] = CHAR_OP;
			char_types['$'] = CHAR_OP;
			char_types['%'] = CHAR_OP;
			char_types['?'] = CHAR_OP;
			char_types['^'] = CHAR_OP;
			char_types['&'] = CHAR_OP;
			char_types['*'] = CHAR_OP;
			char_types['/'] = CHAR_OP;
			char_types['+'] = CHAR_OP;
			char_types['-'] = CHAR_OP;
			char_types['='] = CHAR_OP;
			char_types['<'] = CHAR_OP;
			char_types['>'] = CHAR_OP;

			// setting literal types

			char_types['\"'] = CHAR_LIT;
			char_types['\''] = CHAR_LIT;


			// setting the rest of the visible chars to invalid

			for (int i = 33; i < 256; i++)
			{
				if (char_types[i] == CHAR_NOTYPE)
				{
					char_types[i] = CHAR_INVALID;
				}
			}

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
					{ ";",	SEMICOLON	},
					{ "++",	INCREMENT	},
					{ "--",	DECREMENT	},

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
					{ ".",	PERIOD		},
					{ "->",	POINTER		},
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

					// jump keywords
					{ "return", RETURN },
					{ "break", BREAK },

					// typenames
					{ "int", INT_TYPE },
					{ "double", DOUBLE_TYPE },
					{ "algorithm", ALGORITHM_TYPE },
					{ "indicator", INDICATOR_TYPE },

					// stmt keywords
					{ "if", IF_KWD 		},
					{ "else", ELSE_KWD 	},
					{ "while", WHILE_KWD },
					{ "for", FOR_KWD },
				};
		}

		std::vector<Token> lex(const std::string&str, const std::string& filepath)
		{
			using namespace hirzel;

			std::vector<Token> toks;
			long line = 1;
			int col = 1;
			size_t i = 0;
			while (i < str.size())
			{
				Token tok;
				std::string tmp;
				char char_type = char_types[str[i]];

				switch(char_type)
				{
				case CHAR_NAME:
					tmp += str[i++];
					while (i < str.size())
					{
						char_type = char_types[str[i]];
						if (char_type != CHAR_NAME && char_type != CHAR_DIGIT) break;
						tmp += str[i++];
					}
					tok.type = token_types[tmp];
					if (!tok.type) tok.type = IDENTIFIER;
					break;

				case CHAR_SEP:
					if (i + 1 < str.size())
					{
						if (char_types[str[i + 1]] == CHAR_DIGIT)
						{
							tmp += str[i++];
							tok.type = FLOAT_LITERAL;
							goto float_literal;
						}
					}
					tmp += str[i++];
					tok.type = token_types[tmp];
					break;

				case CHAR_DIGIT:
float_literal:
					if (!tok.type) tok.type = INT_LITERAL;
					tmp += str[i++];
					while (i < str.size())
					{
						if (str[i] == '.')
						{
							if (tok.type == FLOAT_LITERAL)
							{
								//ERROR invalid literal 
								// i.e. it has more than one decimal
								std::cout << "Error: ill-formed float literal\n";
								return {};
							}
							tok.type = FLOAT_LITERAL;
							tmp += str[i++];
							continue;
						}

						if (char_types[str[i]] != CHAR_DIGIT) break;
						tmp += str[i++];
					}
					break;
					
				case CHAR_OP:
					tmp += str[i++];
					while (i < str.size())
					{
						if (char_types[str[i]] != CHAR_OP) break;
						tmp += str[i++];
					}
					tok.type = token_types[tmp];
					break;

				case CHAR_LIT:
					if (str[i] == '\'') tok.type = CHAR_LITERAL;
					else if (str[i] == '\"') tok.type = STRING_LITERAL;

					tmp += str[i++];
					while (i < str.size())
					{
						// is invalid character for string literal
						if (str[i] < 32 && str[i] != 9)
						{
							// it pushes back a spae so that  there is no
							// chance of the last char accidentally being the same
							// as the first. Avoids cases such as "abcd\" being valid
							tmp += ' ';
							break;
						}
						// terminating character
						else if (str[i] == tmp[0] && str[i - 1] != '\\')
						{
							tmp += str[i++];
							break;
						}
						
						tmp += str[i++];
					}

					if (tmp[0] != tmp.back())
					{
						// ERROR
						std::cout << "Missing terminating " << tmp[0] << " character in literal\n";
						return {};
					}
					break;

				case CHAR_NOTYPE:
					if (str[i] == '\n')
					{
						line++;
						col = 1;
					}
					else
					{
						col++;
					}
					i++;
					continue;
				
				case CHAR_INVALID:
					std::cout << "Invalid token found at line: " << line << ", col: " << col << std::endl;
					return {};
				}

				tok.value = tmp;
				tok.line = line;
				tok.col = col;
				toks.push_back(tok);
				col += tmp.size();
			}

			std::cout << "Tok Count: " << toks.size() << std::endl;

			for (long i = 0; i < toks.size(); i++)
			{
				std::cout << i << ": " << toks[i] << std::endl;
			}


			return toks;
		}

		std::ostream& operator<<(std::ostream& out, const Token& t)
		{
			#define TOKEN_VAL_OUTPUT_LEN 16
			if (t.value.size() > TOKEN_VAL_OUTPUT_LEN)
			{
				out << t.value << ": " << t.type << " @ " << t.line << ':' << t.col;
			}
			else
			{
				out << t.value;
				for (int i = 0; i < TOKEN_VAL_OUTPUT_LEN - t.value.size(); i++) out << ' ';
				out << ":\t" << t.type << "\t@ " << t.line << ':' << t.col;
			}
			return out;
		}
	}
}
