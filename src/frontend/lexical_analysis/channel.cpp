#include <sooty/frontend/lexical_analysis/channel.hpp>
#include <atma/assert.hpp>
#include <atma/unittest/unittest.hpp>
//====================================================================
using namespace sooty;
//====================================================================
channel sooty::any_channel(0);
channel sooty::current_channel(31);
//====================================================================


channel::channel()
 : value_()
{
}

channel::channel(const size_t& value)
 : value_(1 << value)
{
}

bool sooty::operator == (const channel& lhs, const channel& rhs)
{
	return lhs.value_ == rhs.value_;
}

bool sooty::operator != (const channel& lhs, const channel& rhs)
{
	return !operator == (lhs, rhs);
}

sooty::multichannel::multichannel( const size_t& value )
 : value_(value)
{

}

sooty::multichannel::multichannel( const channel& c )
 : value_(c.value_)
{

}

bool sooty::multichannel::contains(const channel& c) const
{
	return (value_ & c.value_) != 0;
}

bool sooty::multichannel::contains(const multichannel& m) const
{
	return (value_ & m.value_) != 0;
}

bool sooty::operator == (const multichannel& lhs, const multichannel& rhs)
{
	return lhs.value_ == rhs.value_;
}

bool sooty::operator != (const multichannel& lhs, const multichannel& rhs)
{
	return !operator == (lhs, rhs);
}

multichannel sooty::operator | (const channel& lhs, const channel& rhs)
{
	return multichannel(lhs.value_ | rhs.value_);
}

multichannel sooty::operator | (const multichannel& lhs, const channel& rhs)
{
	return multichannel(lhs.value_ | rhs.value_);
}

multichannel sooty::operator | (const channel& lhs, const multichannel& rhs)
{
	return multichannel(lhs.value_ | rhs.value_);
}


SUITE(channel_suite)
{
	TEST(reserved_channels_are_actually_reserved)
	{
		// should throw an assert
		channel bad_channel_one(0);
		// should also throw an assert
		channel bad_channel_two(31);
	}

	TEST(equality_between_channels)
	{
		channel c1(4), c2(4);
		CHECK(c1 == c2);
	}

	TEST(equality_between_multichannels_and_channels)
	{
		channel c1(4), c2(13), c3(24);
		multichannel m = c1 | c2;
		
		CHECK(m.contains(c1));
		CHECK(m.contains(c2));
		CHECK(!m.contains(c3));
	}
}