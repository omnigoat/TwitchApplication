#include <iterator>
#include <sstream>
#include <iomanip>
//=====================================================================
#include <boost/program_options.hpp>
//=====================================================================
#include <sooty/frontend/lexical_analysis/lexemifier.hpp>
#include <sooty/frontend/lexical_analysis/input_iterator.hpp>
#include <sooty/frontend/syntactic_analysis/parser.hpp>
#include <sooty/frontend/syntactic_analysis/parse.hpp>
#include <sooty/frontend/syntactic_analysis/algorithm.hpp>
#include <sooty/frontend/syntactic_analysis/sandbox.hpp>

//=====================================================================
// IDs. always best to keep lexical ids and AST ids seperate
//=====================================================================
struct lexid {
	enum Enum {
		variable,
		integer,
		lparen,
		rparen,
		plus,
		dash,
		fwdslash,
		star
	};
};

struct parsid {
	enum Enum {
		addition = 100,
		subtraction,
		multiplication,
		division,
		number,
		variable
	};
};



//=====================================================================
// print tree in prefix
//=====================================================================
void print_node(sooty::const_parseme_ptr_ref x) {
	std::cout << " ";
	
	if (x->id != 0 && x->id != parsid::number && x->id != parsid::variable)
		std::cout << "(";
	
	switch (x->id) {
		case parsid::addition:
			std::cout << "+";
			break;
		
		case parsid::subtraction:
			std::cout << "-";
			break;
		
		case parsid::multiplication:
			std::cout << "*";
			break;
		
		case parsid::division:
			std::cout << "/";
			break;
		
		case parsid::number:
			std::cout << std::setprecision(2) << x->value.real;
			break;
		
		case parsid::variable:
			std::cout << x->value.string;
			break;
	}
}

void print_rparen(sooty::const_parseme_ptr_ref x) {
	if (x->id != 0 && x->id != parsid::number && x->id != parsid::variable)
		std::cout << ")";
}







//=====================================================================
//
//  Reduction
//  -----------
//
//=====================================================================
float add(float lhs, float rhs) { return lhs + rhs; }
float sub(float lhs, float rhs) { return lhs - rhs; }
float mul(float lhs, float rhs) { return lhs * rhs; }
float idiv(float lhs, float rhs) { return lhs / rhs; }




// if we have the following:
//
//     oper {1}
//    /    \
//  num    oper {2}
//        /    \
//      num     _
//
// and oper{1} and oper{2} have the same precedence, we can fold
//
// @N refers to the index in x->children that is a parsid::number
// @O refers to the other node
void reduce_to_parent(sooty::parseme_ptr_ref parent, sooty::parseme_ptr_ref x, sooty::parseme_ptr_ref sibling, size_t N, size_t O, float(*oper)(float, float), bool swap = false)
{
	if (!swap)
		parent->id = x->id;
	
	parent->children[N]->id = parsid::number;
	parent->children[N]->value = sooty::value_t( (*oper)(sibling->value.real, x->children[N]->value.real) );
	
	parent->children[O] = x->children[O];
	parent->children[O]->parent = parent;
	
	parent->children[N]->children.clear();
	
	
	if (swap)
		std::swap(parent->children[O], parent->children[N]);
}

sooty::parseme_ptr sibling_of(sooty::parseme_ptr_ref x) {
	sooty::parseme_ptr parent = x->parent.lock();
	
	if (parent && parent->id != 0)
		if (parent->children[0] == x)
			return parent->children[1];
		else
			return parent->children[0];
	else
		return sooty::parseme_ptr();
}

