//=====================================================================
//
// 
//
//=====================================================================
#ifndef ATMA_SOOTY_LEXEMIFIER_HPP
#define ATMA_SOOTY_LEXEMIFIER_HPP
//=====================================================================
#include <string>
//=====================================================================
#include <boost/lexical_cast.hpp>
//=====================================================================
#include <sooty/sooty.hpp>
#include <sooty/frontend/lexical_analysis/lexeme.hpp>
#include <sooty/frontend/lexical_analysis/lexeme_list.hpp>
#include <sooty/frontend/lexical_analysis/lexer.hpp>
//=====================================================================
namespace sooty {
//=====================================================================

	namespace detail
	{
		template <typename T> inline sooty::value_t make_value(const lex_results_t& R);

		template <> inline sooty::value_t make_value<std::string>(const lex_results_t& R) {
			return value_t( make_string(R.start, R.stop) );
		}

		template <> inline sooty::value_t make_value<int>(const lex_results_t& R) {
			return value_t( boost::lexical_cast<int>(make_string(R.start, R.stop)) );
		}
		
		template <> inline sooty::value_t make_value<float>(const lex_results_t& R) {
			return value_t( boost::lexical_cast<float>(make_string(R.start, R.stop)) );
		}
	}
	
	
	struct lexemifier_t
	{
		size_t row, column, pos;
		lexeme_list list;

		lexemifier_t() : row(1), column(1), pos() {}

		struct newliner_t {
			lexemifier_t* L;
			newliner_t(lexemifier_t* L) : L(L) {}
			void operator ()(lex_results_t& R) {
				++L->row;
				L->column = 1;
			}
		};

		newliner_t newliner() { return newliner_t(this); }

		template <typename T>
		struct maker_t {
			lexemifier_t* L;
			size_t id;
			multichannel chnl;
			bool valuify;
			maker_t(lexemifier_t* L, size_t id, multichannel chnl, bool valuify) : L(L), id(id), chnl(chnl), valuify(valuify) {}
			void operator ()(const lex_results_t& R) {
				lexeme new_lexeme(id, chnl, L->row, L->column, L->pos++, detail::make_value<T>(R));
				L->list.push_back(new_lexeme);
				L->column += R.length;
			}
		};

		template <typename T> maker_t<T> make(size_t id, multichannel chnl, bool include_value = true) {
			return maker_t<T>(this, id, chnl, true);
		}

		maker_t<std::string> make(size_t id, multichannel chnl, bool include_value = true) {
			return maker_t<std::string>(this, id, chnl, include_value);
		}

	};

//=====================================================================
} // namespace sooty
//=====================================================================
#endif
//=====================================================================

