//=====================================================================
//
//
//
//
//
//=====================================================================
#ifndef ATMA_LEXER_LEXEME_LIST_HPP
#define ATMA_LEXER_LEXEME_LIST_HPP
//=====================================================================
#include <sooty/frontend/lexical_analysis/lexeme_iterator.hpp>
//=====================================================================
namespace sooty {
//=====================================================================
	
	
	//=====================================================================
	//
	//  a list of lexemes. image it as a std::list<lexeme> with some added
	//  functionality in the iterators.
	//
	//=====================================================================
	class lexeme_list
	{
	public:
		typedef detail::lexeme_iterator<lexeme> iterator;
		typedef detail::lexeme_iterator<const lexeme> const_iterator;
		
	private:
		// we don't want to do the hard work ourselves!
		mutable lexeme_container_type lexemes_;
	
	public:
		size_t size();
		
		// push-back
		void push_back(const lexeme& token);
		iterator erase(const const_iterator&, const const_iterator&);
		
		// insertion ?? even used ??
		iterator insert(const iterator& before, const lexeme& token);
		
		// begin / end
		iterator begin(const multichannel& channel = any_channel);
		iterator end(const multichannel& channel = any_channel);
		const_iterator begin(const multichannel& channel = any_channel) const;
		const_iterator end(const multichannel& channel = any_channel) const;
		
		lexeme& operator [](int i) {
			return lexemes_[i];
		}
		
		const lexeme& operator [](int i) const {
			return lexemes_[i];
		}
	};
	
//=====================================================================
} // namespace sooty
//=====================================================================
#endif
//=====================================================================