void reduce(sooty::parseme_ptr_ref x)
{
	using sooty::parseme_ptr;
	using sooty::parseme_ptr_ref;
	
	// convert the integer to float
	//x->value.real = float(x->value.integer);
	
	for (sooty::parseme_container::iterator i = x->children.begin(); i != x->children.end(); ++i) {
		reduce(*i);
	}
	
	float (*oper)(float,float);
	switch (x->id) {
		case parsid::addition:
			oper = &add;
			break;
		case parsid::subtraction:
			oper = &sub;
			break;
		case parsid::multiplication:
			oper = &mul;
			break;
		case parsid::division:
			oper = &idiv;
			break;
		
		default:
			return;
	}

	assert(x->children.size() == 2);
	
	// the simplest case of reduction is when both our operands are numbers
	if (x->children[0]->id == parsid::number && x->children[1]->id == parsid::number) {
		float result = (*oper)(x->children[0]->value.real, x->children[1]->value.real);
		x->id = parsid::number;
		x->value = sooty::value_t(result);
		x->children.clear();
		return;
	}
	
	
	
	// next comes when we have nested operators of equal precedence
	parseme_ptr sibling = sibling_of(x);
	if (sibling && sibling->id != parsid::number)
		return;
	
	if (x->children[0]->id != parsid::number && x->children[1]->id != parsid::number)
		return;
	
	// if we have a sibling, we defs have a parent
	parseme_ptr parent = x->parent.lock();
		
	 if (x->id == parsid::addition) {
		if (parent->id == parsid::addition) {
			if (x->children[0]->id == parsid::number)
				reduce_to_parent(parent, x, sibling, 0, 1, &add);
			else
				reduce_to_parent(parent, x, sibling, 1, 0, &add);
		}
		else if (parent->id == parsid::subtraction) {
			if (x->children[0]->id == parsid::number)
				reduce_to_parent(parent, x, sibling, 0, 1, &sub);
			else
				reduce_to_parent(parent, x, sibling, 1, 0, &sub, true);
		}
	}
	else if (x->id == parsid::subtraction) {
		if (parent->id == parsid::addition) {
			if (x->children[0]->id == parsid::number)
				reduce_to_parent(parent, x, sibling, 0, 1, &add);
			else
				reduce_to_parent(parent, x, sibling, 1, 0, &sub, true);
		}
		else if (parent->id == parsid::subtraction) {
			if (x->children[0]->id == parsid::number)
				reduce_to_parent(parent, x, sibling, 0, 1, &sub);
			else
				reduce_to_parent(parent, x, sibling, 1, 0, &sub, true);
		}
	}
	else if (x->id == parsid::multiplication) {
		if (parent->id == parsid::multiplication) {
			if (x->children[0]->id == parsid::number)
				reduce_to_parent(parent, x, sibling, 0, 1, &mul);
			else
				reduce_to_parent(parent, x, sibling, 1, 0, &mul);
		}
	}
	else if (x->id == parsid::division) {
		if (parent->id == parsid::multiplication) {
			if (x->children[0]->id == parsid::number)
				reduce_to_parent(parent, x, sibling, 0, 1, &mul);
		}
	}
}

