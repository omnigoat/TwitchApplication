//=====================================================================
//
//  Lexical Analysis
//  ------------------
//    Kaleidescope has m_tokens starting with letters or underscores,
//    case sensitivity, operators, and "other" (brackets, +, - , etc)
//
//=====================================================================
#ifndef ATMA_KAL_TREEWALKER_TO_C_H
#define ATMA_KAL_TREEWALKER_TO_C_H
//=====================================================================
#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <vector>
#include <deque>
#include <list>
#include <map>
#include <sstream>
#include <stack>
//=====================================================================
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
//=====================================================================
//#include <atma/TreeNode.h>
//=====================================================================
#include <atma/kaleidoscope/lexer/token_ids.hpp>
#include <atma/kaleidoscope/parser/parser.hpp>
#include <atma/kaleidoscope/treewalker/treewalker_interface.hpp>
//=====================================================================
ATMA_KAL_BEGIN
//=====================================================================
	
	class interpreter_c
	 : public treewalker_interface
	{
		std::string m_module_name;
		std::vector<std::string> m_typename;
		std::stack<std::string> m_scope;
		std::ofstream m_file;
		
	public:
		interpreter_c(const parser::parseme_ptr& root_node)
		 : treewalker_interface(root_node), m_file("output.txt", std::ofstream::trunc | std::ofstream::out)
		{
			m_scope.push("__"); // global scope is two underscores
		}
		
		~interpreter_c()
		{
			//system("\"C:/Program Files/Microsoft Visual Studio 8/VC/vcvarsall.bat\"");
			//system("cl output.cpp");
			//std::cin.get();
		}
		
		void add_scope(const std::string& scope)
		{
			m_scope.push(scope);
			m_typename.push_back(scope);
		}
		
		std::string create_typename(const std::string& final_type)
		{
			std::string s = "_";
			for (std::vector<std::string>::const_iterator i = m_typename.begin(); i != m_typename.end(); ++i)
			{
				s += *i + "_";
			}
			return s + final_type;
		}
		
		
		void write_file(const std::string& s)
		{
			m_file.write(s.c_str(), std::streamsize(s.size()));
		}
		
		
		void walk()
		{
			rule_module();
		}
		
		bool rule_module()
		{
			// well we must be a module declaration
			if (!delve_if(KI_MODULE)) return false;
			{
			
				// do the identifier rule
				if (current_id() != IDENTIFIER) return false;
				// get its value
				add_scope(current_value());
				
				// and move on
				advance();
				
				// delve into body if it's there
				if (!delve_if(KI_BODY)) return false;
				{
					// one function
					rule_function();
				} surface();
				
				
			} surface(); // KI_MODULE
			
			// all good
			return true;
		}
		
		bool rule_function()
		{
			// delve into function node
			if (!delve_if(KI_FUNCTION)) return false;
			
			// return-type
			rule_return_type();
			
			advance();
			
			// space!
			write_file(" ");
			
			// function identifier
			rule_identifier();
			
			advance();
			
			// parameter list
			rule_parameter_list();
			
			
			write_file("\n{");
			
			
			
			write_file("\n}");
			
			surface(); // KI_FUNCTION
			
			// all good
			return true;
		}
		
		bool rule_return_type()
		{
			if (!delve_if(KI_RETURN_TYPE)) return false;
			rule_typename();
			surface();
			return true;
		}
		
		bool rule_typename()
		{
			switch (current_id())
			{
				case K_INT: write_file("int"); break;
				case K_REAL: write_file("real"); break;
				case K_BOOL: write_file("bool"); break;
				case K_CHAR: write_file("char"); break;
				case IDENTIFIER: write_file(current_value()); break;
				default: return false;
			}
			return true;
		}
		
		
		bool rule_parameter_list()
		{
			if (!delve_if(KI_PARAMETER_LIST)) return false;
			write_file("(");
			
			// while the child is a parameter
			while (current_id() == KI_PARAMETER)
			{
				// perform the parameter rule
				if (!rule_parameter()) { surface(); return false; }
				// if the next child isn't a parameter, bork
				if (LA(1) != KI_PARAMETER) break;
				// else, advance the child, and throw in a comma
				advance();
				write_file(", ");
			}
			
			write_file(")");
			surface();
			return true;
		}
		
		bool rule_parameter()
		{
			// checking is done via parameter-list
			delve_if(KI_PARAMETER);
			// do the typename
			if (!rule_typename()) { surface(); return false; }
			write_file(" ");
			
			advance();
			
			// do the identifier!
			if (!rule_identifier()) { surface(); return false; }
			
			if (LA(1) == KI_DEFAULT_VALUE)
			{
				advance();
				write_file(" = ");
				rule_default_value();
			}
			
			surface();
			return true;
		}
		
		bool rule_identifier()
		{
			if (current_id() != IDENTIFIER) return false;
			write_file( create_typename(current_value()) ); //()->get_value() );
			return true;
		}
		
		bool rule_default_value()
		{
			if (!delve_if(KI_DEFAULT_VALUE)) return false;
			if (!rule_literal()) return false;
			surface();
			return true;
		}
		
		bool rule_literal()
		{
			if (current_id() == INT_LITERAL)
			{
				write_file(current_value());
			}
			else if (current_id() == BOOL_LITERAL)
			{
				write_file(current_value());
			}
			else if (current_id() == CHAR_LITERAL)
			{
				write_file("'");
				write_file(current_value());
				write_file("'");
			}
			else if (current_id() == STRING_LITERAL)
			{
				write_file("\"");
				write_file(current_value());
				write_file("\"");
			}
			else
			{
				return false;
			}
			return true;
		}
		
	};

//=====================================================================
ATMA_KAL_CLOSE
//=====================================================================
#endif
//=====================================================================
