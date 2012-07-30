//=====================================================================
//
//
//
//=====================================================================
#ifndef ATMA_SOOTY_FRONTEND_LEXICAL_ANALYSIS_LEXER_HPP
#define ATMA_SOOTY_FRONTEND_LEXICAL_ANALYSIS_LEXER_HPP
//=====================================================================
#include <string>
//=====================================================================
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
//=====================================================================
#include <sooty/sooty.hpp>
#include <sooty/frontend/lexical_analysis/input_iterator.hpp>
//=====================================================================
namespace sooty {
//=====================================================================

	
	
	
	//=====================================================================
	//
	//=====================================================================
	struct lex_results_t
	{
		bool pass;
		bool hit;
		bool full;
		size_t length;
		input_iterator start;
		input_iterator stop;
		
		lex_results_t(bool pass, bool hit, bool full, size_t length, const input_iterator& start, const input_iterator& stop)
			: pass(pass), hit(hit), full(full), length(length), start(start), stop(stop)
		{
		}
	};
	
	
	//=====================================================================
	// forward declares
	//=====================================================================
	class lexer;
	namespace detail {
		struct abstract_lexer_backend;
		typedef boost::shared_ptr<abstract_lexer_backend> abstract_lexer_backend_ptr;
	}
	
	
	//=====================================================================
	//
	//=====================================================================
	namespace detail
	{
		typedef boost::function<void (lex_results_t&)> userfunc_t;
	}
	
	//=====================================================================
	//
	//=====================================================================
	class lexer
	{
		detail::abstract_lexer_backend_ptr backend_;
		detail::userfunc_t userfunc_;
		
	public:
		lexer( detail::abstract_lexer_backend_ptr backend );
		
		// logical and/or
		lexer operator &  (const lexer&);
		lexer operator |  (const lexer&);
		
		// sequence and/or
		lexer operator >> (const lexer&);
		lexer operator && (const lexer&);
		lexer operator || (const lexer&);
		
		lexer operator *  ();
		lexer operator +  ();
		lexer operator !  ();
		lexer operator ~  ();
		lexer operator [] (const detail::userfunc_t&);
		
		lex_results_t operator ()(const input_iterator&, const input_iterator&) const;
	};

	

	
	//=====================================================================
	//
	//=====================================================================
	namespace detail
	{
		// base
		struct abstract_lexer_backend {
			virtual lex_results_t operator ()(const input_iterator&, const input_iterator&) const = 0;
		};
		
		
		// implementations
		struct eof_lexer_backend : abstract_lexer_backend {
			lex_results_t operator ()(const input_iterator&, const input_iterator&) const;
		};
		
		struct any_lexer_backend : abstract_lexer_backend {
			lex_results_t operator ()(const input_iterator&, const input_iterator&) const;
		};
		
		struct char_lexer_backend : abstract_lexer_backend {
			char c;
			char_lexer_backend(char c) : c(c) {}
			lex_results_t operator ()(const input_iterator&, const input_iterator&) const;
		};
		
		struct in_range_lexer_backend : abstract_lexer_backend {
			std::string::value_type L, R;
			in_range_lexer_backend(std::string::value_type L, std::string::value_type R) : L(L), R(R) {}
			lex_results_t operator ()(const input_iterator&, const input_iterator&) const;
		};
		
		struct any_of_lexer_backend : abstract_lexer_backend {
			std::string str;
			any_of_lexer_backend(const std::string& str) : str(str) {}
			lex_results_t operator ()(const input_iterator&, const input_iterator&) const;
		};
		
		struct string_lexer_backend : abstract_lexer_backend {
			std::string str;
			string_lexer_backend(const std::string& str) : str(str) {}
			lex_results_t operator ()(const input_iterator&, const input_iterator&) const;
		};
		
		struct and_lexer_backend : abstract_lexer_backend {
			lexer first, second;
			and_lexer_backend(const lexer& first, const lexer& second) : first(first), second(second) {}
			lex_results_t operator ()(const input_iterator&, const input_iterator&) const;
		};
		
		struct or_lexer_backend : abstract_lexer_backend {
			lexer lhs, rhs;
			or_lexer_backend(const lexer& lhs, const lexer& rhs) : lhs(lhs), rhs(rhs) {}
			lex_results_t operator ()(const input_iterator&, const input_iterator&) const;
		};
		
		struct seq_and_lexer_backend : abstract_lexer_backend {
			lexer first, second;
			seq_and_lexer_backend(const lexer& first, const lexer& second) : first(first), second(second) {}
			lex_results_t operator ()(const input_iterator&, const input_iterator&) const;
		};
		
		struct one_or_zero_backend : abstract_lexer_backend {
			lexer L;
			one_or_zero_backend(const lexer& L) : L(L) {}
			lex_results_t operator ()(const input_iterator&, const input_iterator&) const;
		};
		
		struct zero_or_more_backend : abstract_lexer_backend {
			lexer L;
			zero_or_more_backend(const lexer& L) : L(L) {}
			lex_results_t operator ()(const input_iterator&, const input_iterator&) const;
		};
		
		struct one_or_more_backend : abstract_lexer_backend {
			lexer L;
			one_or_more_backend(const lexer& L) : L(L) {}
			lex_results_t operator ()(const input_iterator&, const input_iterator&) const;
		};
		
		struct func_backend : abstract_lexer_backend {
			lexer L;
			userfunc_t func;
			func_backend(const lexer& L, const userfunc_t& func) : L(L), func(func) {}
			lex_results_t operator ()(const input_iterator&, const input_iterator&) const;
		};
		
		struct not_lexer_backend : abstract_lexer_backend {
			lexer L;
			not_lexer_backend(const lexer& L) : L(L) {}
			lex_results_t operator ()(const input_iterator&, const input_iterator&) const;
		};
	}
	
	inline lexer eof_()
		{ return lexer( detail::abstract_lexer_backend_ptr(new detail::eof_lexer_backend) ); }
	
	inline lexer any()
		{ return lexer( detail::abstract_lexer_backend_ptr(new detail::any_lexer_backend) ); }
	
	inline lexer char_(char c)
		{ return lexer( detail::abstract_lexer_backend_ptr(new detail::char_lexer_backend(c)) ); }
	
	inline lexer string_(const std::string& str)
		{ return lexer( detail::abstract_lexer_backend_ptr(new detail::string_lexer_backend(str)) ); }

	inline lexer any_of(const std::string& delims)
		{ return lexer( detail::abstract_lexer_backend_ptr(new detail::any_of_lexer_backend(delims)) ); }
	
	inline lexer in_range(char L, char R)
		{ return lexer( detail::abstract_lexer_backend_ptr(new detail::in_range_lexer_backend(L, R)) ); }

	
//=====================================================================
} // namespace sooty
//=====================================================================
#endif
//=====================================================================

