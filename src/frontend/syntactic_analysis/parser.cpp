#include <sooty/frontend/syntactic_analysis/parser.hpp>
//=====================================================================
#include <boost/bind.hpp>
//=====================================================================
#include <atma/unittest/unittest.hpp>
//=====================================================================
#include <atma/assert.hpp>
#include <atma/time.hpp>
//====================================================================
#include <sooty/frontend/syntactic_analysis/algorithm.hpp>

using namespace sooty;


//#define MATCH(thing) std::cout << "matched " << thing << std::endl
//#define INSERT(thing) std::cout << "inserted " << thing << std::endl
//#define MATCH_INSERT(thing1, thing2) std::cout << "match " << thing1 << " && insert " << thing2 << std::endl;
#define MATCH(thing)
#define INSERT(thing)
#define MATCH_INSERT(thing1, thing2)

//====================================================================
// parser
//====================================================================
sooty::parser::parser() : handle_(new detail::parser_handle_t)
{
}

sooty::parser::parser( const detail::abstract_parser_backend_ptr& backend ) : handle_(new detail::parser_handle_t(backend))
{
}

sooty::parser::parser( const parser& rhs )
	: handle_(new detail::parser_handle_t)
{
	*this = rhs;
}


parser& sooty::parser::operator = ( const parser& rhs )
{
	*handle_ = *rhs.handle_;
	return *this;
}

atma::time_t full_time = 0;
struct full_time_adder {
	atma::time_t begin;
	full_time_adder() : begin(atma::time()) {}
	~full_time_adder() { full_time += (atma::time() - begin); }
};

sooty::parse_results_t sooty::parser::operator()( const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr& dest ) const
{
	parse_results_t results = (*handle_->backend)(begin, end, dest);
	if (results.pass)
		if (handle_->on_success_)
			handle_->on_success_(results);
	return results;
}

sooty::parser sooty::parser::operator[]( parser& child )
{
	return sooty::parser( detail::abstract_parser_backend_ptr(new detail::delve_parser_backend(*this, child)) );
}

sooty::parser sooty::parser::operator >> ( parser& rhs )
{
	return sooty::parser( detail::abstract_parser_backend_ptr(new detail::seq_and_parser_backend(*this, rhs)) );
}

sooty::parser sooty::parser::operator | ( parser& rhs )
{
	return sooty::parser( detail::abstract_parser_backend_ptr(new detail::or_parser_backend(*this, rhs)) );
}

sooty::parser sooty::parser::operator || ( parser& rhs )
{
	return sooty::parser( detail::abstract_parser_backend_ptr(new detail::seq_or_parser_backend(*this, rhs)) );
}

sooty::parser sooty::parser::operator , ( parser& rhs )
{
	return operator >> (rhs);
}

sooty::parser sooty::parser::operator * ()
{
	return sooty::parser( detail::abstract_parser_backend_ptr(new detail::zero_or_more_parser_backend(*this)) );
}

sooty::parser sooty::parser::operator + ()
{
	return sooty::parser( detail::abstract_parser_backend_ptr(new detail::one_or_more_parser_backend(*this)) );
}

sooty::parser sooty::parser::operator!()
{
	return sooty::parser( detail::abstract_parser_backend_ptr(new detail::zero_or_one_parser_backend(*this)) );
}

sooty::parser sooty::parser::operator % ( parser& rhs )
{
	return sooty::parser( detail::abstract_parser_backend_ptr(new detail::seperated_zero_or_more(*this, rhs)) );
}

parser& sooty::parser::on_success( parsefunc_t func )
{
	handle_->on_success_ = func;
	return *this;
}
/*
sooty::parser sooty::parser::operator + ()
{
	return sooty::parser( detail::abstract_parser_backend_ptr(new detail::one_or_more_backend(*this)) );
}
*/

//====================================================================
// parser_ref
//====================================================================
sooty::detail::parser_ref::parser_ref()
{
}

sooty::detail::parser_ref::parser_ref( const parser& rhs )
{
	handle_ = rhs.handle_;
}

sooty::detail::parser_ref& sooty::detail::parser_ref::operator=( const parser& rhs )
{
	handle_ = rhs.handle_;
	return *this;
}


//====================================================================
// parser backends
//====================================================================
lexical_position last_file_postition;

