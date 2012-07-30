#include <sooty/frontend/lexical_analysis/lexer.hpp>
//=====================================================================
#include <atma/assert.hpp>
#include <atma/unittest/unittest.hpp>
//=====================================================================
using namespace sooty;

//=====================================================================
// lexer
//=====================================================================
lexer::lexer( detail::abstract_lexer_backend_ptr backend ) : backend_(backend)
{

}

lex_results_t lexer::operator ()(const input_iterator& begin, const input_iterator& end) const
{
	return (*backend_)(begin, end);
}

lexer lexer::operator & ( const lexer& rhs )
{
	return lexer( detail::abstract_lexer_backend_ptr(new detail::and_lexer_backend(*this, rhs)) );
}

lexer lexer::operator | ( const lexer& rhs )
{
	return lexer( detail::abstract_lexer_backend_ptr(new detail::or_lexer_backend(*this, rhs)) );
}

lexer lexer::operator >> ( const lexer& rhs )
{
	return operator && (rhs);
}

lexer lexer::operator && ( const lexer& rhs )
{
	return lexer( detail::abstract_lexer_backend_ptr(new detail::seq_and_lexer_backend(*this, rhs)) );
}

lexer lexer::operator * ()
{
	return lexer( detail::abstract_lexer_backend_ptr(new detail::zero_or_more_backend(*this)) );
}

lexer lexer::operator + ()
{
	return lexer( detail::abstract_lexer_backend_ptr(new detail::one_or_more_backend(*this)) );
}

lexer lexer::operator ! ()
{
	return lexer( detail::abstract_lexer_backend_ptr(new detail::one_or_zero_backend(*this)) );
}

lexer lexer::operator [] ( const detail::userfunc_t& userfunc)
{
	return lexer( detail::abstract_lexer_backend_ptr(new detail::func_backend(*this, userfunc)) );
}


sooty::lexer sooty::lexer::operator ~()
{
	return lexer( detail::abstract_lexer_backend_ptr(new detail::not_lexer_backend(*this)) );
}

//=====================================================================
// abstract lexer backends
//=====================================================================
sooty::lex_results_t sooty::detail::eof_lexer_backend::operator ()( const input_iterator& begin, const input_iterator& end) const
{
	if (begin == end)
		return lex_results_t(true, false, true, 0, begin, begin);
	else
		return lex_results_t(false, false, false, 0, begin, begin);
}

lex_results_t detail::any_lexer_backend::operator ()(const input_iterator& begin, const input_iterator& end) const
{
	if (begin == end) return lex_results_t(false, false, false, 0, begin, begin);
	char c = *begin;
	input_iterator stop = begin;
	++stop;
	return lex_results_t(true, true, stop == end, 1, begin, stop);
}

lex_results_t detail::char_lexer_backend::operator ()(const input_iterator& begin, const input_iterator& end) const
{
	if (begin != end && *begin == c) {
		input_iterator stop = begin;
		++stop;
		return lex_results_t(true, true, stop == end, 1, begin, stop);
	}
	return lex_results_t(false, false, false, 0, begin, begin);
}

lex_results_t detail::string_lexer_backend::operator ()(const input_iterator& begin, const input_iterator& end) const
{
	std::string::const_iterator current_char = str.begin();
	input_iterator current_input = begin;
	while ( current_char != str.end() )
	{
		if (current_input == end)
			return lex_results_t(false, false, false, 0, begin, begin);
		else if (*current_input != *current_char)
			return lex_results_t(false, false, false, 0, begin, begin);
		
		// we need to early-out to stop incrementing current_input
		++current_char;
		++current_input;
	}
	
	
	return lex_results_t(true, true, current_input == end, str.size(), begin, current_input);
}

lex_results_t detail::any_of_lexer_backend::operator ()(const input_iterator& begin, const input_iterator& end) const
{
	if (begin == end) return lex_results_t(false, false, false, 0, begin, begin);
	
	for (std::string::const_iterator i = str.begin(); i != str.end() && begin != end; ++i)
	{
		if (*i == *begin) {
			input_iterator stop = begin;
			++stop;
			return lex_results_t(true, true, stop == end, 1, begin, stop);
		}
	}
	
	return lex_results_t(false, false, false, 0, begin, begin);
}


