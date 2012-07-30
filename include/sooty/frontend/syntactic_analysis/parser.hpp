//=====================================================================
//
//  Lexical Analysis
//  ------------------
//    Kaleidescope has m_tokens starting with letters or underscores,
//    case sensitivity, operators, and "other" (brackets, +, - , etc)
//
//=====================================================================
#ifndef ATMA_SOOTY_RULE_HPP
#define ATMA_SOOTY_RULE_HPP
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
//#include <boost/signal.hpp>
//=====================================================================
#include <atma/functor.hpp>
//=====================================================================
#include <sooty/frontend/lexical_analysis/lexeme_list.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/parseme.hpp>
//=====================================================================
namespace sooty {
//=====================================================================
struct rule;
struct rule_storage;
//=====================================================================
	
	//=====================================================================
	// parsing results
	//=====================================================================
	struct parse_results_t
	{
		bool pass;
		bool hit;
		bool full;
		lexeme_list::const_iterator start, stop;
		parseme_ptr node;
		
		parse_results_t(bool pass, bool hit, bool full, const lexeme_list::const_iterator& start, const lexeme_list::const_iterator& stop, const parseme_ptr& node)
			: pass(pass), hit(hit), full(full), start(start), stop(stop), node(node)
		{
		}
	};
	
	//=====================================================================
	// forward declares
	//=====================================================================
	class parser;
	namespace detail {
		struct abstract_parser_backend;
		typedef boost::shared_ptr<abstract_parser_backend> abstract_parser_backend_ptr;
	}

	typedef boost::function< void (parse_results_t&) > parsefunc_t;
	typedef boost::function< parse_results_t(const lexeme_list::const_iterator&, const lexeme_list::const_iterator&, parseme_ptr&) > user_parser_t;
	
	
	
	//=====================================================================
	//=====================================================================
	namespace detail
	{
		struct parser_handle_t
		{
			detail::abstract_parser_backend_ptr backend;
			//parsefunc_t on_success;
			parsefunc_t on_failure;
			
			parser_handle_t() {}
			parser_handle_t(const detail::abstract_parser_backend_ptr& backend) : backend(backend) {}
			
			parsefunc_t on_success_;
		};
		
		typedef boost::shared_ptr<parser_handle_t> parser_handle_ptr;
	}
	
	
	//=====================================================================
	//=====================================================================
	namespace detail
	{
		struct abstract_parser_backend
		{
			virtual parse_results_t operator ()(const lexeme_list::const_iterator&, const lexeme_list::const_iterator&, parseme_ptr&) const = 0;
			virtual ~abstract_parser_backend() {}
		};
	}
	
	
	//=====================================================================
	// all the user need see
	//=====================================================================
	class parser
	{
	public:
		// SERIOUSLY, fix this. make it private.
		detail::parser_handle_ptr handle_;
	
		parser();
		parser(const parser&);
		parser(const detail::abstract_parser_backend_ptr& backend);
		
		parser& operator = (const parser& rhs);
		
		parse_results_t operator ()(const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr& dest) const;
		
		parser operator [] (parser&);
		parser operator >> (parser&);
		parser operator |  (parser&);
		parser operator || (parser&);
		parser operator ,  (parser&);
		parser operator *  ();
		parser operator +  ();
		parser operator !  ();
		parser operator %  (parser&);
		
		parser& on_success(parsefunc_t func); //parser P(*this); P.on_success_ = func; return P;}
		//parser& on_failure(parsefunc_t func) const { return *this; } //parser P(*this); P.on_failure_ = func; return P;}
		
		
		//parsefunc_t on_failure_;
	};
	
	
	
	
	//=====================================================================
	// a reference to a parser (points to the same backend)
	//=====================================================================
	namespace detail
	{
		struct parser_ref : parser
		{
			parser_ref();
			//parser_ref(const parser_ref&);
			parser_ref(const parser&);
			
			parser_ref& operator = (const parser&);
			
			
			//parser_ref& operator =
			using parser::operator *;
			using parser::operator !;
			using parser::operator ();
			using parser::operator +;
			using parser::operator ,;
			using parser::operator =;
			using parser::operator >>;
			using parser::operator [];
			using parser::operator |;
			using parser::operator ||;
		};
	}
	
	
	
	
	//=====================================================================
	// backends
	//=====================================================================
	namespace detail
	{
		struct match_parser_backend : abstract_parser_backend
		{
			size_t match_id;
			bool insert;
			match_parser_backend(size_t match_id, bool insert) : match_id(match_id), insert(insert) {}
			parse_results_t operator ()(const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr&) const;
		};
		
		struct insert_parser_backend : abstract_parser_backend
		{
			size_t id;
			value_t value;
			insert_parser_backend(size_t id, const value_t& value) : id(id), value(value) {}
			parse_results_t operator ()(const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr&) const;
		};
		
		struct insert_ref_parser_backend : abstract_parser_backend
		{
			const_parseme_ptr_ref p;
			insert_ref_parser_backend(sooty::const_parseme_ptr_ref p) : p(p) {}
			parse_results_t operator ()(const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr&) const;
		};
		
