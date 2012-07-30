#ifndef SOOTY_SANDBOX_HPP
#define SOOTY_SANDBOX_HPP
//=====================================================================
#include <map>
#include <string>
#include <stack>
#include <vector>
//=====================================================================
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <boost/optional.hpp>
//=====================================================================
#include <atma/functor.hpp>
//=====================================================================
#include <sooty/frontend/lexical_analysis/lexeme_list.hpp>
#include <sooty/frontend/syntactic_analysis/algorithm.hpp>
//=====================================================================
#include <atma/assert.hpp>
#include <sooty/frontend/syntactic_analysis/parser.hpp>
#include <boost/enable_shared_from_this.hpp>
namespace sooty {
namespace cuil {
//=====================================================================
	
	struct abstract_rule_backend;
	typedef boost::shared_ptr<abstract_rule_backend> abstract_rule_backend_ptr;
	typedef sooty::parseme_container::iterator parseme_iterator;
	typedef sooty::parseme_container& parseme_container_ref;
	typedef std::back_insert_iterator<parseme_container> parseme_insert_iterator;
	
	//=====================================================================
	//
	//    various structures during parsing
	//    -------------------------------------
	//
	//=====================================================================
	struct global_state_t
	{
		std::map<int, std::vector<sooty::parseme_ptr> > globals;
	};
	
	struct local_state_t
	{
		local_state_t* parent_state;
		std::map<int, std::vector<sooty::parseme_ptr> > locals;
		std::map<int, local_state_t*> child_states;
		
		sooty::parseme_ptr us() { return locals[0].front(); }
	};
	
	namespace detail {
		static parseme_container_ref unused_container() {
			static parseme_container _;
			return _;
		}
	}
	
		
	struct matching_context_t
	{
		sooty::parseme_container& container;
		parseme_iterator begin, end;
		
		matching_context_t(parseme_container_ref container, const parseme_iterator& begin, const parseme_iterator& end)
			: container(container), begin(begin), end(end)
		{
		}
		
		bool is_exhausted() const { return container.empty() || begin == end; }
		
		void reset(parseme_container& container) {
			this->container = container;
			begin = container.begin();
			end = container.end();
		}

		static matching_context_t nullary() { return matching_context_t(detail::unused_container(), parseme_iterator(), parseme_iterator()); }
		
		matching_context_t copy_match(int n_matched) const {
			return matching_context_t(container, begin, begin + n_matched);
		}
	};
	typedef matching_context_t& matching_context_ref;
	typedef const matching_context_t& const_matching_context_ref;
	
	
	// this represents what happens when we match. we get the global state, the local
	// state, a copy of the context of our match
	struct rule_results_t
	{
		// flag.
		bool pass;
		// the global scope (there is only one!)
		global_state_t& gs;
		// the local scope we will add *children of this result* to
		local_state_t& ls;
		// what we matched - a [begin, end) pair and the container
		matching_context_t context;
		
		
		rule_results_t(bool pass, global_state_t& gs, local_state_t& ls, const_matching_context_ref context)
			: pass(pass), gs(gs), ls(ls), context(context)
		{
		}
		
		static rule_results_t failure(global_state_t& gs, local_state_t& ls, const_matching_context_ref context) {
			return rule_results_t(false, gs, ls, context);
		}
	};
	
	struct rewrite_result_t
	{
		global_state_t& gs;
		local_state_t& ls;
		
		// the node that was created
		parseme_ptr node;
		// where the iterator passed into apply now is
		parseme_insert_iterator end;
		
