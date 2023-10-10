#ifndef PARSER_HPP
# define PARSER_HPP

# include <iostream>
# include <vector>
# include <fcntl.h>
# include <fstream>
# include <sstream>
# include <cstdio>

# include "Server.hpp"
# include "Location.hpp"
# include "ParserHelper.hpp"
# include "../utils.hpp"

class Server;

class Parser
{

	public:

		int	backlog;

		Parser( void );
		Parser( Parser const &obj );
		Parser &operator = ( Parser const &obj );
		virtual ~Parser( void );

		void	load( char const *file );
		int		size( void );
		Server	&operator [] ( int i );

	private:

		std::string			_cleanFile;
		std::vector<Server>	_servers;

		void							_readFile( char const *file );
		void							_parse( std::istringstream *istr );
		Server							_parse_servers( std::istringstream *istr );
		static Location					_parse_location( std::istringstream *istr );
		static std::vector<std::string>	_get_split_lines( std::string line );

};

#endif
