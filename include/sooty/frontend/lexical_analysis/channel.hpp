//=====================================================================
//
//  channel
//  ---------
//
//=====================================================================
#ifndef ATMA_SOOTY_LEXER_CHANNEL_HPP
#define ATMA_SOOTY_LEXER_CHANNEL_HPP
//=====================================================================
#include <sooty/sooty.hpp>
//=====================================================================
namespace sooty {
//=====================================================================

	//=====================================================================
	// forward declares
	//=====================================================================
	class channel;
	class multichannel;
	
	
	//=====================================================================
	// one channel!
	//=====================================================================
	class channel
	{
		friend class multichannel;
		size_t value_;
		
	public:
		channel();
		channel(const size_t& value);
		
		friend bool operator == (const channel& lhs, const channel& rhs);
		friend multichannel operator | (const channel& lhs, const channel& rhs);
		friend multichannel operator | (const multichannel& lhs, const channel& rhs);
		friend multichannel operator | (const channel& lhs, const multichannel& rhs);
	};
	
	bool operator == (const channel&, const channel&);
	bool operator != (const channel&, const channel&);

	
	//=====================================================================
	// multiple channels!
	//=====================================================================
	class multichannel
	{
		size_t value_;
	
	private:
		multichannel(const size_t& value);
		
	public:
		multichannel(const channel& c);
		
		bool contains(const channel&) const;
		bool contains(const multichannel&) const;
		
		friend bool operator == (const multichannel&, const multichannel&);
		
		friend multichannel operator | (const channel& lhs, const channel& rhs);
		friend multichannel operator | (const multichannel& lhs, const channel& rhs);
		friend multichannel operator | (const channel& lhs, const multichannel& rhs);
	};
	
	bool operator == (const multichannel&, const multichannel&);
	bool operator != (const multichannel&, const multichannel&);
	
	multichannel operator | (const channel& lhs, const channel& rhs);
	multichannel operator | (const multichannel& lhs, const channel& rhs);
	multichannel operator | (const channel& lhs, const multichannel& rhs);
	
	
	
	//=====================================================================
	// two defaults
	//=====================================================================
	extern channel any_channel;
	extern channel current_channel;
	
//=====================================================================
} // namespace sooty
//=====================================================================
#endif
//=====================================================================