		rewrite_result_t(global_state_t& gs, local_state_t& ls, parseme_insert_iterator end) : gs(gs), ls(ls), end(end) {}
		rewrite_result_t(global_state_t& gs, local_state_t& ls, parseme_insert_iterator end, parseme_ptr node) : gs(gs), ls(ls), end(end), node(node) {}
	};
	

	
	//=====================================================================
	// a backend
	//=====================================================================
	struct abstract_rule_backend
	{
		virtual rule_results_t apply(global_state_t&, local_state_t&, parseme_container_ref, parseme_iterator, parseme_iterator) = 0;
		virtual ~abstract_rule_backend() {}
	};
	
	
	struct abstract_match_rule_backend //: abstract_rule_backend
	{
		virtual rule_results_t apply(global_state_t&, local_state_t&, matching_context_ref) = 0;
		virtual local_state_t& local_state() = 0;
		virtual ~abstract_match_rule_backend() {}
	};
	typedef boost::shared_ptr<abstract_match_rule_backend> abstract_match_rule_backend_ptr;
	
	
	struct abstract_rewrite_rule_backend //: abstract_rule_backend
	{
		virtual rewrite_result_t apply(global_state_t&, local_state_t&, parseme_insert_iterator) = 0;
		virtual ~abstract_rewrite_rule_backend() {}
	};
	typedef boost::shared_ptr<abstract_rewrite_rule_backend> abstract_rewrite_rule_backend_ptr;
	
	
	//=====================================================================
	//
	//    PATTERN MATCHING
	//
	//=====================================================================
	struct rewrite_rule;

	inline void reset_parents(parseme_ptr parent)
	{
//		std::for_each(parent->children.begin(), parent->children.end(), set_parent_to(parent));
//		std::for_each(parent->children.begin(), parent->children.end(), reset_parents);
	}
	
	struct match_rule
	{
		abstract_match_rule_backend_ptr backend;
		
		match_rule() {}
		match_rule(abstract_match_rule_backend_ptr backend) : backend(backend) {}
		
		inline rule_results_t apply(global_state_t& gs, local_state_t& ls, matching_context_ref);
		inline local_state_t& local_state();
		
		inline match_rule operator [] (match_rule rhs);
		//match_rule operator [] (rewrite_rule rhs);
		inline match_rule operator ,  (match_rule rhs);
		inline match_rule operator *  ();
		inline match_rule operator |  (match_rule rhs);
		inline match_rule operator ~  ();
		
		inline match_rule rewrite( rewrite_rule rhs );
		
		//typedef boost::function<bool (global_state_t&, local_state_t&, parseme_iterator, parseme_iterator)> func_t;
		typedef boost::function< bool (const_matching_context_ref) > func_t;
		inline match_rule perform( func_t );
		
		bool operator ()(matching_context_ref context)
		{
			global_state_t gs;
			local_state_t ls;

			//parseme_container temp;
			//temp.push_back(node);
			
			//sooty::parseme_ptr parent = node->parent.lock();
//			ATMA_ASSERT(parent);
			//sooty::parseme_container::iterator i = position_of(node);
			rule_results_t R = backend->apply(gs, ls, context);
			if (R.pass) {
				// reset parents! important!
				//sooty::parseme_ptr parent = (*R.context.begin)->parent.lock();
				//reset_parents(parent ? parent : *R.context.begin);
				//std::cout << "awesome!" << std::endl;
				return true;
			}

			return false;
		}
		
	};
	
	struct any_abstract_rule_backend : abstract_match_rule_backend
	{
		local_state_t local_state_;
		
		rule_results_t apply(global_state_t& gs, local_state_t& ls, matching_context_ref context) {
			if (context.is_exhausted())
				return rule_results_t::failure(gs, ls, context);
			
			ls.locals[0].resize(1, *context.begin);
			return rule_results_t(true, gs, ls, context.copy_match(1));
		}
		
		local_state_t& local_state() { return local_state_; }
	};
	
	inline match_rule any() {
		return match_rule( abstract_match_rule_backend_ptr(new any_abstract_rule_backend) );
	}


	struct eq_rule : abstract_match_rule_backend
	{
		size_t id;
		boost::optional<value_t> value;
		local_state_t local_state_;
		
		eq_rule(size_t id) : id(id) {}
		eq_rule(size_t id, value_t value) : id(id), value(value) {}
		
