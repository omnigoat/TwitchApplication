//=====================================================================
//
//  Lexical Analysis
//  ------------------
//    Kaleidescope has m_tokens starting with letters or underscores,
//    case sensitivity, operators, and "other" (brackets, +, - , etc)
//
//=====================================================================
#ifndef ATMA_SOOTY_PARSE_HPP
#define ATMA_SOOTY_PARSE_HPP
//=====================================================================
#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <vector>
#include <deque>
#include <list>
#include <map>
#include <sstream>
#include <stack>
//=====================================================================
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
//=====================================================================
#include <sooty/frontend/lexical_analysis/lexeme_list.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/parseme.hpp>
#include <sooty/frontend/syntactic_analysis/parser.hpp>
//=====================================================================
namespace sooty {
//=====================================================================
	
	namespace detail {
		bool parse(lexeme_list::const_iterator& current, const lexeme_list::const_iterator& end, 
			parseme_ptr& dest, const parser& parser, int& inserts);
	}
	
	
	void parse(lexeme_list::const_iterator begin, lexeme_list::const_iterator end,
		parseme_ptr& dest, const parser& start_rule);

	void print_tree(const parseme_ptr&);
	
	
	
	
	
//=====================================================================
} // namespace sooty
//=====================================================================
#endif
//=====================================================================
