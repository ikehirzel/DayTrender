#include "lexer.h"

#include "dtbuild.h"

#include <iostream>

#include <hirzel/strutil.h>
#include <hirzel/fileutil.h>

namespace dtbuild
{
	char char_types[256];

	std::unordered_map<std::string, short> token_types;

	void lex_init()
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
				{ "<",	LANGBRACK	},
				{ ">",	RANGBRACK	},

				// arithmetic operators
				{ "!",	NOT			},
				{ "%",	MODULUS		},
				{ "/",	SLASH		},
				{ "*",	ASTERISK	},
				{ "+",	PLUS		},
				{ "-",	MINUS		},
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
				{ "^", 	POWER		},

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

	std::vector<Token> lex(const std::string& filepath)
	{
		using namespace hirzel;

		std::string str = file::read_file_as_string(filepath);

		std::vector<Token> toks;
		long line = 1;
		int col = 1;
		size_t i = 0;
		bool newline = true;
		int comment = 0;
		while (i < str.size())
		{
			Token tok;
			std::string tmp;
			char char_type = char_types[str[i]];

			if (newline)
			{
				if (str[i] > 32) newline = false;
				if (str[i] == '#')
				{
					bool reading = false;
					bool in_string = false;
					std::vector<std::string> args;

					size_t oi = i;
					i++;
					while (i < str.size())
					{
						if (str[i] == '\n')
						{
							break;
						}
						else if (str[i] == '\"')
						{
							in_string = !in_string;
						}

						if (reading)
						{
							if (str[i] < 33)
							{
								reading = false;
							}
							else
							{
								args.back() += str[i];
							}
						}
						else
						{
							if (str[i] > 32)
							{
								reading = true;
								args.push_back(std::string(1, str[i]));
							}
						}

						i++;
					}

					if (in_string)
					{
						std::cout << "Missing terminating \" in preprocessor directive\n";
						return {};
					}
					
					if (args.empty())
					{
						std::cout << "Stray '#' found in program!\n";
						return {};
					}
					else if (args.size() < 2)
					{
						std::cout << "No arguments given to preprocessor directive #" << args[0] << '\n';
						return {};
					}

					if (args[0] == "require")
					{
						for (int i = 1; i < args.size(); i++)
						{
							std::string fp = str::get_folder(filepath) + '/' + args[i].substr(1, args[i].size() - 2);
							std::vector<Token> ts = lex(fp);
							toks.insert(toks.end(), ts.begin(), ts.end());
						}
					}
					else
					{
						std::cout << "Unknown preprocessor directive #" << args[0] << '\n';
						return {};
					}

					continue;
				}
			}

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

				tok.filepath = filepath;
				tok.value += tmp;
				tok.line = line;
				tok.col = col;
				toks.push_back(tok);
				col += tmp.size();
				break;

			case CHAR_SEP:
				if (i + 1 < str.size())
				{
					if (str[i] == '.' && char_types[str[i + 1]] == CHAR_DIGIT)
					{
						tmp += str[i++];
						tok.type = FLOAT_LITERAL;
						goto float_literal;
					}
				}

				tok.filepath = filepath;
				tok.value += str[i++];
				tok.type = token_types[tok.value];
				tok.line = line;
				tok.col = col;
				toks.push_back(tok);
				col++;
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

				tok.filepath = filepath;
				tok.value = tmp;
				tok.line = line;
				tok.col = col;
				toks.push_back(tok);
				col += tmp.size();
				break;
				
			case CHAR_OP:
				tmp += str[i++];
				while (i < str.size())
				{
					if (char_types[str[i]] != CHAR_OP) break;
					if (tmp.back() == '/')
					{
						if (str[i] == '/')
						{
							comment = 1;
							tmp.pop_back();
							break;
						}
						else if (str[i] == '*')
						{
							comment = 2;
							tmp.pop_back();
							break;
						}
					}
					tmp += str[i++];
				}
				if (!tmp.empty())
				{
					tok.filepath = filepath;
					tok.value = tmp;
					tok.type = token_types[tmp];
					tok.line = line;
					tok.col = col;
					toks.push_back(tok);
					col += tmp.size();
				}
				break;

			case CHAR_LIT:
				if (str[i] == '\'') tok.type = CHAR_LITERAL;
				else if (str[i] == '\"') tok.type = STRING_LITERAL;

				tmp += str[i++];

				tok.filepath = filepath;
				tok.line = line;
				tok.col = col;
				col++;
				while (i < str.size())
				{
					if (str[i] < 32)
					{
						switch(str[i])
						{
						case '\n':
							line++;
							col = 0;
							tmp += "\\n";
							break;

						case '\t':
							tmp += "\\t";
							break;

						default:
							tmp += ' ';
							break;
						}
						i++;
						col++;
						continue;
					}
					else if (str[i] == tmp[0] && str[i - 1] != '\\')
					{
						col++;
						tmp += str[i++];
						break;
					}
					col++;
					tmp += str[i++];
				}

				if (tmp[0] != tmp.back())
				{
					// ERROR
					std::cout << "Missing terminating " << tmp[0] << " character in literal\n";
					return {};
				}
				tok.value = tmp;
				toks.push_back(tok);
				break;

			case CHAR_NOTYPE:
				if (str[i] == '\n')
				{
					newline = true;
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

			if (comment == 1)
			{
				line++;
				col = 1;
				while (i < str.size())
				{
					if (str[i++] == '\n') break;
				}
				comment = 0;
			}
			else if (comment == 2)
			{
				col += 2;
				while (i < str.size())
				{
					col++;
					i++;
					if (str[i] == '\n')
					{
						line++;
						col = 0;
					}
					else if (str[i - 1] == '*' && str[i] == '/')
					{
						i++;
						break;
					}
				}
				comment = 0;
			}
		}

		// std::cout << "Tok Count: " << toks.size() << std::endl;

		// for (long i = 0; i < toks.size(); i++)
		// {
		// 	std::cout << i << ": " << toks[i] << std::endl;
		// }


		return toks;
	}

	std::ostream& operator<<(std::ostream& out, const Token& t)
	{
		#define TOKEN_VAL_OUTPUT_LEN 16
		if (t.value.size() > TOKEN_VAL_OUTPUT_LEN)
		{
			out << t.value << ": " << t.type << " @ " << t.line << ':' << t.col << '\t' << t.filepath;
		}
		else
		{
			out << t.value;
			for (int i = 0; i < TOKEN_VAL_OUTPUT_LEN - t.value.size(); i++) out << ' ';
			out << ":\t" << t.type << "\t@ " << t.line << ':' << t.col << '\t' << t.filepath;
		}
		return out;
	}
}