		rule_results_t apply(global_state_t& gs, local_state_t& ls, matching_context_ref context)
		{
			if (context.is_exhausted() || (*context.begin)->id != id) {
				return rule_results_t::failure(gs, ls, context);
			}
			else {
				if (value.is_initialized()) {
					const value_t& v = value.get();
					if (v != (*context.begin)->value)
						return rule_results_t::failure(gs, ls, context.copy_match(1));
				}
			}
			
			local_state_.locals[0].resize(1, *context.begin);
			return rule_results_t(true, gs, this->local_state_, context.copy_match(1));
		}
		
		local_state_t& local_state() { return local_state_; }
	};
	
	inline match_rule eq(size_t id) {
		return match_rule( abstract_match_rule_backend_ptr(new eq_rule(id)) );
	}
	
	inline match_rule eq(size_t id, value_t value) {
		return match_rule( abstract_match_rule_backend_ptr(new eq_rule(id, value)) );
	}
	
	struct not_rule : abstract_match_rule_backend
	{
		abstract_match_rule_backend_ptr lhs;
		
		not_rule(abstract_match_rule_backend_ptr lhs) : lhs(lhs) {}
		
		rule_results_t apply(global_state_t& gs, local_state_t& ls, matching_context_ref context)
		{
			rule_results_t R = lhs->apply(gs, ls, context);
			R.pass = !R.pass;
			return R;
		}
		
		local_state_t& local_state() { return lhs->local_state(); }
	};
	
	
	struct delve_rule : abstract_match_rule_backend
	{
		abstract_match_rule_backend_ptr lhs, rhs;

		delve_rule(abstract_match_rule_backend_ptr lhs, abstract_match_rule_backend_ptr rhs)
			: lhs(lhs), rhs(rhs)
		{
		}
		
		rule_results_t apply(global_state_t& gs, local_state_t& ls, matching_context_ref context)
		{
			rule_results_t head_result = lhs->apply(gs, ls, context);
			if (!head_result.pass)
				return rule_results_t::failure(gs, ls, context);
			
			sooty::parseme_ptr_ref head = *head_result.context.begin;
			
			// special note: since this is a pattern-matching algorithm, we need to test
			// every child node, since the pattern could be matched via any child
			matching_context_t child_context(head->children, head->children.begin(), head->children.end());
			//while (!child_context.is_exhausted())
			{
				rule_results_t child_result = rhs->apply(gs, head_result.ls, child_context);
				if (child_result.pass)
					return head_result;
			//	++child_context.begin;
			}
			
			return rule_results_t::failure(gs, ls, context);
		}

		local_state_t& local_state() {
			return lhs->local_state();
		}
	};
	
	
	struct or_rule : abstract_match_rule_backend
	{
		abstract_match_rule_backend_ptr lhs, rhs;

		or_rule(abstract_match_rule_backend_ptr lhs, abstract_match_rule_backend_ptr rhs)
			: lhs(lhs), rhs(rhs)
		{
		}

		rule_results_t apply(global_state_t& gs, local_state_t& ls, matching_context_ref context) {
			if (context.is_exhausted())
				return rule_results_t::failure(gs, ls, context);

			rule_results_t R = lhs->apply(gs, ls, context);
			if (!R.pass)
				return rhs->apply(gs, ls, context);

			return R;
		}

		local_state_t& local_state() { return rhs->local_state(); }
	};
	
	struct seq_and_rule : abstract_match_rule_backend
	{
		abstract_match_rule_backend_ptr lhs, rhs;

		seq_and_rule(abstract_match_rule_backend_ptr lhs, abstract_match_rule_backend_ptr rhs)
			: lhs(lhs), rhs(rhs)
		{
		}

