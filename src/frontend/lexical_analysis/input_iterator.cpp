#include <sooty/frontend/lexical_analysis/input_iterator.hpp>
//=====================================================================
#include <atma/unittest/unittest.hpp>
//=====================================================================
using namespace sooty;
//=====================================================================


sooty::input_iterator::input_iterator()
{

}

sooty::input_iterator::input_iterator( const sooty::input_iterator& rhs )
	: backend( rhs.backend->clone() )
{

}


input_iterator& sooty::input_iterator::operator=( const input_iterator& rhs )
{
	backend.reset( rhs.backend->clone() );
	return *this;
}


input_iterator& sooty::input_iterator::operator++()
{
	backend->increment();
	return *this;
}

sooty::input_iterator sooty::input_iterator::operator++( int )
{
	input_iterator L(*this);
	++*this;
	return L;
}

char sooty::input_iterator::operator*() const
{
	return backend->dereference();
}

bool sooty::input_iterator::operator==( const input_iterator& rhs ) const
{
	return backend->equality(rhs.backend.get());
}

bool sooty::input_iterator::operator!=( const input_iterator& rhs ) const
{
	return !operator == (rhs);
}

std::string sooty::input_iterator::make_string( const input_iterator& rhs ) const
{
	return backend->stringize(rhs.backend.get());
}




SUITE(input_iterator)
{
	struct fixture
	{
		std::string S;
		std::string::const_iterator sbegin;
		std::string::const_iterator ssecond;
		std::string::const_iterator send;

		fixture() : S("giraffe"), sbegin(S.begin()), ssecond(++S.begin()), send(S.end())
		{
		}
	};
	
	TEST_FIXTURE(fixture, equality_operator)
	{
		input_iterator lhs(sbegin);
		input_iterator rhs(sbegin);
		CHECK(lhs == rhs);
	}

	TEST_FIXTURE(fixture, inequality_operator)
	{
		input_iterator lhs(sbegin);
		input_iterator rhs(send);
		CHECK(lhs != rhs);
	}
	
	TEST_FIXTURE(fixture, nonmodification_of_original)
	{
		input_iterator iter(sbegin);
		++iter;
		CHECK(iter != input_iterator(sbegin));
	}
	
	TEST_FIXTURE(fixture, correct_modification_of_copy)
	{
		input_iterator iter(sbegin);
		++iter;
		CHECK(iter == input_iterator(sbegin + 1));
	}
	
	TEST_FIXTURE(fixture, copy_constructor)
	{
		input_iterator iter(sbegin);
		input_iterator me_too(iter);
		CHECK(iter == me_too);
		++iter;
		CHECK(iter != me_too);
	}
	
	TEST_FIXTURE(fixture, assignment)
	{
		input_iterator iter(sbegin);
		input_iterator me_too;
		me_too = iter;
		CHECK(iter == me_too);
		++iter;
		CHECK(iter != me_too);
	}
	
	TEST_FIXTURE(fixture, increment_operator)
	{
		input_iterator begin(sbegin);
		input_iterator second(ssecond);
		++begin;
		CHECK(begin == second);
	}
	
	TEST_FIXTURE(fixture, dereference_operator)
	{
		input_iterator begin(sbegin);
		CHECK_EQUAL(*begin, *sbegin);
	}
	
	
}

