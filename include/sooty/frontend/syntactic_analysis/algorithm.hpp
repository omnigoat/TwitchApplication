//=====================================================================
//
//    NOTE: all of these algorithms are based off the 'scope tree'
//          concept. that is, when going up [sic], they will go up
//          the tree until they reach the root.
//
//=====================================================================
#ifndef SOOTY_ALGORITHM_HPP
#define SOOTY_ALGORITHM_HPP
//=====================================================================
#include <atma/assert.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/parseme.hpp>
#include <sooty/frontend/syntactic_analysis/algorithm/detail/algorithms.hpp>
#include <sooty/frontend/syntactic_analysis/algorithm/detail/operations.hpp>
#include <sooty/frontend/syntactic_analysis/algorithm/detail/depth.hpp>
//#include <sooty/frontend/syntactic_analysis/sandbox.hpp>
//=====================================================================
namespace sooty {
//=====================================================================
	
	//=====================================================================
	// direct_upwards_find_first_if
	//  - goes directly up the ancestry (parent to parent) until NULL
	//=====================================================================
	template <typename PR>
	sooty::parseme_ptr direct_upwards_find_first_if(sooty::const_parseme_ptr_ref starting_node, PR pred)
	{
		sooty::parseme_ptr P;
		detail::direct_upwards( starting_node, detail::find_first_if(P, pred) );
		return P;
	}
	
	template <typename IT, typename PR>
	void direct_upwards_copy_if(IT dest, sooty::const_parseme_ptr_ref starting_node, PR pred)
	{
		detail::direct_upwards( starting_node, detail::copy_if(dest, pred) );
	}
	
	//=====================================================================
	// upwards_bredth_first_copy_if:
	//=====================================================================
	template <typename IT, typename PR>
	void upwards_bredth_first_copy_if(sooty::const_parseme_ptr_ref starting_node, IT dest, PR pred)
	{
		detail::upwards_bredth_first(starting_node, detail::copy_if(dest, pred), detail::all_levels());
	}
	
	template <typename IT, typename PR, typename DT>
	void upwards_bredth_first_copy_if(sooty::const_parseme_ptr_ref starting_node, IT dest, PR pred, DT depther)
	{
		detail::upwards_bredth_first(starting_node, detail::copy_if(dest, pred), detail::all_levels());
	}
	
	//=====================================================================
	// upwards_bredth_first_for_each:
	//=====================================================================
	template <typename FN>
	void upwards_bredth_first_for_each(sooty::const_parseme_ptr_ref starting_node, FN func)
	{
		detail::upwards_bredth_first(starting_node, detail::for_each(func), detail::all_levels());
	}
	
	
	//=====================================================================
	// upwards_bredth_first_find_first_if
	//=====================================================================
	template <typename PR>
	sooty::parseme_ptr upwards_bredth_first_find_first_if(sooty::const_parseme_ptr_ref starting_node, PR pred) {
		parseme_ptr P;
		detail::upwards_bredth_first(starting_node, detail::find_first_if(P, pred), detail::all_levels());
		return P;
	}
	
	template <typename PR, typename D>
	sooty::parseme_ptr upwards_bredth_first_find_first_if(sooty::const_parseme_ptr_ref starting_node, PR pred, D depther) {
		parseme_ptr P;
		detail::upwards_bredth_first(starting_node, detail::find_first_if(P, pred), depther);
		return P;
	}
	
	//=====================================================================
	// upwards_find_first_of
	//=====================================================================
	template <typename PR>
	sooty::parseme_ptr linear_upwards_find_first_if(sooty::const_parseme_ptr_ref starting_node, PR pred)
	{
		sooty::parseme_ptr P;
		detail::linear_upwards( starting_node, detail::find_first_if(P, pred) );
		return P;
	}
	
	template <typename IT, typename PR>
	void linear_upwards_copy_if(sooty::const_parseme_ptr_ref starting_node, IT dest, PR pred)
	{
		detail::linear_upwards(starting_node, detail::copy_if(dest, pred));
	}
	
	//=====================================================================
	// depth_first_for_each:
	//=====================================================================
	struct do_nothing {
		detail::state::Enum operator ()(sooty::const_parseme_ptr_ref) { return detail::state::keep_going; }
	};
	
	template <typename Pre, typename Post>
	void depth_first_for_each(sooty::parseme_ptr_ref starting_node, Pre prefix_function, Post postfix_function)
	{
		detail::depth_first( starting_node, detail::for_each(prefix_function), detail::for_each(postfix_function) );
	}
	
	//=====================================================================
	// depth_first_find_first_if:
	//=====================================================================
	template <typename PR>
	sooty::parseme_ptr depth_first_find_first_if(sooty::parseme_ptr_ref starting_node, PR pred) {
		sooty::parseme_ptr P;
		detail::depth_first( starting_node, detail::find_first_if(P, pred), do_nothing() );
		return P;
	}
	
	
	//=====================================================================
	// depth_first_copy_if
	//=====================================================================
	template <typename IT, typename PR>
	void depth_first_copy_if(IT out, sooty::const_parseme_ptr_ref starting_node, PR pred) {
		detail::depth_first( starting_node, detail::copy_if(out, pred), do_nothing() );
	}
	
	
	/*
	template <typename F>
	void for_each(sooty::parseme_container& container, F func)
	{
		sooty::parseme_container master = container;
		for (size_t i = 0, i_end = container.size(); i != i_end; )
		{
			func(container[i]);
				
			// special sort of incrementing
			if (master != container)
			{
				sooty::parseme_container& lhs = (master.size() < container.size()) ? master : container;
				sooty::parseme_container& rhs = (master.size() < container.size()) ? container : master;
			
				std::pair< sooty::parseme_container::iterator, sooty::parseme_container::iterator > result = 
					std::mismatch(lhs.begin(), lhs.end(), rhs.begin());
			
				i = std::distance(rhs.begin(), result.second);
				master = container;
				i_end = master.size();
			}
			else {
				++i;
			}
		}
	}
*/

	inline sooty::parseme_container::iterator position_of(sooty::const_parseme_ptr_ref N) {
		sooty::parseme_ptr parent(N->parent.lock());
		return std::find(parent->children.begin(), parent->children.end(), N);
	}

	
	// makes sure parent points to correct location etc
	inline void replace_parseme_with_another(sooty::const_parseme_ptr_ref current, sooty::const_parseme_ptr_ref new_)
	{
		sooty::parseme_ptr parent = current->parent.lock();
		ATMA_ASSERT(parent);
		sooty::parseme_container::iterator i = position_of(current);
		ATMA_ASSERT(i != parent->children.end());
		*i = new_;
		new_->parent = parent;
	}
	
	
	struct set_parent_to {
		sooty::parseme_ptr parent;
		set_parent_to(sooty::const_parseme_ptr_ref parent) : parent(parent) {}
		void operator ()(sooty::parseme_ptr_ref N) const {
			N->parent = parent;
		}
	};
	
	//=====================================================================
	// the depth of a tree
	//=====================================================================
	inline int depth(sooty::const_parseme_ptr_ref root, sooty::const_parseme_ptr_ref child)
	{
		parseme_ptr n = child;
		int result = 0;
		while (n && n != root) {
			++result;
			n = n->parent.lock();
		}
		
		ATMA_ASSERT(n == root);
		return result;
	}
	
//=====================================================================
} // namespace sooty
//=====================================================================
#endif
//=====================================================================

