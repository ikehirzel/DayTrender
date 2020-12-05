#include "generator.h"

namespace dtbuild
{
	namespace generator
	{
		std::string generate(const parser::Node& tree)
		{
			std::string out;
			using namespace parser;
			switch(tree.type)
			{
			case PROGRAM:
				std::cout << "\nPROGRAM:\n\n";
				for (const Node& arg : tree.args)
				{
					out += generate(arg);
				}
				break;

			case FUNC:
				out += generate(tree.args[0]);
				std::cout << "(";
				out += generate(tree.args[1]);
				std::cout << ")\n{\n";
				out += generate(tree.args[2]);
				std::cout << "}\n";
				break;

			case FUNC_CALL:
				out += generate(tree.args[0]);
				std::cout << '(';
				if (tree.args.size()>1)
				out += generate(tree.args[1]);
				std::cout << ')';
				break;

			case FACTOR:
				if (tree.args[0].type == EXPR)
				{
					std::cout << '(';
					for (const Node& arg : tree.args) out += generate(arg);
					std::cout << ") ";
				}
				else
				{
					for (const Node& arg : tree.args) out += generate(arg);
				}
				break;

			case FUNCBDY:
				for (const Node& arg : tree.args)
				{
					out += generate(arg);
				}
				break;

			case STMTS:
				for (const Node& arg : tree.args)
				{
					std::cout << '\t';
					out += generate(arg);
				}
				break;

			case LIST:
			case ARGS:
				for (int i = 0; i < tree.args.size(); i++)
				{
					out += generate(tree.args[i]);
					if (i < tree.args.size() - 1)
					{
						std::cout << ", ";
					}
				}
				break;

			case INIT_LIST:
				std::cout << '<';
				for (const Node& arg : tree.args)
				{
					out += generate(arg);
				}
				std::cout << "> ";
				break;

			case STMT:
				for (const Node& arg : tree.args)
				{
					out += generate(arg);
				}

				std::cout << ";\n";
				break;

			default:
				if (tree.args.empty())
				{
					std::cout << tree.value << ' ';
				}
				else
				{
					for (const Node& arg : tree.args)
					{
						out += generate(arg);
					}
				}
				break;
			}
			return out;
		}
	} // namespace generator
} // namespace dtbuild