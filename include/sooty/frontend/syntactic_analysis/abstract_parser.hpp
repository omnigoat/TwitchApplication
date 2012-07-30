//=====================================================================
//
//  Lexical Analysis
//  ------------------
//    Kaleidescope has m_tokens starting with letters or underscores,
//    case sensitivity, operators, and "other" (brackets, +, - , etc)
//
//=====================================================================
#ifndef ATMA_SOOTY_PARSER_PARSER_HPP
#define ATMA_SOOTY_PARSER_PARSER_HPP
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
//=====================================================================
SOOTY_BEGIN
//=====================================================================
class abstract_parser;
//=====================================================================
	
		/*
	struct parse_state
	{
	private:
		// a bunch of lexemes!
		lexeme_list lexemes;
		// our current lexeme!
		lexeme_list::iterator current_lexeme_;
		// head tree node
		parseme_ptr root_;
		// current node we're inserting into
		parseme_ptr current_;

	public:
		const lexeme& current_lexeme() const { return *current_lexeme_; }
		const parseme_ptr& root_node() const { return root_; }
		const parseme_ptr& current_node() const { return current_; }
	};
	
	namespace detail 
	{
		void match(const lexeme_list::iterator&, parser&, parseme_ptr&);
	}
	
	parseme_ptr parse(lexeme_list::iterator begin, lexeme_list::iterator end, parser& start_rule);
	
	*/
	
	//=====================================================================
	//
	//  abstract_parser
	//  -----------------
	//    an abstract recursive descent parser. nice!
	//
	//=====================================================================
	class abstract_parser
	{
		// m_list of lexer m_tokens
		lexeme_list lexeme_list_;
		// current location
		lexeme_list::iterator current_lexeme_iter_;
		
		// head tree node
		parseme_ptr root_;
		// stack of pointers
		std::stack<parseme_ptr> node_stack_;
		
	public:
		//=====================================================================
		// constructor!
		//=====================================================================
		abstract_parser(const lexeme_list& ltl, const multichannel& starting_channel);
		
		//=====================================================================
		// pretty print for debugging
		//=====================================================================
		virtual void print_tree() const = 0;
		
		//=====================================================================
		// entry point
		//=====================================================================
		void parse();
		
		parseme_ptr root_node() { return root_; }
	
	protected:
		lexeme_list::iterator& current_lexeme_iter() { return current_lexeme_iter_; }
		
		//=====================================================================
		//=====================================================================
		const parseme_ptr& get_current_node() const;
		
		//=====================================================================
		// error messages
		//=====================================================================
		void expected(const std::string&);
		void unexpected();
		
		
		//=====================================================================
		// delving and surfacing into the last child added
		//=====================================================================
		void delve();
		void surface();

		//=====================================================================
		// Matching
		// ---------
		//  Matching checks the current lexeme's IDENTIFIER against token_id, and if it
		//  is a match, creates a new parseme in the tree with that value, if
		//  insert is true. If delve is true, it then "delves" into that child
		//  node.
		//
		//  match_insert allows the user to "switch" the new parseme's id and/or
		//  value for a specified one.
		//
		//  Any and all delving delves into the newly-created node.
		//
		//=====================================================================
	public:
		
		parseme_ptr operator ()(size_t token_id, bool insert = true, bool delve = false);
		//parseme_ptr match(size_t token_id, const value_t& value, bool insert = true, bool delve = false);
		parseme_ptr match_insert_impl(size_t token_to_match, size_t token_to_insert, bool delve = false);
		parseme_ptr match_insert_impl(size_t token_to_match, size_t token_to_insert, const value_t& value, bool delve = false);
		
		
		//=====================================================================
		// Inserting
		// ----------
		//  As opposed to matching, inserting is simply inserting nodes that
		//  are created by the user, and aren't related to the lexeme list at
		//  all.
		//
		//  Any and all delving delves into the newly-created node.
		//
		//=====================================================================
		parseme_ptr insert_impl(size_t token_id, bool delve = false);
		parseme_ptr insert_impl(size_t token_id, const value_t& value, bool delve = false);
		
		
		//=====================================================================
		// Error Recovery
		// ---------------
		//  There are two ways to undo previous insertions
		//=====================================================================
		void undo_last_insert(size_t amount = 1);
		
		
		//=====================================================================
		// moves the position forwards one in the requested channel
		//=====================================================================
	public:
		inline void advance(const multichannel& c) {
			current_lexeme_iter().move(1, c);
		}
		
		void advance(int n, const multichannel& c) {
			current_lexeme_iter().move(n, c);
		}
		
		const parseme_ptr& get_root() const { return root_; }
		
		virtual void start_rule_impl() = 0;
		
		
		
		size_t LA(int num)
		{
			lexeme_list::const_iterator i = current_lexeme_iter_;
			if (!i.move(num)) return -1;
			return i->id;
		}
		
		
		parseme_ptr create_parseme(size_t id, const value_t& value)
		{
			return parseme::create(parseme::nullary, id, value, current_lexeme_iter_->file_position);
		}
		
		
		template <size_t A> parseme_ptr failsafe(const_parseme_ptr_ref P)
		{
			if (!P) undo_last_insert(A);
			return P;
		}
		
	private:
	//	std::stack< boost::function< parseme_ptr(const_parseme_ptr_ref, bool) > > inserters_;
		parseme_ptr insert_impl(const parseme_ptr& p, bool delve);
		//const std::string& current_value() { return current_lexeme_iter()->value; }
	};

//=====================================================================
SOOTY_CLOSE
//=====================================================================
#endif
//=====================================================================