sooty::parse_results_t sooty::detail::match_parser_backend::operator()( const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr& dest ) const
{
	full_time_adder FTA;
	lexeme_list::const_iterator start = begin;
	if (begin != end && start->id == match_id)
	{
		++start;
		parseme_ptr P = parseme::create(dest, match_id, begin->value, begin->position);
//		last_file_postition = begin->position;
		if (this->insert)
			dest->children.push_back(P);
		MATCH(match_id);
		return parse_results_t(true, true, start == end, begin, start, P);
	}
//	std::cout << "couldn't match " << match_id << " against " << begin->id << std::endl;
	return parse_results_t(false, false, begin == end, begin, begin, parseme::nullary);
}


sooty::parse_results_t sooty::detail::insert_parser_backend::operator()( const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr& dest ) const
{
	full_time_adder FTA;
	dest->children.push_back( sooty::make_parseme(dest, this->id, this->value) );
	INSERT(id);
	return parse_results_t(true, true, false, begin, begin, dest->children.back());
}


sooty::parse_results_t sooty::detail::insert_ref_parser_backend::operator()( const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr& dest ) const
{
	dest->children.push_back(p);
	p->parent = dest;
	return parse_results_t(true, true, false, begin, begin, dest->children.back());
}

sooty::parse_results_t sooty::detail::insert_some_parser_backend::operator()( const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr& dest ) const
{
	std::for_each(this->begin, this->end, sooty::set_parent_to(dest));
	dest->children.insert(dest->children.end(), this->begin, this->end);
	return parse_results_t(true, true, false, begin, begin, dest->children.back());
}

sooty::parse_results_t sooty::detail::match_insert_parser_backend::operator()( const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr& dest ) const
{
	full_time_adder FTA;
	lexeme_list::const_iterator start = begin;
	if (begin != end && start->id == match_id)
	{
		++start;
		parseme_ptr P;
		
		if (this->use_value)
			P = parseme::create(dest, insert_id, this->value, begin->position);
		else
			P = parseme::create(dest, insert_id, begin->value, begin->position);
		
//		last_file_postition = begin->position;
		dest->children.push_back(P);
		MATCH_INSERT(match_id, insert_id);
		return parse_results_t(true, true, start == end, begin, start, P);
	}
	return parse_results_t(false, false, false, begin, begin, parseme::nullary);
}

sooty::parse_results_t sooty::detail::delve_parser_backend::operator()( const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr& dest ) const
{
	parse_results_t head_results = head(begin, end, dest);
	if (!head_results.pass) return parse_results_t(false, false, false, begin, begin, parseme::nullary);
	parse_results_t child_results = child(head_results.stop, end, head_results.node);
	if (!child_results.pass) {
		if (!dest->children.empty())
			dest->children.pop_back();
		return parse_results_t(false, false, false, begin, begin, parseme::nullary);
	}
	
	return parse_results_t(true, true, child_results.stop == end, begin, child_results.stop, head_results.node);
}

sooty::parse_results_t sooty::detail::seq_and_parser_backend::operator()( const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr& dest ) const
{
	parse_results_t lhs_results = lhs(begin, end, dest);
	if (!lhs_results.pass) return parse_results_t(false, false, false, begin, begin, parseme::nullary);
	parse_results_t rhs_results = rhs(lhs_results.stop, end, dest);
	if (!rhs_results.pass) return parse_results_t(false, false, false, begin, begin, parseme::nullary);
	lhs_results.stop = rhs_results.stop;
	lhs_results.full = rhs_results.full;
	//lhs_results.node = rhs_results.node;
	return lhs_results;
}

sooty::parse_results_t sooty::detail::seq_or_parser_backend::operator()( const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr& dest ) const
{
	parse_results_t lhs_results = lhs(begin, end, dest);
	//if (!lhs_results.pass) return parse_results_t(false, false, false, begin, begin, parseme::nullary);
	parse_results_t rhs_results = rhs(lhs_results.stop, end, dest);
	if (!lhs_results.pass && !rhs_results.pass) 
		return parse_results_t(false, false, false, begin, begin, parseme::nullary);
	lhs_results.stop = rhs_results.stop;
	lhs_results.full = rhs_results.full;
	lhs_results.node = rhs_results.node;
	return lhs_results;
}