lex_results_t detail::in_range_lexer_backend::operator ()(const input_iterator& begin, const input_iterator& end) const
{
	if (begin != end && *begin >= L && *begin <= R) {
		input_iterator stop = begin;
		++stop;
		return lex_results_t(true, true, stop == end, 1, begin, stop);
	}
	return lex_results_t(false, false, false, 0, begin, begin);
}


lex_results_t detail::and_lexer_backend::operator ()(const input_iterator& begin, const input_iterator& end) const
{
	lex_results_t fr = first(begin, end);
	if (!fr.pass) return lex_results_t(false, false, false, 0, begin, begin);
	lex_results_t sr = second(begin, end);
	if (!sr.pass) return lex_results_t(false, false, false, 0, begin, begin);
	return lex_results_t(fr.pass && sr.pass, fr.hit || sr.hit, fr.full && sr.full, std::max(fr.length, sr.length), begin,
		fr.length > sr.length ? fr.stop : sr.stop);
}


lex_results_t detail::or_lexer_backend::operator ()(const input_iterator& begin, const input_iterator& end) const
{
	lex_results_t fr = lhs(begin, end);
	if (fr.pass) return fr;
	lex_results_t sr = rhs(begin, end);
	if (sr.pass) return sr;
	return lex_results_t(false, false, false, 0, begin, begin);
}


lex_results_t detail::seq_and_lexer_backend::operator ()(const input_iterator& begin, const input_iterator& end) const
{
	lex_results_t fr = first(begin, end);
	if (!fr.pass) return lex_results_t(false, false, false, 0, begin, begin);
	lex_results_t sr = second(fr.stop, end);
	if (!sr.pass) return lex_results_t(false, false, false, 0, begin, begin);
	return lex_results_t(true, true, fr.full && sr.full, fr.length + sr.length, begin, sr.stop);
}


sooty::lex_results_t sooty::detail::one_or_zero_backend::operator ()(const input_iterator& begin, const input_iterator& end) const
{
	lex_results_t fr = L(begin, end);
	fr.pass = true;
	return fr;
}

lex_results_t detail::zero_or_more_backend::operator ()(const input_iterator& begin, const input_iterator& end) const
{
	lex_results_t result(true, false, false, 0, begin, begin);
	
	for (;;)
	{
		lex_results_t temp_result = L(result.stop, end);
		if (!temp_result.hit) break;
		result.length += temp_result.length;
		result.stop = temp_result.stop;
		result.hit = result.hit || temp_result.hit;
	}
	result.full = result.stop == end;
	return result;
}

lex_results_t detail::one_or_more_backend::operator ()(const input_iterator& begin, const input_iterator& end) const
{
	lex_results_t result = L(begin, end);
	if (!result.pass) return lex_results_t(false, false, false, 0, begin, begin);
	
	for (;;)
	{
		lex_results_t temp_result = L(result.stop, end);
		if (!temp_result.hit) break;
		result.length += temp_result.length;
		result.stop = temp_result.stop;
	}
	result.full = result.stop == end;
	return result;
}


sooty::lex_results_t sooty::detail::not_lexer_backend::operator()( const input_iterator& begin, const input_iterator& end ) const
{
	lex_results_t result = L(begin, end);
	result.pass = !result.pass;
	return result;
}

sooty::lex_results_t sooty::detail::func_backend::operator()( const input_iterator& begin, const input_iterator& end ) const
{
	lex_results_t R = L(begin, end);
	ATMA_ASSERT(func);
	if (R.pass)
		func(R);
	return R;
}

//std::string detail::required_star_backend::match( std::istream& ) const
//{
//	std::string final_result;
//	std::string result;
//	while ( (result = L(input)) != std::string() )
//		final_result += result;
//	return final_result;
//}