//=====================================================================
//
//  main
//  ------
//
//=====================================================================
int main(int arg_count, char* args[])
{
	namespace po = boost::program_options;
	
	//=====================================================================
	// command-line
	//=====================================================================
	po::variables_map vm;
	std::string input;
	{
		po::options_description
			cmd_line("Allowed options"),
			hidden,
			all
			;
		
		hidden.add_options()
			("input-file", "bam")
			;
		
		cmd_line.add_options()
			(",r", "reduce expression")
			(",a", "expression is passed as an argument on the command line")
			//("display", po::value(&display_mode)->default_value("prefix"), "[prefix,postfix] how to print syntax tree")
			;
		
		all.add(cmd_line).add(hidden);
		
		po::positional_options_description pod;
		pod.add("input-file", -1);
		
		try {
			po::store(po::command_line_parser(arg_count, args).options(all).positional(pod).run(), vm);
			po::notify(vm);
		}
		catch (const std::exception&) {
			std::cout << "There was an error parsing the command line. Make sure, if using the " << std::endl
				<< " -a command, to put the expression in quotes" << std::endl;
			std::cout << std::endl;
			std::cout << cmd_line << std::endl;
			std::cout << "Examples:" << std::endl
				<< "  sooty -r test.txt" << std::endl
				<< "  sooty -ra \"4 + 5 * dragon - (12 * knight)\"" << std::endl;
			return -1;
		}
		
		if (vm.count("input-file") < 1) {
			std::cout << cmd_line << std::endl;
			return -1;
		}
		
		if (vm.count("-a") == 1) {
			input = vm["input-file"].as<std::string>();
		}
		else {
			std::ifstream file(vm["input-file"].as<std::string>().c_str());
			file.unsetf(std::ifstream::skipws);
			std::copy(std::istream_iterator<char>(file), std::istream_iterator<char>(), std::back_inserter(input));
		}
		
	}
	
	
	//=====================================================================
	// lexing
	//=====================================================================
	sooty::lexemifier_t lexemifier;
	{
		sooty::lexer main_lexer =
		+(
			(+sooty::in_range('a', 'z'))[lexemifier.make(lexid::variable, sooty::any_channel, true)] |
			(!sooty::char_('-') >> +sooty::in_range('0', '9'))[lexemifier.make<float>(lexid::integer, sooty::any_channel, true)] |
			sooty::char_('+')[lexemifier.make(lexid::plus, sooty::any_channel)] |
			sooty::char_('-')[lexemifier.make(lexid::dash, sooty::any_channel)] |
			sooty::char_('*')[lexemifier.make(lexid::star, sooty::any_channel)] |
			sooty::char_('/')[lexemifier.make(lexid::fwdslash, sooty::any_channel)] |
			sooty::char_('(')[lexemifier.make(lexid::lparen, sooty::any_channel)] |
			sooty::char_(')')[lexemifier.make(lexid::rparen, sooty::any_channel)] |
			sooty::char_(' ')
		)
		;
		
		main_lexer(input.begin(), input.end());
	}
	
	//=====================================================================
	// parser
	//=====================================================================
	sooty::parseme_ptr root = sooty::make_parseme(sooty::parseme_ptr(), 0);
	{
		
		using sooty::match;
		using sooty::insert;
		using sooty::match_insert;
		
		sooty::parser
			additive_expression,
			multiplicative_expression,
			primary_expression
			;
		
		sooty::parseme_stack
			additive_expression_stack,
			multiplicative_expression_stack
			;
		
		additive_expression =
			sooty::store(additive_expression_stack, multiplicative_expression)
			>>
			(
				(
					+(
						(match(lexid::plus, false) >>
						sooty::store(additive_expression_stack,
							insert(parsid::addition) [
								sooty::retrieve(additive_expression_stack) >>
								multiplicative_expression
							]
						))
						|
						(match(lexid::dash, false) >>
						sooty::store(additive_expression_stack,
							insert(parsid::subtraction) [
								sooty::retrieve(additive_expression_stack) >>
								multiplicative_expression
							]
						))
					)
					>>
					sooty::retrieve(additive_expression_stack)
				)
				|
				sooty::retrieve(additive_expression_stack)
			)
			;
		
		multiplicative_expression =
			sooty::store(multiplicative_expression_stack, primary_expression)
			>>
			((
				+(
					(match(lexid::star, false) >>
					sooty::store(multiplicative_expression_stack,
						insert(parsid::multiplication) [
							sooty::retrieve(multiplicative_expression_stack) >>
							primary_expression
						]
					))
					|
					(match(lexid::fwdslash, false) >>
					sooty::store(multiplicative_expression_stack,
						insert(parsid::division) [
							sooty::retrieve(multiplicative_expression_stack) >>
							primary_expression
						]
					))
				)
				>>
				sooty::retrieve(multiplicative_expression_stack)
			)
			|
			sooty::retrieve(multiplicative_expression_stack)
			)
			;
		
		primary_expression =
			match_insert(lexid::variable, parsid::variable) |
			match_insert(lexid::integer, parsid::number) |
			(
				match(lexid::lparen, false) >>
				additive_expression >>
				match(lexid::rparen, false)
			)
			;
		
		
		additive_expression(lexemifier.list.begin(), lexemifier.list.end(), root);
	}
	
	
	//=====================================================================
	// reduction
	//=====================================================================
	if (vm.count("-r")) {
		std::cout << "before reduction:\n  ";
		sooty::depth_first_for_each(root, print_node, print_rparen);
		
		std::cout << std::endl;
		
		reduce(root);
		
		std::cout << std::endl;
		std::cout << "after reduction:\n  ";
	}
	else {
		std::cout << "expression:\n  ";
	}
	
	
	sooty::depth_first_for_each(root, print_node, print_rparen);
	std::cout << std::endl;
}