sooty::parse_results_t sooty::detail::or_parser_backend::operator()( const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr& dest ) const
{
	parse_results_t lhs_results = lhs(begin, end, dest);
	if (lhs_results.pass) return lhs_results;
	parse_results_t rhs_results = rhs(begin, end, dest);
	if (rhs_results.pass) return rhs_results;
	return parse_results_t(false, false, false, begin, begin, parseme::nullary);
}

sooty::parse_results_t sooty::detail::zero_or_more_parser_backend::operator()( const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr& dest ) const
{
	parse_results_t results(true, true, true, begin, begin, parseme::nullary);
	
	while ( results.stop != end )
	{
		parse_results_t temp_results = P(results.stop, end, dest);
		if (!temp_results.hit) break;
		results.hit = results.hit || temp_results.hit;
		results.stop = temp_results.stop;
	}
	results.full = results.stop == end;
	return results;
}

sooty::parse_results_t sooty::detail::one_or_more_parser_backend::operator()( const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr& dest ) const
{
	parse_results_t results = P(begin, end, dest);
	if (!results.pass) return results;
	
	while ( results.stop != end )
	{
		parse_results_t temp_results = P(results.stop, end, dest);
		if (!temp_results.hit) break;
		results.hit = results.hit || temp_results.hit;
		results.stop = temp_results.stop;
	}
	results.full = results.stop == end;
	return results;
}

sooty::parse_results_t sooty::detail::seperated_zero_or_more::operator()( const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr& dest ) const
{
	parse_results_t results(true, true, true, begin, begin, parseme::nullary);

	while ( results.stop != end )
	{
		if (results.stop != begin) {
			parse_results_t temp_results = rhs(results.stop, end, dest);
			if (!temp_results.pass) break;
			results.hit = results.hit || temp_results.hit;
			results.stop = temp_results.stop;
		}
		
		parse_results_t temp_results = lhs(results.stop, end, dest);
		if (!temp_results.pass) break;
		results.hit = results.hit || temp_results.hit;
		results.stop = temp_results.stop;
	}
	results.full = results.stop == end;
	return results;
}

sooty::parse_results_t sooty::detail::zero_or_one_parser_backend::operator()( const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr& dest) const
{
	parse_results_t result = P(begin, end, dest);
	result.pass = true;
	return result;
}

sooty::parse_results_t sooty::detail::user_parser_backend::operator()( const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr& dest ) const
{
	full_time_adder FTA;
	return P(begin, end, dest);
}

sooty::parse_results_t sooty::detail::guard_parser_backend::operator()( const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr& dest ) const
{
	parse_results_t R = P(begin, end, dest);
	R.pass = true;
	R.hit = true;
	return R;
}

sooty::parse_results_t sooty::detail::direct_parser_backend::operator()( const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr& dest ) const
{
	return P(begin, end, dest);
}

sooty::parse_results_t sooty::detail::stack_store_parser_backend::operator()( const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr& dest ) const
{
	stack.push( sooty::make_parseme(parseme_ptr(), 0, sooty::value_t()) );
	parse_results_t results = P(begin, end, stack.top());
	if (!results.pass) {
		stack.pop();
	}
	
	return results;
}

sooty::parse_results_t sooty::detail::stack_retrieve_parser_backend::operator()( const lexeme_list::const_iterator& begin, const lexeme_list::const_iterator& end, parseme_ptr& dest) const
{
	ATMA_ASSERT(!stack.empty());
	dest->children.insert( dest->children.end(), stack.top()->children.begin(), stack.top()->children.end() );
	std::for_each(stack.top()->children.begin(), stack.top()->children.end(), sooty::set_parent_to(dest));
	stack.pop();
	
	return parse_results_t(true, true, begin == end, begin, begin, dest); //dest->children.empty() ? dest : dest->children.back());
}



















sooty::parse_results_t sooty::immediate::operator()( const parser& P )
{
	return P( lexeme_list::const_iterator(), lexeme_list::const_iterator(), N );
}

sooty::parse_results_t sooty::assigner_t::operator()( const parser& P )
{
	sooty::parseme_ptr dummy = sooty::make_parseme( sooty::parseme_ptr(), 0, sooty::value_t() );
	parse_results_t result = P( lexeme_list::const_iterator(), lexeme_list::const_iterator(), dummy );
	if (result.pass) {
		container.insert(position, dummy->children.begin(), dummy->children.end());
	}
	std::for_each(dummy->children.begin(), dummy->children.end(), set_parent_to(parent));
	return result;
}