		rule_results_t apply(global_state_t& gs, local_state_t& ls, matching_context_ref context) {
			if (context.is_exhausted())
				return rule_results_t::failure(gs, ls, context);
			
			rule_results_t R = lhs->apply(gs, ls, context);
			//ATMA_ASSERT(&R.context.container == &context.container);
			if (!R.pass)
				return rule_results_t::failure(gs, ls, context);
			
			return rhs->apply(gs, ls, matching_context_t(R.context.container, R.context.end, R.context.container.end()));
		}
		
		local_state_t& local_state() { return rhs->local_state(); }
	};
	
	
	struct zero_or_more_rule : abstract_match_rule_backend
	{
		abstract_match_rule_backend_ptr lhs;

		zero_or_more_rule(abstract_match_rule_backend_ptr lhs) : lhs(lhs) {}

		rule_results_t apply(global_state_t& gs, local_state_t& ls, matching_context_ref context)
		{
			parseme_iterator cbegin = context.begin;
			while ( cbegin != context.end )
			{
				rule_results_t R = lhs->apply(gs, ls, matching_context_t(context.container, cbegin, context.end));
				cbegin = R.context.end;
				if (!R.pass)
					break;
			}
			
			return rule_results_t(true, gs, ls, matching_context_t(context.container, context.begin, cbegin));
		}
		
		local_state_t& local_state() { return lhs->local_state(); }
	};
	
	
	struct assign_local_variable : abstract_match_rule_backend
	{
		int id;
		abstract_match_rule_backend_ptr rhs;

		assign_local_variable(int id, abstract_match_rule_backend_ptr rhs) : id(id), rhs(rhs) {}

		rule_results_t apply(global_state_t& gs, local_state_t& ls, matching_context_ref context)
		{
			rule_results_t results = rhs->apply(gs, ls, context);
			if (results.pass) {
				ls.locals[id].assign(results.context.begin, results.context.end);
				ls.child_states[id] = &rhs->local_state();
				rhs->local_state().parent_state = &ls;
			}
			return results;
		}
		
		local_state_t& local_state() { rhs->local_state(); }
	};
	
	
	struct append_local_variable : abstract_match_rule_backend
	{
		int id;
		abstract_match_rule_backend_ptr rhs;

		append_local_variable(int id, abstract_match_rule_backend_ptr rhs) : id(id), rhs(rhs) {}

		rule_results_t apply(global_state_t& gs, local_state_t& ls, matching_context_ref context)
		{
			rule_results_t results = rhs->apply(gs, ls, context);
			if (results.pass) {
				ls.locals[id].insert(ls.locals[id].end(), results.context.begin, results.context.end);
				ls.child_states[id] = &rhs->local_state();
				rhs->local_state().parent_state = &ls;
			}
			return results;
		}

		local_state_t& local_state() { rhs->local_state(); }
	};
	
	
	struct assign_global_variable : abstract_match_rule_backend
	{
		int id;
		abstract_match_rule_backend_ptr rhs;

		assign_global_variable(int id, abstract_match_rule_backend_ptr rhs) : id(id), rhs(rhs) {}

		rule_results_t apply(global_state_t& gs, local_state_t& ls, matching_context_ref context)
		{
			rule_results_t results = rhs->apply(gs, ls, context);
			if (results.pass) {
				gs.globals[id].assign(results.context.begin, results.context.end);
			}
			return results;
		}
		
		local_state_t& local_state() { return rhs->local_state(); }
	};
	
	
	struct append_global_variable : abstract_match_rule_backend
	{
		int id;
		abstract_match_rule_backend_ptr rhs;

		append_global_variable(int id, abstract_match_rule_backend_ptr rhs) : id(id), rhs(rhs) {}

		rule_results_t apply(global_state_t& gs, local_state_t& ls, matching_context_ref context)
		{
			rule_results_t results = rhs->apply(gs, ls, context);
			if (results.pass) {
				gs.globals[id].insert(gs.globals[id].end(), results.context.begin, results.context.end);
			}
			return results;
		}
		
		local_state_t& local_state() { return rhs->local_state(); }
	};
	
	
	struct perform_rule_backend : abstract_match_rule_backend
	{
		abstract_match_rule_backend_ptr lhs;
		match_rule::func_t rhs;
		
