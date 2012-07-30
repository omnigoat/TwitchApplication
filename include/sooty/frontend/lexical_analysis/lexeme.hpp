//=====================================================================
//
//  lexeme
//  --------
//    During lexical analysis, we'll be required to store "packets" of
//  information regarding symbols we're lexing. These are known as
//  lexemes. They're quite simple little things. Note that we don't
//  support unicode in the source code. Probably never will, unless this
//  thing really takes off.
//
//=====================================================================
#ifndef ATMA_SOOTY_LEXER_LEXEME_HPP
#define ATMA_SOOTY_LEXER_LEXEME_HPP
//=====================================================================
#include <string>
//=====================================================================
#include <sooty/sooty.hpp>
#include <sooty/common/value_t.hpp>
#include <sooty/frontend/lexical_analysis/channel.hpp>
//=====================================================================
namespace sooty {
//=====================================================================
	
	namespace detail {
		inline size_t& guid() {
			static size_t _ = 0;
			return _;
		}
		
		inline void reset_guid() {
			guid() = 0;
		}
		
		inline size_t generate_guid() {
			return guid()++;
		}
	}
	
	//=====================================================================
	// position in the file
	//=====================================================================
	struct lexical_position
	{
		lexical_position()
			: row(), column(), stream(), guid_(detail::generate_guid())
		{
		}
		
		lexical_position(size_t row, size_t column, size_t stream)
			: row(row), column(column), stream(stream), guid_(detail::generate_guid())
		{
		}
		
		lexical_position(const lexical_position& rhs)
			: row(rhs.row), column(rhs.column), stream(rhs.stream), guid_(detail::generate_guid())
		{
		}
		
		// row and column are pretty obvious, but stream is the position in terms
		// of one big "stream" of lexemes - where they are lexed sequentially
		size_t row, column, stream;
		
		lexical_position& operator = (const lexical_position& rhs) {
			row = rhs.row;
			column = rhs.column;
			stream = rhs.stream;
			guid_ = detail::generate_guid();
			return *this;
		}
			
	private:
		// global-unique-identifier. useful because we synthesize new things sometimes.
		size_t guid_;
	public:
		size_t guid() const { return guid_; }
	};
	
	inline std::ostream& operator << (std::ostream& lhs, const lexical_position& rhs)
	{
		lhs << rhs.row << ":" << rhs.column;
		return lhs;
	}
	
	
	//=====================================================================
	// lexeme
	//=====================================================================
	struct lexeme
	{
		// our constructor!
		lexeme(size_t id, const sooty::multichannel& channel, size_t row, size_t column, size_t pos, const value_t& value);
		
		size_t id;
		multichannel channel;
		lexical_position position;
		value_t value;
		
		// pretty sure there's no comparable thing for this.
	private:
		bool operator == (const lexeme& rhs) const;
	};

//=====================================================================
} // namespace sooty
//=====================================================================
#endif
//=====================================================================

