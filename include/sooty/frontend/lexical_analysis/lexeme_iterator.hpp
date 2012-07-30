//=====================================================================
//
//  lexeme_iterator
//  -----------------
//    Because regular iterators have no concept of channels, we need to
//  provide ones that do. This is that implementation.
//
//=====================================================================
#ifndef ATMA_LEXER_LEXEME_ITERATOR_HPP
#define ATMA_LEXER_LEXEME_ITERATOR_HPP
//=====================================================================
#include <vector>
//=====================================================================
#include <boost/utility/enable_if.hpp>
#include <boost/iterator/iterator_facade.hpp>
//=====================================================================
#include <sooty/frontend/lexical_analysis/lexeme.hpp>
//=====================================================================
namespace sooty {
//=====================================================================
class lexeme_list;
//=====================================================================

	
	//=====================================================================
	// SERIOUSLY, not the best place, but I can't think of a better one
	//=====================================================================
	typedef std::vector<lexeme> lexeme_container_type;
	
	
	//=====================================================================
	// iterator, thanks to boost
	//=====================================================================
	namespace detail
	{
		template <typename Value>
		class lexeme_iterator
		 : public boost::iterator_facade<lexeme_iterator<Value>, Value, boost::bidirectional_traversal_tag>
		{
			friend class boost::iterator_core_access;
			template <class> friend class lexeme_iterator;
			friend class lexeme_list;
			
		private:
			lexeme_container_type* container_;
			lexeme_container_type::iterator iter_;
			multichannel channel_;
			
		private:
			explicit lexeme_iterator(lexeme_container_type* container, lexeme_container_type::iterator iter, const multichannel& c)
			 : container_(container), iter_(iter), channel_(c)
			{
			}
			
			struct enabler {};
			
			template <class OtherValue>
			bool equal(lexeme_iterator<OtherValue> const& other,
				typename boost::enable_if
				<
					boost::is_convertible<OtherValue*,Value*>,
					enabler
				>::type = enabler()) const
			{
				if (!container_)
					return !other.container_;
				
				return this->iter_ == other.iter_;
			}
			
			
			// both increment and decrement will do their thing along the current channel.
			// if you want to specify behaviour, you'll have to use a method.
			void increment()
			{
				increment(channel_);
			}
			
			void decrement()
			{
				decrement(channel_);
			}
			
			
			
			// not implemented yet, maybe never
			//void increment(int n);
			
			Value& dereference() const
			{
				return *iter_;
			}

		public:
			lexeme_iterator()
			 : container_(), iter_(), channel_(sooty::any_channel)
			{
			}
			
			// copy constructor, boost-textbook-style
			template <class OtherValue>
			lexeme_iterator(lexeme_iterator<OtherValue> const& other,
				typename boost::enable_if
				<
				boost::is_convertible<OtherValue*,Value*>,
				enabler
				>::type = enabler())
				: container_(other.container_), iter_(other.iter_), channel_(other.channel_)
			{
			}
			
			bool increment(multichannel channel)
			{
				// continuation of current channel?
				if (channel.contains(current_channel)) channel = iter_->channel;
				// iterate until match is found
				while (++iter_ != container_->end())
				{
					if (channel.contains(any_channel)) break;
					if (iter_->channel.contains(channel)) break;
				}
				
				channel_ = channel;
				return iter_ != container_->end();
			}
			
			bool decrement(multichannel channel)
			{
				// continuation of current channel?
				if (channel.contains(current_channel)) channel = iter_->channel;
				// iterate until match is found
				while (--iter_ != container_->end())
				{
					if (channel.contains(any_channel)) break;
					if (iter_->channel.contains(channel)) break;
				}
				
				channel_ = channel;
				return iter_ != container_->end();
			}
			
			bool move(int n, multichannel c = current_channel)
			{
				if (n < 0)
					while (n++ < 0 && iter_ != container_->end())
						decrement(c);
				else if (n > 0)
					while (n-- > 0 && iter_ != container_->end())
						increment(c);
				
				return iter_ != container_->end();
			}
			
			int distance_to(const lexeme_iterator& rhs) const {
				return (rhs.iter_ - this->iter_);
			}
			
			void advance(size_t n) {
				this->iter_ += n;
			}
			
		};
	}
	
//=====================================================================
} // namespace sooty
//=====================================================================
#endif
//=====================================================================