		perform_rule_backend(abstract_match_rule_backend_ptr lhs, match_rule::func_t rhs) : lhs(lhs), rhs(rhs) {}
		
		rule_results_t apply(global_state_t& gs, local_state_t& ls, matching_context_ref context)
		{
			rule_results_t lhs_results = lhs->apply(gs, ls, context);
			if (!lhs_results.pass)
				return lhs_results;
			if (!rhs(lhs_results.context))
				return rule_results_t::failure(gs, ls, context);
			return lhs_results;
		}
		
		local_state_t& local_state() { return lhs->local_state(); }
	};
	
	inline rule_results_t match_rule::apply(global_state_t& gs, local_state_t& ls, matching_context_ref context) {
		return backend->apply(gs, ls, context);
	}
	
	inline match_rule match_rule::operator [] (match_rule rhs) {
		return match_rule( abstract_match_rule_backend_ptr(new delve_rule(backend, rhs.backend)) );
	}
	/*
	match_rule match_rule::operator [] (rewrite_rule rhs) {
		return match_rule( abstract_match_rule_backend_ptr(new hide_rewrite_rule(rhs.backend)) );
	}
	*/
	inline match_rule match_rule::operator * () {
		return match_rule( abstract_match_rule_backend_ptr(new zero_or_more_rule(backend)) );
	}

	inline match_rule match_rule::operator , (match_rule rhs) {
		return match_rule( abstract_match_rule_backend_ptr(new seq_and_rule(backend, rhs.backend)) );
	}
	
	inline match_rule match_rule::operator | (match_rule rhs) {
		return match_rule( abstract_match_rule_backend_ptr(new or_rule(backend, rhs.backend)) );
	}
	
	inline match_rule match_rule::operator ~ () {
		return match_rule( abstract_match_rule_backend_ptr(new not_rule(backend)) );
	}
	
	local_state_t& match_rule::local_state() {
		return backend->local_state();
	}

	
	
	
	
	
	
	
	
	//=====================================================================
	//
	//    REWRITING
	//
	//=====================================================================
	struct rewrite_rule
	{
		abstract_rewrite_rule_backend_ptr backend;

		rewrite_rule() {}
		rewrite_rule(abstract_rewrite_rule_backend_ptr backend) : backend(backend) {}

		rewrite_result_t apply(global_state_t& gs, local_state_t& ls, parseme_insert_iterator end);

		inline rewrite_rule operator [] (rewrite_rule rhs);
		inline rewrite_rule operator ,  (rewrite_rule rhs);
		inline rewrite_rule operator *  ();
		
//		operator match_rule() {
//			return match_rule( abstract_match_rule_backend_ptr(new hide_rewrite_rule(backend)) );
//		}
	};
	
	namespace detail {
	namespace rewrite {
	
	struct make_node_rule : abstract_rewrite_rule_backend
	{
		size_t id;
		value_t value;
		make_node_rule(size_t id, value_t value) : id(id), value(value) {}

		rewrite_result_t apply(global_state_t& gs, local_state_t& ls, parseme_insert_iterator end)
		{
			parseme_ptr node = sooty::make_parseme(parseme_ptr(), id, value);
			*end++ = node;
			return rewrite_result_t(gs, ls, end, node); 
		}
	};
	
	
	struct insert_local_variable_rule : abstract_rewrite_rule_backend
	{
		std::vector<int> ids;
		insert_local_variable_rule(const std::vector<int>& ids) : ids(ids) {}

		rewrite_result_t apply(global_state_t& gs, local_state_t& ls, parseme_insert_iterator end)
		{
			local_state_t* cls = &ls;
			for (std::vector<int>::const_iterator i = ids.begin(); i != ids.end(); ++i) {
				cls = cls->child_states[*i];
			}
			
			std::for_each(cls->locals[0].begin(), cls->locals[0].end(), sooty::set_parent_to(ls.us()));
			return rewrite_result_t(gs, ls, std::copy(cls->locals[0].begin(), cls->locals[0].end(), end), cls->locals[0].back());
		}
	};
	
	
	struct insert_global_variable_rule : abstract_rewrite_rule_backend
	{
		int id;
		insert_global_variable_rule(int id) : id(id) {}

