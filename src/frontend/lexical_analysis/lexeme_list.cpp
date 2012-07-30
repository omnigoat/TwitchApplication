#include <atma/assert.hpp>
#include <sooty/frontend/lexical_analysis/lexeme_list.hpp>
#include <atma/unittest/unittest.hpp>

using namespace sooty;

size_t lexeme_list::size()
{
	return lexemes_.size();
}

void lexeme_list::push_back(const lexeme& token)
{
	lexemes_.push_back(token);
}


sooty::lexeme_list::iterator lexeme_list::insert(const iterator& before, const lexeme& token)
{
	return iterator(&lexemes_, lexemes_.insert(before.iter_, token), sooty::any_channel);
}

lexeme_list::iterator lexeme_list::begin(const multichannel& channel)
{
	lexeme_container_type::iterator start = lexemes_.begin();

	if (!channel.contains(any_channel))
		while (start != lexemes_.end() && !channel.contains(start->channel))
			++start;

	// return iterator!
	return iterator(&lexemes_, start, channel);
}

lexeme_list::iterator lexeme_list::end(const multichannel& channel)
{
	if (lexemes_.empty()) return iterator(&lexemes_, lexemes_.end(), sooty::any_channel);
	
	// begin at the end
	lexeme_container_type::reverse_iterator current = lexemes_.rbegin();
	// advance 'till match only if we're looking for a specific channel
	if (channel != any_channel) {
		while (current != lexemes_.rend() && !current->channel.contains(channel))
			++current;
	}
	
	// return iterator!
	return iterator(&lexemes_, current == lexemes_.rend() ? lexemes_.end() : current.base(), channel);
}

lexeme_list::const_iterator lexeme_list::begin(const multichannel& channel) const
{
	lexeme_container_type::iterator start = lexemes_.begin();

	if (!channel.contains(any_channel))
		while (start != lexemes_.end() && !channel.contains(start->channel))
			++start;

	// return iterator!
	return const_iterator(&lexemes_, start, channel);
}

lexeme_list::const_iterator lexeme_list::end(const multichannel& channel) const
{
	if (lexemes_.empty()) return iterator(&lexemes_, lexemes_.end(), sooty::any_channel);
	
	// begin at the end
	lexeme_container_type::iterator start = lexemes_.end();
	// advance 'till match only if we're looking for a specific channel
	if (channel != any_channel)
		do { --start; } 
			while (!start->channel.contains(channel) && start != lexemes_.begin());
	// return iterator!
	return const_iterator(&lexemes_, start, channel);
}

sooty::lexeme_list::iterator sooty::lexeme_list::erase( const const_iterator& begin, const const_iterator& end )
{
	return iterator(&lexemes_, lexemes_.erase(begin.iter_, end.iter_), sooty::any_channel);
}







SUITE(lexeme_list_test_suite)
{
	struct lexeme_list_test_fixture
	{
		channel code;
		channel whitespace;
		channel nonshared;
		
		lexeme_list_test_fixture()
			: 
			code(2),
			whitespace(3),
			nonshared(4),
			L1(1, code, 1, 1, 1, std::string("giraffe")),
			L2(2, whitespace, 1, 1, 1, std::string("elephant")),
			L3(3, code, 1, 1, 1, std::string("zebra"))
		{
			default_test_list.push_back(L1);
			default_test_list.push_back(L2);
			default_test_list.push_back(L3);
		}

		lexeme L1, L2, L3;

		lexeme_list default_test_list;
		lexeme_list empty_list;
	};
	
	TEST_FIXTURE(lexeme_list_test_fixture, iterators_equality)
	{
		lexeme_list::iterator i = default_test_list.begin();
		lexeme_list::const_iterator ci = i;
		
		CHECK(ci == i);
		CHECK(i == ci);
	}
	
	TEST_FIXTURE(lexeme_list_test_fixture, begin_end_nonfound)
	{
		//ATMA_HALT("yup");
		lexeme_list::iterator i = default_test_list.end(nonshared);
		CHECK(i == default_test_list.end());
		i = empty_list.end(code);
		CHECK(i == empty_list.end());
	}
	
	TEST_FIXTURE(lexeme_list_test_fixture, lexeme_list_iteration_current)
	{
		// i skip L2 because we are in the "code" stream
		lexeme_list::const_iterator i = default_test_list.begin(code);
		++i;
		CHECK(L3.value.string == i->value.string);
		--i;
		CHECK(L1.value.string == i->value.string);
	}
	
	TEST_FIXTURE(lexeme_list_test_fixture, lexeme_list_iteration_any)
	{
		// i should be L2, since we're advancing on any channel
		lexeme_list::const_iterator i = default_test_list.begin(any_channel);
		++i;
		CHECK(L2.value.string == i->value.string);
	}
	
	TEST_FIXTURE(lexeme_list_test_fixture, pushback_insertion)
	{
		lexeme_list test_list;
		
		test_list.push_back(L1);
		test_list.push_back(L2);
		test_list.insert(test_list.begin(), L3);
		
		// check size now
		CHECK(3 == test_list.size());
		
		// check order (L3, L1, L2)
		lexeme_list::iterator i = test_list.begin();
		CHECK(3 == i->id);
		i.increment(any_channel);
		CHECK(1 == i->id);
		i.increment(any_channel);
		CHECK(2 == i->id);
	}
	
	TEST_FIXTURE(lexeme_list_test_fixture, movement)
	{
		lexeme_list::const_iterator i = default_test_list.begin();
		i.move(1);
		CHECK(3 == i->id);
		i.move(-1, any_channel);
		CHECK(2 == i->id);
		
		//CHECK(false);
	}
	
	TEST_FIXTURE(lexeme_list_test_fixture, begin_end)
	{
		lexeme_list::iterator begin = default_test_list.begin();
		lexeme_list::iterator end = default_test_list.end();
		CHECK( begin != end );
		lexeme_list::const_iterator cbegin = default_test_list.begin();
		lexeme_list::const_iterator cend = default_test_list.end();
		CHECK( cbegin != cend );
	}
}