//=====================================================================
// test to make sure that
//  a) things match as expected
//  b) if things don't match, they leave the stream unchanged
//=====================================================================
SUITE(lexer_backend_behaviour)
{
	struct fixture
	{
		std::string ss;
		bool action_activated;
		
		fixture() : ss("abelephant"), action_activated(false)
		{
		}
		
		template <bool ShouldPass, bool ShouldHit, bool ShouldFull, int Length> bool test(const lexer& L)
		{
			input_iterator begin = input_iterator(ss.begin());
			input_iterator end = input_iterator(ss.end());
			
			lex_results_t R = L(begin, end);
			return (R.pass == ShouldPass) && (R.hit == ShouldHit) && (R.full == ShouldFull) 
				&& (R.length == Length) && (R.stop == input_iterator(ss.begin() + Length));
		}
		
		void action(lex_results_t& R) 
		{
			action_activated = true;
		}
	};
	
	TEST_FIXTURE(fixture, any_)
	{
		CHECK(( test<true, true, false, 1>( any() ) ));
	}
	
	TEST_FIXTURE(fixture, char_)
	{
		CHECK(( test<true, true, false, 1>( char_('a') ) ));
		CHECK(( test<false, false, false, 0>( char_('z') ) ));
	}
	
	TEST_FIXTURE(fixture, in_range)
	{
		CHECK(( test<true, true, false, 1>( in_range('a', 'z') ) ));
		CHECK(( test<false, false, false, 0>( in_range('w', 'z') ) ));
	}
	
	TEST_FIXTURE(fixture, any_of)
	{
		CHECK(( test<true, true, false, 1>(any_of("poiuaer")) ));
		CHECK(( test<false, false, false, 0>(any_of("qpoeiwuruuuuu")) ));
	}

	TEST_FIXTURE(fixture, string_)
	{
		CHECK(( test<true, true, true, 10>(string_("abelephant")) ));
		CHECK(( test<true, true, false, 9>(string_("abelephan")) ));
		CHECK(( test<false, false, false, 0>(string_("abelezannt")) ));
		CHECK(( test<false, false, false, 0>(string_("abelephantf")) ));
	}
	
	TEST_FIXTURE(fixture, and_)
	{
		CHECK(( test<true, true, false, 1>(char_('a') & in_range('a', 'c')) ));
		CHECK(( test<false, false, false, 0>(char_('d') & char_('a')) ));
	}
	
	TEST_FIXTURE(fixture, or_)
	{
		CHECK(( test<true, true, false, 1>(char_('a') | char_('z')) ));
		CHECK(( test<true, true, false, 1>(char_('z') | char_('a')) ));
		CHECK(( test<true, true, false, 1>(char_('a') | char_('a')) ));
		CHECK(( test<false, false, false, 0>(char_('z') | char_('z')) ));
	}
	
	TEST_FIXTURE(fixture, sequence_and)
	{
		CHECK(( test<true, true, false, 2>(char_('a') >> in_range('a', 'c')) ));
		CHECK(( test<false, false, false, 0>(char_('d') >> char_('a')) ));
		CHECK(( test<false, false, false, 0>(char_('a') >> char_('z')) ));
	}
	
	TEST_FIXTURE(fixture, one_or_zero)
	{
		CHECK(( test<true, true, false, 1>(!char_('a')) ));
		// a failed optional is still a pass
		CHECK(( test<true, false, false, 0>(!char_('d')) ));
	}
	
	TEST_FIXTURE(fixture, zero_or_more)
	{
		CHECK(( test<true, true, true, 10>(*in_range('a', 'z')) ));
		CHECK(( test<true, true, false, 5>(*in_range('a', 'l')) ));
		CHECK(( test<true, false, false, 0>(*in_range('y', 'z')) ));
	}
	
	TEST_FIXTURE(fixture, one_or_more)
	{
		CHECK(( test<true, true, true, 10>(+in_range('a', 'z')) ));
		CHECK(( test<true, true, false, 5>(+in_range('a', 'l')) ));
		CHECK(( test<false, false, false, 0>(+in_range('y', 'z')) ));
	}
	
	TEST_FIXTURE(fixture, actions)
	{
		//test<true, false, 1>(char_('a')[boost::bind(&fixture::action, this, _1)]);
		lexer K = char_('a');
		lexer L = char_('b');
		test<true, true, false, 1>( *(any() >> (K[boost::bind(&fixture::action, this, _1)] | L[boost::bind(&fixture::action, this, _1)])) );
		CHECK(( action_activated ));
	}
}