		rewrite_result_t apply(global_state_t& gs, local_state_t& ls, parseme_insert_iterator end)
		{
			return rewrite_result_t(gs, ls, std::copy(gs.globals[id].begin(), gs.globals[id].end(), end), ls.locals[0].back());
		}
	};
	
	
	struct delve_rule : abstract_rewrite_rule_backend
	{
		abstract_rewrite_rule_backend_ptr lhs, rhs;

		delve_rule(abstract_rewrite_rule_backend_ptr lhs, abstract_rewrite_rule_backend_ptr rhs)
			: lhs(lhs), rhs(rhs)
		{
		}
		
		rewrite_result_t apply(global_state_t& gs, local_state_t& ls, parseme_insert_iterator end)
		{
			rewrite_result_t head_result = lhs->apply(gs, ls, end);
			ATMA_ASSERT(head_result.node);
			head_result.node->children.clear();
			rewrite_result_t child_result = rhs->apply(gs, head_result.ls, std::back_inserter(head_result.node->children));
			return rewrite_result_t(gs, ls, head_result.end, head_result.node);
		}
	};
		
	
	struct seq_and_rule : abstract_rewrite_rule_backend
	{
		abstract_rewrite_rule_backend_ptr lhs, rhs;

		seq_and_rule(abstract_rewrite_rule_backend_ptr lhs, abstract_rewrite_rule_backend_ptr rhs)
			: lhs(lhs), rhs(rhs)
		{
		}

		rewrite_result_t apply(global_state_t& gs, local_state_t& ls, parseme_insert_iterator end)
		{
			rewrite_result_t lhs_results = lhs->apply(gs, ls, end);
			return rhs->apply(gs, ls, lhs_results.end);
		}
	};
	
	/*
	struct zero_or_more_rule : abstract_rewrite_rule_backend
	{
		abstract_rewrite_rule_backend_ptr lhs;

		zero_or_more_rule(abstract_rewrite_rule_backend_ptr lhs) : lhs(lhs) {}

		rewrite_result_t apply(global_state_t& gs, local_state_t& ls, parseme_insert_iterator end)
		{
			//sooty::parseme_ptr parent = node->parent.lock();
			//sooty::parseme_container::iterator cpi = std::find(parent->children.begin(), parent->children.end(), node);
			while ( cpi != parent->children.end() && lhs->apply(gs, ls, *cpi).pass )
				++cpi;

			return rule_results_t(gs, ls, node, true);
		}
	};
	*/
	
	
	
	struct rewrite_rule : abstract_match_rule_backend {
		abstract_match_rule_backend_ptr lhs;
		abstract_rewrite_rule_backend_ptr rhs;

		rewrite_rule(abstract_match_rule_backend_ptr lhs, abstract_rewrite_rule_backend_ptr rhs)
			: lhs(lhs), rhs(rhs)
		{
		}
		
		rule_results_t apply(global_state_t& gs, local_state_t& ls, matching_context_ref context)
		{
			rule_results_t match_results = lhs->apply(gs, ls, context);
			if (!match_results.pass)
				return match_results;
			
			std::cout << "rewriting..." << std::endl;
			
			// write out the rewrite rule to a dummy parseme_ptr
			sooty::parseme_ptr dummy = sooty::make_parseme(parseme_ptr(), -1, value_t());
			rewrite_result_t rewrite_results = rhs->apply(gs, match_results.ls, std::back_inserter(dummy->children));
			
			// now insert the elements into the real node
			parseme_iterator i = match_results.context.container.erase(match_results.context.begin, match_results.context.end);
			parseme_container new_children(match_results.context.container.begin(), i);
			new_children.insert(new_children.end(), dummy->children.begin(), dummy->children.end());
			new_children.insert(new_children.end(), i, match_results.context.container.end());
			context.container.assign(new_children.begin(), new_children.end());
			context.begin = context.container.begin();
			context.end = context.container.end();
			return rule_results_t(true, gs, ls, context);
		}
		
