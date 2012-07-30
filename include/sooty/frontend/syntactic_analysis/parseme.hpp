//=====================================================================
//
//
//
//=====================================================================
#ifndef ATMA_SOOTY_PARSER_PARSEME_HPP
#define ATMA_SOOTY_PARSER_PARSEME_HPP
//=====================================================================
#include <string>
#include <vector>
#include <map>
#include <list>
//=====================================================================
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
//=====================================================================
#include <sooty/common/value_t.hpp>
#include <sooty/frontend/lexical_analysis/lexeme.hpp>
//=====================================================================
namespace sooty {
//=====================================================================
	
	struct parseme;
	typedef boost::shared_ptr<parseme> parseme_ptr;
	typedef parseme_ptr& parseme_ptr_ref;
	typedef const parseme_ptr& const_parseme_ptr_ref;
	typedef boost::weak_ptr<parseme> parseme_wptr;
	typedef std::vector<parseme_ptr> parseme_container;
	
	
	
	//=====================================================================
	// a semant!
	//=====================================================================
	struct semant_t;
	typedef semant_t& semant_ref;
	typedef const semant_t& const_semant_ref;
	typedef std::map<std::string, semant_t> semants_t;
	
	struct semant_t
	{
		semant_t() : integer(), real() {}
		semant_t( const value_t& value ) : integer(value.integer), real(value.real), string(value.string) {}
		semant_t( const_parseme_ptr_ref p ) : integer(), real(), parseme(p) {}
		int integer;
		float real;
		std::string string;
		parseme_ptr parseme;
		semants_t children;
	};
	
	
	
	//=====================================================================
	// a parseme!
	//=====================================================================
	
	
	struct parseme
	{
		static parseme_ptr nullary;
		static parseme_ptr create(const parseme_ptr& parent, size_t id, const lexical_position&);
		static parseme_ptr create(const parseme_ptr& parent, size_t id, const value_t& value, const lexical_position&);
		
		size_t id;
		value_t value;
		lexical_position position;
		parseme_wptr parent;
		parseme_container children;
		semants_t semantics;
		
		//bool operator < (const parseme& rhs) const {
		//	return id < rhs.id;
		//}
		
	private:
		parseme(const boost::shared_ptr<parseme>& parent, size_t id, const value_t& value, const sooty::lexical_position&);
	};
	
	parseme_ptr make_parseme(const parseme_ptr& parent, size_t id, const value_t& value = value_t());
	
	
	
	//=====================================================================
	// predicates
	//=====================================================================
	struct id_less_than_pred {
		bool operator ()(sooty::const_parseme_ptr_ref lhs, sooty::const_parseme_ptr_ref rhs) const {  return lhs->id < rhs->id;  }
		bool operator ()(sooty::const_parseme_ptr_ref lhs, size_t id) const {  return lhs->id < id;  }
		bool operator ()(size_t id, sooty::const_parseme_ptr_ref rhs) const {  return id < rhs->id;  }
	};
	
	struct id_matches {
		size_t id;
		id_matches(size_t id) : id(id) {}
		bool operator ()(const_parseme_ptr_ref rhs) const { return id == rhs->id; }
	};
	
	struct value_string_matches {
		std::string string;
		value_string_matches(const std::string& string) : string(string) {}
		bool operator ()(const_parseme_ptr_ref rhs) const { return string == rhs->value.string; }
	};
	


	//=====================================================================
	// cloning a tree
	//=====================================================================
	parseme_ptr clone_tree(const_parseme_ptr_ref);
	
	namespace detail {
		void clone_tree_impl(parseme_ptr_ref new_parent, const_parseme_ptr_ref old_parent);
	}
	
	
//=====================================================================
} // namespace sooty
//=====================================================================
#endif
//=====================================================================

