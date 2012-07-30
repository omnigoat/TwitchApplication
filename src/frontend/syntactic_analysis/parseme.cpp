#include <sooty/frontend/syntactic_analysis/parseme.hpp>
#include <atma/assert.hpp>
#include <atma/unittest/unittest.hpp>

using namespace sooty;
boost::shared_ptr<parseme> parseme::nullary;

parseme_ptr parseme::create(const boost::shared_ptr<parseme>& parent, size_t id, const sooty::lexical_position& fp)
{
	return parseme_ptr( new parseme(parent, id, value_t(), fp) );
}

parseme_ptr parseme::create(const boost::shared_ptr<parseme>& parent, size_t id, const value_t& value, const sooty::lexical_position& fp)
{
	return parseme_ptr( new parseme(parent, id, value, fp) );
}

parseme::parseme( const boost::shared_ptr<parseme>& parent, size_t id, const value_t& value, const sooty::lexical_position& fp )
	: parent(parent), id(id), value(value), position(fp)
{
}


sooty::parseme_ptr sooty::clone_tree( const_parseme_ptr_ref N )
{
	parseme_ptr newN(new parseme(*N));
	newN->children.clear();
	newN->parent = parseme_ptr();
	detail::clone_tree_impl(newN, N);
	return newN;
}

sooty::parseme_ptr sooty::make_parseme( const parseme_ptr& parent, size_t id, const value_t& value )
{
	return parseme::create(parent, id, value, lexical_position());
}

/*

*/

void sooty::detail::clone_tree_impl( parseme_ptr_ref new_parent, const_parseme_ptr_ref old_parent )
{
	for (sooty::parseme_container::const_iterator i = old_parent->children.begin(); i != old_parent->children.end(); ++i)
	{
		new_parent->children.push_back( parseme_ptr(new parseme(**i)) );
		new_parent->children.back()->parent = new_parent;
		new_parent->children.back()->children.clear();
		clone_tree_impl( new_parent->children.back(), *i );
	}
}



TEST(parseme_test)
{
	parseme_ptr p = parseme::create(parseme::nullary, size_t(), "everything", lexical_position(3, 4, 8) );
	
	CHECK(size_t() == p->id);
	CHECK("everything" == p->value.string);
	CHECK(0 == p->value.integer);
//	CHECK(0 == p.value.real);
	CHECK(0 == p->children.size());
	CHECK(3 == p->position.row);
	CHECK(4 == p->position.column);
}