		local_state_t& local_state() { return lhs->local_state(); }
	};
	
	
	} // namespace rewrite
	} // namespace detail
	
	inline match_rule match_rule::rewrite( rewrite_rule rhs ) {
		return match_rule( abstract_match_rule_backend_ptr(new detail::rewrite::rewrite_rule(backend, rhs.backend)) );
	}
	
	inline match_rule match_rule::perform( func_t func )
	{
		return match_rule( abstract_match_rule_backend_ptr(new perform_rule_backend(backend, func)) );
	}
	
	inline rewrite_rule mk(size_t id, value_t value = value_t()) {
		return rewrite_rule( abstract_rewrite_rule_backend_ptr(new detail::rewrite::make_node_rule(id, value)) );
	}
	
	
	

	inline rewrite_rule rewrite_rule::operator [] (rewrite_rule rhs) {
		return rewrite_rule( abstract_rewrite_rule_backend_ptr(new detail::rewrite::delve_rule(backend, rhs.backend)) );
	}
/*
	rewrite_rule rewrite_rule::operator * () {
		return rewrite_rule( abstract_rewrite_rule_backend_ptr(new detail::rewrite::zero_or_more_rule(backend)) );
	}
*/
	inline rewrite_rule rewrite_rule::operator , (rewrite_rule rhs) {
		return rewrite_rule( abstract_rewrite_rule_backend_ptr(new detail::rewrite::seq_and_rule(backend, rhs.backend)) );
	}
	
	
	
	
	
	
	
	
	
	
	
	
	
	struct local_variable;
	struct member_local_variable
	{
		std::vector<int> ids;
		member_local_variable& operator () (const local_variable& rhs);
		rewrite_rule operator [] (member_local_variable& rhs);
		rewrite_rule operator [] (local_variable& rhs);
		rewrite_rule operator ,  (rewrite_rule rhs);
		
		operator rewrite_rule() {
			return rewrite_rule( abstract_rewrite_rule_backend_ptr(new detail::rewrite::insert_local_variable_rule(ids)) );
		}
		
	};
	
	struct local_variable {
		int id;
		local_variable(int id) : id(id) {}
		
		match_rule operator = (match_rule rhs) {
			return match_rule( abstract_match_rule_backend_ptr(new assign_local_variable(id, rhs.backend)) );
		}
		
		match_rule operator += (match_rule rhs) {
			return match_rule( abstract_match_rule_backend_ptr(new append_local_variable(id, rhs.backend)) );
		}
		
		member_local_variable operator () (local_variable rhs) {
			return member_local_variable()(*this)(rhs);
		}
		
		
		operator rewrite_rule() {
			std::vector<int> ids;
			ids.push_back(id);
			
			return rewrite_rule( abstract_rewrite_rule_backend_ptr(new detail::rewrite::insert_local_variable_rule(ids)) );
		}
	};
	
	
	
	
	
	
	
	inline member_local_variable& member_local_variable::operator () (const local_variable& rhs) {
		ids.push_back(rhs.id);
		return *this;
	}
	
	inline rewrite_rule member_local_variable::operator [](sooty::cuil::member_local_variable& rhs) {
		return rewrite_rule( abstract_rewrite_rule_backend_ptr(new detail::rewrite::delve_rule(rewrite_rule(*this).backend, rewrite_rule(rhs).backend)) );
	}
	
	inline rewrite_rule member_local_variable::operator [](local_variable& rhs) {
		return rewrite_rule( abstract_rewrite_rule_backend_ptr(new detail::rewrite::delve_rule(rewrite_rule(*this).backend, rewrite_rule(rhs).backend)) );
	}
	
