//=====================================================================
//
//
//
//=====================================================================
#ifndef ATMA_TIME_HPP
#define ATMA_TIME_HPP
//=====================================================================
namespace atma {
//=====================================================================

	typedef unsigned long long time_t;
	
	inline time_t time();
	inline double convert_time_to_seconds(const time_t&);
	
//=====================================================================
} // namespace atma
//=====================================================================
#include <atma/config/platform.hpp>
//=====================================================================
#if defined(ATMA_PLATFORM_WIN32)
#	include <atma/time/detail/win32/time_impl.hpp>
#endif
//=====================================================================
#endif // inclusion guard
//=====================================================================