//=====================================================================
//
//
//
//=====================================================================
#ifndef ATMA_SOOTY_COMMON_VALUE_T_HPP
#define ATMA_SOOTY_COMMON_VALUE_T_HPP
//=====================================================================
#include <string>
//=====================================================================
#include <sooty/sooty.hpp>
#include <sooty/frontend/lexical_analysis/channel.hpp>
//=====================================================================
namespace sooty {
//=====================================================================

	//=====================================================================
	//
	//=====================================================================
	struct value_t
	{
		int integer;
		float real;
		std::string string;
		
		value_t() : integer(), real() {}
		value_t(int a) : integer(a), real() {}
		value_t(float b) : integer(), real(b) {}
		value_t(const std::string& c) : integer(), real(), string(c) {}
		value_t(const char* c) : integer(), real(), string(c) {}
		value_t(const std::string& string, int integer) : string(string), integer(integer), real() {}
		
		bool operator == (const value_t& rhs) const
		{
			return integer == rhs.integer && real == rhs.real && string == rhs.string;
		}
		
		bool operator != (const value_t& rhs) const {
			return !operator == (rhs);
		}
		
		const value_t& operator = (const std::string& rhs) {
			*this = value_t(rhs);
			return *this;
		}
		
		const value_t& operator = (int rhs) {
			*this = value_t(rhs);
			return *this;
		}
		
		const value_t& operator = (float rhs) {
			*this = value_t(rhs);
			return *this;
		}
	};

//=====================================================================
} // namespace sooty
//=====================================================================
#endif
//=====================================================================

