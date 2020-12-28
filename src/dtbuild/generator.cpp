#include "generator.h"

#include "dtbuild.h"

namespace dtbuild
{
	// dtbuild specific rules for c++ generation
	void generate(std::string& out, const Node& tree, int tab_depth = 0)
	{
		switch(tree.type)
		{
		case PROGRAM:
			for (const Node& arg : tree.args)
			{
				generate(out, arg, tab_depth);
			}
			break;

		case INDI_INIT:
			generate(out, tree.args[0]);
			out += "(_candles, ";
			for (const Node& arg : tree.args[1].args)
			{
				generate(out, arg);
			}
			out += ')';
			break;

		case SEL_STMT:
			out += tabs(tab_depth) + "if (";
			generate(out, tree.args[0]);
			out += ")\n";
			generate(out, tree.args[1], tab_depth);
			break;

		case JUMP_STMT:
			out += tabs(tab_depth) + "return ";
			generate(out, tree.args[0]);
			out += ";\n";
			break;

		case DECL_STMT:
			out += tabs(tab_depth);
			generate(out, tree.args[0]);
			out += "= ";
			generate(out, tree.args[1]);
			out += ";\n";
			break;

		case EXPR_STMT:
			out += tabs(tab_depth);
			for (const Node& arg : tree.args)
			{
				generate(out, arg);
			}
			out += ";\n";
			break;

		case FOR_STMT:
			out += tabs(tab_depth);
			out += "for (";
			// /generate(out, tree.args[0]);
			generate(out, tree.args[0].args[0]);
			out += "= ";
			generate(out, tree.args[0].args[1]);
			out += "; ";
			generate(out, tree.args[1]);
			out += "; ";
			generate(out, tree.args[2]);
			out += ")\n"; 
			generate(out, tree.args[3], tab_depth);
			break;

		case INDEX_OP:
			out += '[';
			generate(out, tree.args[0]);
			out += ']';
			break;

		case ALGO_ARGS:
			out += tabs(tab_depth) + "out.dataset.resize(" + std::to_string(tree.args.size()) + ");\n\n";
			for (int i = 0; i < tree.args.size(); i++)
			{

				const Node& n = tree.args[i];
				out += tabs(tab_depth) + "out.dataset[" + std::to_string(i) + "] = ";
				generate(out, n.args[0], tab_depth);
				out += ";\n";
				out += tabs(tab_depth) + "const std::vector<double>& _";
				generate(out, n.args[1], tab_depth);
				out += " = out.dataset[" + std::to_string(i) + "];\n\n"; // the identifier
			}
			break;

		case INDI:
			out += tabs(tab_depth) + "std::vector<double> ";
			generate(out, tree.args[0]);
			out += "(const candleset& _candles, ";
			generate(out, tree.args[1], tab_depth);
			out += ")\n";
			generate(out, tree.args[2], tab_depth);
			out += '\n';
			break;
			
		case ALGO:
			out += tabs(tab_depth) + "extern \"C\" void ";
			generate(out, tree.args[0]);
			out += "(const candleset& _candles, algorithm_data& out)\n";
			out += tabs(tab_depth) + "{\n";
			generate(out, tree.args[1], tab_depth+1);
			generate(out, tree.args[2], tab_depth+1);
			out += tabs(tab_depth) + "}\n";
			break;

		case FUNC:
			out += tabs(tab_depth);
			generate(out, tree.args[0], tab_depth + 1);
			out += '(';
			generate(out, tree.args[1]);
			out += ")\n";
			generate(out, tree.args[2], tab_depth);
			out += '\n';
			break;

		case CALL_OP:
			out += '(';
			generate(out, tree.args[0]);
			out += ')';
			break;

		case FACTOR:
			if (tree.args[0].type == EXPR)
			{
				out += '(';
				for (const Node& arg : tree.args) generate(out, arg, tab_depth);
				out += ") ";
			}
			else
			{
				for (const Node& arg : tree.args) generate(out, arg, tab_depth);
			}
			break;

		case FUNCBDY:
			for (const Node& arg : tree.args)
			{
				generate(out, arg, tab_depth);
			}
			break;

		case ACCESSOR:
			out += '.';
			generate(out, tree.args[0]);
			break;

		case PACCESSOR:
			out += "->";
			generate(out, tree.args[0]);
			break;

		case COMPOUND_STMT:
			out += tabs(tab_depth) + "{\n";
			for (const Node& arg : tree.args)
			{
				generate(out, arg, tab_depth + 1);
			}
			out += tabs(tab_depth) + "}\n";
			break;

		case LIST:
		case ARGS:
		case INIT_LIST:
			for (int i = 0; i < tree.args.size(); i++)
			{
				generate(out, tree.args[i], tab_depth);
				if (i < tree.args.size() - 1)
				{
					out += ", ";
				}
			}
			break;

		case DECLARATION:
			out += tabs(tab_depth);
			generate(out, tree.args[0]);
			out += "= ";
			generate(out, tree.args[1]);
			out += ";\n";
			break;

		default:
			if (tree.args.empty())
			{
				out += tree.value + ' ';
			}
			else
			{
				for (const Node& arg : tree.args)
				{
					generate(out, arg, tab_depth);
				}
			}
			break;
		}
	}

	std::string generate_code(const Node& tree)
	{
		std::string out = "#include <candle.h>\n#include <algodefs.h>\n\n";
		generate(out, tree, 1);

		return out;
	}
} // namespace dtbuild