		struct insert_some_parser_backend : abstract_parser_backend
		{
			sooty::parseme_container::iterator begin, end;
			insert_some_parser_backend(sooty::parseme_container::iterator begin, sooty::parseme_container::iterator end)
				: begin(begin), end(end) {}
			parse_results_t operator ()(const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr&) const;
		};
				
		struct match_insert_parser_backend : abstract_parser_backend
		{
			size_t match_id;
			size_t insert_id;
			value_t value;
			bool use_value;
			match_insert_parser_backend(size_t match_id, size_t insert_id, const value_t& value, bool use_value)
				: match_id(match_id), insert_id(insert_id), value(value), use_value(use_value) {}
			parse_results_t operator ()(const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr&) const;
		};
		
		struct delve_parser_backend : abstract_parser_backend
		{
			parser_ref head, child;
			delve_parser_backend(const parser& head, const parser& child) : head(head), child(child) {}
			parse_results_t operator ()(const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr&) const;
		};
		
		struct seq_and_parser_backend : abstract_parser_backend
		{
			parser_ref lhs, rhs;
			seq_and_parser_backend(const parser& lhs, const parser& rhs)  : lhs(lhs), rhs(rhs) {}
			parse_results_t operator ()(const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr&) const;
		};
		
		struct seq_or_parser_backend : abstract_parser_backend
		{
			parser_ref lhs, rhs;
			seq_or_parser_backend(const parser& lhs, const parser& rhs) : lhs(lhs), rhs(rhs) {}
			parse_results_t operator ()(const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr&) const;
		};
		
		struct or_parser_backend : abstract_parser_backend
		{
			parser_ref lhs, rhs;
			or_parser_backend(const parser& lhs, const parser& rhs) : lhs(lhs), rhs(rhs) {}
			parse_results_t operator ()(const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr&) const;
		};
		
		struct zero_or_more_parser_backend : abstract_parser_backend
		{
			parser_ref P;
			zero_or_more_parser_backend(const parser& P) : P(P) {}
			parse_results_t operator ()(const lexeme_list::const_iterator&, const lexeme_list::const_iterator&, parseme_ptr&) const;
		};
		
		struct zero_or_one_parser_backend : abstract_parser_backend
		{
			parser_ref P;
			zero_or_one_parser_backend(const parser& P) : P(P) {}
			parse_results_t operator ()(const lexeme_list::const_iterator&, const lexeme_list::const_iterator&, parseme_ptr&) const;
		};
		
		struct one_or_more_parser_backend : abstract_parser_backend
		{
			parser_ref P;
			one_or_more_parser_backend(const parser& P) : P(P) {}
			parse_results_t operator ()(const lexeme_list::const_iterator&, const lexeme_list::const_iterator&, parseme_ptr&) const;
		};
		
		struct seperated_zero_or_more : abstract_parser_backend
		{
			parser_ref lhs, rhs;
			seperated_zero_or_more(const parser& lhs, const parser& rhs) : lhs(lhs), rhs(rhs) {}
			parse_results_t operator ()(const lexeme_list::const_iterator&, const lexeme_list::const_iterator&, parseme_ptr&) const;
		};
		
		
		
		struct user_parser_backend : abstract_parser_backend
		{
			user_parser_t P;
			user_parser_backend(user_parser_t P) : P(P) {}
			parse_results_t operator ()(const lexeme_list::const_iterator&, const lexeme_list::const_iterator&, parseme_ptr&) const;
		};
		
		struct guard_parser_backend : abstract_parser_backend
		{
			parser_ref P;
			guard_parser_backend(const parser& P) : P(P) {}
			parse_results_t operator ()(const lexeme_list::const_iterator&, const lexeme_list::const_iterator&, parseme_ptr&) const;
		};
		
		struct direct_parser_backend : abstract_parser_backend
		{
			parser_ref P;
			direct_parser_backend(const parser& P) : P(P) {}
			parse_results_t operator ()(const lexeme_list::const_iterator&, const lexeme_list::const_iterator&, parseme_ptr&) const;
		};
		/*
		struct assign_to_parser_backend : abstract_parser_backend
		{
			parseme_ptr& p;
			assign_to_parser_backend(parseme_ptr&);
			assign_to_parser_backend(const assign_to_parser_backend& rhs);
			parse_results_t operator ()(const lexeme_list::const_iterator&, const lexeme_list::const_iterator&, parseme_ptr&) const;
		};
		
		struct insert_from_parser_backend : abstract_parser_backend
		{
			parseme_ptr_ref p;
			insert_from_parser_backend(parseme_ptr_ref);
			parse_results_t operator ()(const lexeme_list::const_iterator&, const lexeme_list::const_iterator&, parseme_ptr&) const;
		};
		
		struct store_parser_backend : abstract_parser_backend
		{
			parser_ref P;
			mutable parseme_container& C;
			store_parser_backend(const parser& P, parseme_container& C) : P(P), C(C) {}
			parse_results_t operator ()(const lexeme_list::const_iterator&, const lexeme_list::const_iterator&, parseme_ptr&) const;
		};

		struct retrieve_parser_backend : abstract_parser_backend
		{
			mutable parseme_container& C;
			retrieve_parser_backend(parseme_container& C) : C(C) {}
			parse_results_t operator ()(const lexeme_list::const_iterator&, const lexeme_list::const_iterator&, parseme_ptr&) const;
		};
		*/
		struct stack_store_parser_backend : abstract_parser_backend
		{
			std::stack<parseme_ptr>& stack;
			parser_ref P;
			stack_store_parser_backend(std::stack<parseme_ptr>& stack, const parser& P) : stack(stack), P(P) {}
			parse_results_t operator ()(const lexeme_list::const_iterator&, const lexeme_list::const_iterator&, parseme_ptr&) const;
		};
		
