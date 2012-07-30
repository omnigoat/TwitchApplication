#include <sooty/frontend/lexical_analysis/lexeme.hpp>
//====================================================================
#include <atma/assert.hpp>
#include <atma/unittest/unittest.hpp>
//====================================================================
using namespace atma;
//====================================================================


sooty::lexeme::lexeme(size_t id, const sooty::multichannel& channel, size_t row, size_t column, size_t pos, const value_t& value)
 : id(id), channel(channel), position(row, column, pos), value(value)
{
}

TEST(lexeme_test)
{
	//ATMA_ASSERT(false);
	
	sooty::lexeme test_lexeme(12, sooty::channel(4), 3, 4, 8, std::string("giraffe"));
	
	CHECK_EQUAL(test_lexeme.id, 12);
	CHECK_EQUAL(test_lexeme.position.row, 3);
	CHECK_EQUAL(test_lexeme.position.column, 4);
	CHECK_EQUAL(test_lexeme.position.stream, 8);
	CHECK_EQUAL(test_lexeme.value.string, "giraffe");
	CHECK(test_lexeme.channel.contains(sooty::channel(4)));
}
