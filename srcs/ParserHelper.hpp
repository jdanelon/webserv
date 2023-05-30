#ifndef PARSERHELPER_HPP
# define PARSERHELPER_HPP

# include <iostream>
# include <cstring>
# include <vector>
# include <set>
# include <algorithm>

# include <netinet/in.h>
# include <arpa/inet.h>

# include "utils.hpp"
# include "ParserException.hpp"

class ParserHelper
{

	public:

		ParserHelper( void );
		ParserHelper( ParserHelper const &obj );
		ParserHelper &operator = ( ParserHelper const &obj );
		virtual ~ParserHelper( void );

		bool							duplicated_directives( std::vector<std::string> const &tokens );
		std::pair<in_addr_t, size_t>	get_listen( void );
		std::vector<std::string>		get_server_name( void );
		std::string						get_root( void );
		std::vector<std::string>		get_index( void );
		std::string						get_error_page( void );
		size_t							get_timeout( void );
		size_t							get_client_max_body_size( void );
		std::string						get_access_log( void );
		std::string						get_error_log( void );
		bool							get_autoindex( void );
		std::string						get_cgi( void );
		std::pair<size_t, std::string>	get_return( void );
		bool							get_upload( void );
		std::string						get_upload_store( void );
		std::vector<std::string>		get_limit_except( void );

	private:

		std::vector<std::string>	_tokens;
		std::set<std::string>		_list;

		bool	_valid_host( std::string const &ip );
		bool	_valid_port( std::string const &port );
		bool	_valid_server_name( std::string const &str );
	
	public:

		class InvalidLine: public ParserException
		{
			public:

				explicit InvalidLine( std::string const &str );
				char const	*what( void ) const throw();
		};

		class DuplicatedDirectives: public ParserException
		{
			public:

				explicit DuplicatedDirectives( std::string const &str );
				char const	*what( void ) const throw();
		};

		class InvalidNumberArgs : public ParserException
		{
			public:

				explicit InvalidNumberArgs( std::string const &str );
				char const	*what( void ) const throw();
		};

		class InvalidValues : public ParserException
		{
			public:

				explicit InvalidValues( std::string const &directive, std::string const &value );
				char const	*what( void ) const throw();
		};

		class UnknownDirective: public ParserException
		{
			public:

				explicit UnknownDirective( std::string const &str );
				char const *what( void ) const throw();
		};

};

#endif