		struct stack_retrieve_parser_backend : abstract_parser_backend
		{
			std::stack<parseme_ptr>& stack;
			stack_retrieve_parser_backend(std::stack<parseme_ptr>& stack) : stack(stack) {}
			parse_results_t operator ()(const lexeme_list::const_iterator&, const lexeme_list::const_iterator&, parseme_ptr&) const;
		};
	}
	
	

	
	inline parser match(size_t id, bool insert = true) {
		return parser( detail::abstract_parser_backend_ptr(new detail::match_parser_backend(id, insert)) );
	}
	
	inline parser insert(size_t id, const value_t& value = value_t()) {
		return parser( detail::abstract_parser_backend_ptr(new detail::insert_parser_backend(id, value)) );
	}
	
	inline parser insert(sooty::const_parseme_ptr_ref n) {
		return parser( detail::abstract_parser_backend_ptr(new detail::insert_ref_parser_backend(n)) );
	}
	
	inline parser insert(sooty::parseme_container::iterator begin, sooty::parseme_container::iterator end) {
		return parser( detail::abstract_parser_backend_ptr(new detail::insert_some_parser_backend(begin, end)) );
	}
	
	inline parser match_insert(size_t match_id, size_t insert_id) {
		return parser( detail::abstract_parser_backend_ptr(new detail::match_insert_parser_backend(match_id, insert_id, value_t(), false)) );
	}
	
	inline parser match_insert(size_t match_id, size_t insert_id, const value_t& value) {
		return parser( detail::abstract_parser_backend_ptr(new detail::match_insert_parser_backend(match_id, insert_id, value, true)) );
	}
	
	inline parser user_parser( user_parser_t P ) {
		return parser( detail::abstract_parser_backend_ptr(new detail::user_parser_backend(P)) );
	}
	
	inline parser guard(const parser& P) {
		return parser( detail::abstract_parser_backend_ptr(new detail::guard_parser_backend(P)) );
	}
	
	typedef std::stack<parseme_ptr> parseme_stack;
	inline parser store(parseme_stack& stack, const parser& p) {
		return parser( detail::abstract_parser_backend_ptr(new detail::stack_store_parser_backend(stack, p)) );
	}
	
	inline parser retrieve(parseme_stack& stack) {
		return parser( detail::abstract_parser_backend_ptr(new detail::stack_retrieve_parser_backend(stack)) );
	}
	
	struct immediate {
		parseme_ptr_ref N;
		immediate(parseme_ptr_ref N) : N(N) {}
		parse_results_t operator ()(const parser& P);
	};
	
	struct assigner_t {
		parseme_ptr parent;
		parseme_container& container;
		parseme_container::iterator position;
		assigner_t(const_parseme_ptr_ref parent, parseme_container& container, parseme_container::iterator position) 
			: parent(parent), container(container), position(position) {}
		parse_results_t operator ()(const parser& P);
	};
	
	struct stack_assigner_t {
		std::stack<parseme_ptr>& stack;
		stack_assigner_t(std::stack<parseme_ptr>& stack) : stack(stack) {}
		parse_results_t operator ()(const parser& P);
	};
	
	struct rewriter_t {
		sooty::parseme_ptr_ref n;
		rewriter_t(sooty::parseme_ptr_ref n) : n(n) {}
		parse_results_t operator [](const parser& P);
	};
	
	inline rewriter_t rewrite_branch(sooty::parseme_ptr_ref n) {
		return rewriter_t(n);
	}
	
	inline assigner_t immediate_assign(parseme_container& container) {
		return assigner_t(parseme_ptr(), container, container.end());
	}
	
	inline assigner_t immediate_assign(parseme_container& container, sooty::parseme_container::iterator i) {
		return assigner_t(parseme_ptr(), container, i);
	}
	
	inline assigner_t immediate_assign(const_parseme_ptr_ref parent, parseme_container& container, sooty::parseme_container::iterator i) {
		return assigner_t(parent, container, i);
	}
	
	
	
	#define SOOTY_FUNCTOR(name, argcount, auxcount) \
		ATMA_FUNCTOR(name, argcount, auxcount, void, (parse_results_t& R))
	
	
//=====================================================================
} // namespace sooty
//=====================================================================
#endif
//=====================================================================
