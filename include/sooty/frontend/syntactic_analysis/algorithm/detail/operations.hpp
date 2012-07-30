//=====================================================================
//
//
//=====================================================================
#ifndef SOOTY_ALGORITHM_OPERATIONS_HPP
#define SOOTY_ALGORITHM_OPERATIONS_HPP
//=====================================================================
#include <boost/function.hpp>
//=====================================================================
#include <sooty/sooty.hpp>
#include <sooty/frontend/syntactic_analysis/parseme.hpp>
//=====================================================================
namespace sooty {
namespace detail {
//=====================================================================

	//=====================================================================
	// for-each
	//=====================================================================
	template <typename T>
	struct for_each_t
	{
		T func;
		for_each_t( T& func) : func(func) {}
		state::Enum operator ()(sooty::parseme_ptr_ref N) {
			func(N);
			return state::keep_going;
		}
	};

	template <typename T>
	for_each_t<T> for_each(T func) {
		return for_each_t<T>(func);
	};
	

	//=====================================================================
	// find-first-if
	//=====================================================================
	template <typename T>
	struct find_first_t
	{
		sooty::parseme_ptr_ref output;
		T pred;
		find_first_t(sooty::parseme_ptr_ref output, T pred) : output(output), pred(pred) {}
		state::Enum operator ()(sooty::const_parseme_ptr_ref N) const
		{
			if (pred(N)) {
				output = N;
				return state::stop;
			}
			else {
				return state::keep_going;
			}
		}
	};
	
	template <typename T>
	find_first_t<T> find_first_if(sooty::parseme_ptr_ref output, T pred) {
		return find_first_t<T>(output, pred);
	}
	
	
	//=====================================================================
	// copy-if
	//=====================================================================
	template <typename IT, typename PR>
	struct copy_if_t
	{
		IT output;
		PR pred;
		copy_if_t(IT output, PR pred) : output(output), pred(pred) {}
		state::Enum operator ()(sooty::const_parseme_ptr_ref N) {
			if (pred(N))
				*output++ = N;
			return state::keep_going;
		}
	};
	
	template <typename IT, typename PR>
	copy_if_t<IT, PR> copy_if(IT output, PR pred) {
		return copy_if_t<IT, PR>(output, pred);
	}
	
	
//=====================================================================
} // namespace detail
} // namespace sooty
//=====================================================================
#endif
//=====================================================================