	inline rewrite_rule member_local_variable::operator ,(rewrite_rule rhs) {
		return rewrite_rule( abstract_rewrite_rule_backend_ptr(new detail::rewrite::seq_and_rule(rewrite_rule(*this).backend, rhs.backend)) );
	}
	
	
	
	struct global_variable {
		int id;
		global_variable(int id) : id(id) {}
		
		match_rule operator = (match_rule rhs) {
			return match_rule( abstract_match_rule_backend_ptr(new assign_global_variable(id, rhs.backend)) );
		}
		
		match_rule operator += (match_rule rhs) {
			return match_rule( abstract_match_rule_backend_ptr(new append_global_variable(id, rhs.backend)) );
		}
		
		rewrite_rule operator ,  (rewrite_rule rhs) {
			return rewrite_rule( abstract_rewrite_rule_backend_ptr(new detail::rewrite::seq_and_rule(rewrite_rule(*this).backend, rhs.backend)) );
		}
		
		operator rewrite_rule() {
			return rewrite_rule( abstract_rewrite_rule_backend_ptr(new detail::rewrite::insert_global_variable_rule(id)) );
		}
	};
	
	
	struct assign_string_value
	{
		std::string& out;
		assign_string_value(std::string& out) : out(out)
		{
		}
		
		bool operator ()( const_matching_context_ref context ) {
			out = (*context.begin)->value.string;
			return true;
		}
	};
	
	
	
	
	
	namespace placeholders {
		namespace {
			local_variable _parent(-1);
			local_variable _this(0);
			local_variable _a(1);
			local_variable _b(2);
			local_variable _c(3);
			
			global_variable _1(1);
			global_variable _2(2);
			global_variable _3(3);
			global_variable _4(4);
			global_variable _5(5);
			global_variable _6(6);
			global_variable _7(7);
			global_variable _8(8);
			global_variable _9(9);
		}
	}
	
	struct no_pattern {
		sooty::detail::state::Enum operator ()(sooty::cuil::matching_context_ref) { return sooty::detail::state::keep_going; }
	};
	
	
	namespace detail {
		template <typename PR1, typename PR2>
		void tree_pattern_impl(sooty::parseme_container& container, sooty::parseme_container::iterator& current, sooty::parseme_container::iterator& end, PR1 prefix, PR2 postfix)
		{
			// prefix
			if (prefix(cuil::matching_context_t(container, current, end))) {
				current = container.begin();
				end = container.end();
				return;
			}
			
			// children
			for (parseme_container::iterator i = (*current)->children.begin(), i_end = (*current)->children.end(); i != i_end; )
			{
				tree_pattern_impl((*current)->children, i, i_end, prefix, postfix);
			}
			
			// postfix
			if (postfix(cuil::matching_context_t(container, current, end))) {
				current = container.begin();
				end = container.end();
				return;
			}
			
			// increment properly
			++current;
		}
	}
	
	template <typename PR1, typename PR2>
	void tree_pattern(sooty::parseme_ptr_ref starting_node, PR1 prefix, PR2 postfix)
	{
		sooty::parseme_ptr parent = starting_node->parent.lock();
		sooty::parseme_container& children = parent ? parent->children : starting_node->children;
		
		for (parseme_container::iterator i = children.begin(), i_end = children.end(); i != i_end; )
		{
			detail::tree_pattern_impl(children, i, i_end, prefix, postfix);
		}
	}
	
	template <typename PR1, typename PR2>
	void tree_pattern(sooty::parseme_container& starting_container, PR1 prefix, PR2 postfix) {
		for (parseme_container::iterator i = starting_container.begin(), i_end = starting_container.end(); i != i_end; ) {
			detail::tree_pattern_impl(starting_container, i, i_end, prefix, postfix);
		}
	}
	
//=====================================================================
} // namespace cuil
} // namespace sooty
//=====================================================================
#endif // inclusion guard
//=====================================================================




