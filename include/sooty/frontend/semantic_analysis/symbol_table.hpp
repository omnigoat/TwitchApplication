//=====================================================================
//
//    symbol_table
//    ----------------
//      here's how I imagine this to work: in each scope, there's an
//    associated parseme_ptr, instead of a string. this conveys
//    everything those string representations provide, plus more. also
//    we don't have to lex ANOTHER string, over and over again.
//
//=====================================================================
#ifndef ATMA_SOOTY_SYMBOL_TABLE_HPP
#define ATMA_SOOTY_SYMBOL_TABLE_HPP
//=====================================================================
#include <list>
#include <map>
//=====================================================================
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
//=====================================================================
#include <sooty/sooty.hpp>
#include <sooty/common/value_t.hpp>
#include <sooty/frontend/syntactic_analysis/parseme.hpp>
//=====================================================================
namespace sooty {
//=====================================================================
class symbol_table;
//=====================================================================

	//=====================================================================
	// for debugging, we'll use std::multimap, for release, boost::unordered_multimap
	//=====================================================================
	#if defined(_DEBUG)
		typedef std::multimap<std::string, parseme_ptr> symbol_table_map;
	#else
		typedef boost::unordered_multimap<std::string, parseme_ptr> symbol_table_map;
	#endif
	
	
	//=====================================================================
	//
	//=====================================================================
	class symbol_table_state
	{
		friend class symbol_table;
		symbol_table_state() {}
		std::list<std::string> scope_list_;
	public:
		symbol_table_state(const symbol_table_state& rhs)
			: scope_list_(rhs.scope_list_)
		{
		}
	};
	
	
	//=====================================================================
	// the symbol table proper
	//=====================================================================
	class symbol_table
	{
	public:
		typedef std::string key_type;
		typedef parseme_ptr value_type;
		
		typedef std::vector<value_type> result_type;
		
	protected:
		struct node;
		
		boost::shared_ptr<node> root_;
		node* current_;
		
	public:
		symbol_table();
		
		// - enters a scope, and creates one if not present
		void enter_scope(const std::string& scope_name);
		// - leaves a scope, and deletes it if it contained no children
		// - returns true if it did, in fact, go up a scope
		bool leave_scope();
		
		// adds a binding to the current scope
		void add_binding(const key_type& key, const value_type& value);
		// looks up a binding in the current scope
		result_type lookup(const key_type& key) const;
		
		
		symbol_table_state save_state() const;
		void load_from_state(const symbol_table_state&);
	};
	
	
	//=====================================================================
	// a node in our symbol table
	//=====================================================================
	struct symbol_table::node
	{
		node()
			: parent()
		{
		}

		node(node* parent, const std::string& name)
			: parent(parent), name(name)
		{
		}

		node* parent;
		std::string name;
		symbol_table_map map;

		typedef std::list<node> children_container;
		children_container children;
	};
	
//	typedef symbol_table::ptr symbol_table_ptr;

//=====================================================================
} // namespace sooty
//=====================================================================
#endif
//=====================================================================