sooty::parse_results_t sooty::stack_assigner_t::operator ()( const parser& P )
{
	stack.push( sooty::make_parseme(parseme_ptr(), 0, sooty::value_t()) );
	parse_results_t result = P( lexeme_list::const_iterator(), lexeme_list::const_iterator(), stack.top() );
	if (!result.pass) {
		stack.pop();
	}
	return result;
}

sooty::parse_results_t sooty::rewriter_t::operator [](const sooty::parser& P)
{
	sooty::parseme_ptr dummy = sooty::make_parseme( sooty::parseme_ptr(), 0, sooty::value_t() );
	parse_results_t result = P( lexeme_list::const_iterator(), lexeme_list::const_iterator(), dummy );
	if (result.pass) {
		// can't have more than one top-node for a rewrite! :O
		ATMA_ASSERT(dummy->children.size() == 1);
		sooty::parseme_ptr parent = n->parent.lock();
		sooty::parseme_container::iterator position_of_us = 
			std::find(parent->children.begin(), parent->children.end(), n);
		ATMA_ASSERT(position_of_us != parent->children.end());
		//position_of_us = parent->children.erase(position_of_us);
		std::for_each(dummy->children.begin(), dummy->children.end(), sooty::set_parent_to(parent));
		//parent->children.insert(position_of_us, dummy->children.begin(), dummy->children.end());
		*position_of_us = dummy->children.back();
		result.node = dummy->children.back();
	}
	
	return result;
}

//====================================================================
// tests!
//====================================================================
SUITE(parser)
{
	struct fixture
	{
		lexeme first, second, third;
		lexeme_list lexemes;
		parseme_ptr root;
		
		fixture()
			: first(1, sooty::any_channel, 3, 4, 5, "cheese"),
			  second(2, sooty::any_channel, 13, 14, 15, "avocado"),
			  third(3, sooty::any_channel, 113, 114, 115, "chicken"),
			  root(parseme::create(parseme_ptr(), 0, value_t(), lexical_position(0, 0, 0)))
		{
			lexemes.push_back(first);
			lexemes.push_back(second);
			lexemes.push_back(third);
		}
	};
	
	TEST_FIXTURE(fixture, match)
	{
		parser P( detail::abstract_parser_backend_ptr(new detail::match_parser_backend(1, true)) );
		parse_results_t results = P(lexemes.begin(), lexemes.end(), root);
		CHECK(results.hit);
		CHECK(!results.full);
		//CHECK(results.node);
	}
	
	TEST_FIXTURE(fixture, delve)
	{
		parser P( detail::abstract_parser_backend_ptr(new detail::match_parser_backend(1, true)) );
		parser P2( detail::abstract_parser_backend_ptr(new detail::match_parser_backend(2, true)) );
		//parser P3( detail::abstract_parser_backend_ptr(new detail::match_parser_backend(3, true)) );
		// delve parser, having the result of P2 as a child node of P
		parser D = P[P2];
		parse_results_t results = D(lexemes.begin(), lexemes.end(), root);
		CHECK(results.hit);
		CHECK(!results.full);
		CHECK(root->children.size() == 1);
	}
	
	TEST_FIXTURE(fixture, delve_multiples)
	{
		parser P( detail::abstract_parser_backend_ptr(new detail::match_parser_backend(1, true)) );
		parser P2( detail::abstract_parser_backend_ptr(new detail::match_parser_backend(2, true)) );
		parser P3( detail::abstract_parser_backend_ptr(new detail::match_parser_backend(3, true)) );
		// delve parser, having the result of P2 as a child node of P
		parser D = P[P2 >> P3];
		parse_results_t results = D(lexemes.begin(), lexemes.end(), root);
		CHECK(results.hit);
		CHECK(results.full);
		CHECK(root->children.size() == 1);
		if (root->children.size() == 1)
		{
			CHECK(root->children.front()->children.size() == 2);
		}
	}
	
	TEST(blah)
	{
		parser first, second;
		
		sooty::detail::parser_handle_ptr first_handle = first.handle_;
		sooty::detail::parser_handle_ptr second_handle = second.handle_;
		
		first = guard( match(0) );
		
		CHECK_EQUAL(first_handle, first.handle_);
		first = second;
		CHECK_EQUAL(first_handle, first.handle_);
		
		
		first = !second;
		
		second = !first;
		
		detail::parser_ref PR = first;
		
		
	}
}

