//=====================================================================
//
//
//=====================================================================
#ifndef SOOTY_ALGORITHM_ALGORITHMS_HPP
#define SOOTY_ALGORITHM_ALGORITHMS_HPP
//=====================================================================
#include <sooty/frontend/syntactic_analysis/parseme.hpp>
#include <sooty/frontend/syntactic_analysis/algorithm/detail/state.hpp>
//=====================================================================
namespace sooty {
namespace detail {
//=====================================================================
	
	
	//=====================================================================
	// depth-first algorithm
	//=====================================================================
	/*
	template <typename F, typename D>
	state::Enum depth_first(sooty::parseme_ptr_ref starting_node, F func, D depther)
	{
		sooty::parseme_ptr parent( starting_node->parent.lock() );
		if (parent && depther(parent)) {
			state::Enum r = func(starting_node);
			if (r == state::stop)
				return state::stop;
		}
		
		for (sooty::parseme_container::iterator i = starting_node->children.begin(); i != starting_node->children.end(); ++i)
		{
			state::Enum r = depth_first(*i, func, depther);
			if (r == state::stop)
				return state::stop;
		}
		
		return state::keep_going;
	}
	
	template <typename F, typename D>
	state::Enum depth_first(sooty::const_parseme_ptr_ref starting_node, F func, D depther)
	{
		sooty::parseme_ptr parent( starting_node->parent.lock() );
		if (parent && depther(parent)) {
			state::Enum r = func(starting_node);
			if (r == state::stop)
				return state::stop;
		}

		for (sooty::parseme_container::const_iterator i = starting_node->children.begin(); i != starting_node->children.end(); ++i)
		{
			state::Enum r = depth_first(*i, func, depther);
			if (r == state::stop)
				return state::stop;
		}

		return state::keep_going;
	}
	*/
	template <typename PreFunc, typename PostFunc>
	state::Enum depth_first(sooty::parseme_ptr_ref starting_node, PreFunc prefix_func, PostFunc postfix_func)
	{
		if (!starting_node)
			return state::keep_going;

		if ( prefix_func(starting_node) == state::stop )
			return state::stop;

		for (sooty::parseme_container::iterator i = starting_node->children.begin(); i != starting_node->children.end(); ++i)
		{
			state::Enum r = depth_first(*i, prefix_func, postfix_func);
			if (r == state::stop)
				return state::stop;
		}

		if ( postfix_func(starting_node) == state::stop )
			return state::stop;

		return state::keep_going;
	}
	
	template <typename PreFunc, typename PostFunc>
	state::Enum depth_first(sooty::const_parseme_ptr_ref starting_node, PreFunc prefix_func, PostFunc postfix_func)
	{
		if (!starting_node)
			return state::keep_going;
		
		if ( prefix_func(starting_node) == state::stop )
			return state::stop;
		
		for (sooty::parseme_container::const_iterator i = starting_node->children.begin(); i != starting_node->children.end(); ++i)
		{
			state::Enum r = depth_first(*i, prefix_func, postfix_func);
			if (r == state::stop)
				return state::stop;
		}

		if ( postfix_func(starting_node) == state::stop )
			return state::stop;
			
		return state::keep_going;
	}
	
	//=====================================================================
	// direct-upwards algorithm
	//=====================================================================
	template <typename F>
	void direct_upwards(sooty::parseme_ptr_ref starting_node, F func)
	{
		if (starting_node->parent.expired())
			return;
		
		parseme_ptr current_node(starting_node->parent);
		while (current_node)
		{
			state::Enum r = func(current_node);
			if (r == state::stop)
				break;

			current_node = current_node->parent.lock();
		}
	}
	
	template <typename F>
	void direct_upwards(sooty::const_parseme_ptr_ref starting_node, F func)
	{
		if (starting_node->parent.expired())
			return;

		parseme_ptr current_node(starting_node->parent);
		while (current_node)
		{
			state::Enum r = func(current_node);
			if (r == state::stop)
				break;

			current_node = current_node->parent.lock();
		}
	}
	
	
	
	//=====================================================================
	// linear-upwards algorithm
	//=====================================================================
	template <typename F>
	void linear_upwards(sooty::const_parseme_ptr_ref starting_node, F func)
	{
		parseme_ptr current_node = parseme_ptr(starting_node->parent);
		parseme_ptr child_node = starting_node;

		while (current_node)
		{
			parseme_container& children = current_node->children;
			parseme_container::reverse_iterator i = std::find(children.rbegin(), children.rend(), child_node);
			
			for (; i != children.rend(); ++i) {
				state::Enum r = func(*i);
				if (r == state::stop)
					return;
			}
			
			child_node = current_node;
			current_node = current_node->parent.lock();
		}
	}
	
	
	//=====================================================================
	// upwards-bredth-first algorithm
	//=====================================================================
	template <typename F, typename D>
	void upwards_bredth_first(sooty::const_parseme_ptr_ref starting_node, F func, D depther)
	{
		parseme_ptr current_node = parseme_ptr(starting_node->parent.lock());

		while (current_node)
		{
			if ( depther(current_node) )
			{
				parseme_container& children = current_node->children;
				
				for (parseme_container::const_iterator i = children.begin(); i != children.end(); ++i) {
					state::Enum r = func(*i);
					if (r == state::stop)
						return;
				}
			}
			
			current_node = current_node->parent.lock();
		}
	}
	
	template <typename F, typename D>
	void upwards_bredth_first(sooty::parseme_ptr_ref starting_node, F func, D depther)
	{
		parseme_ptr current_node = parseme_ptr(starting_node->parent);

		while (current_node)
		{
			if ( depther(current_node) )
			{
				parseme_container& children = current_node->children;

				for (parseme_container::const_iterator i = children.begin(); i != children.end(); ++i) {
					state::Enum r = func(*i);
					if (r == state::stop)
						return;
				}
			}

			current_node = current_node->parent.lock();
		}
	}


//=====================================================================
} // namespace detail
} // namespace sooty
//=====================================================================
#endif
//=====================================================================

