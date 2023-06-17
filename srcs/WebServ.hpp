#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>

# include <poll.h>

# include "parser/Parser.hpp"
# include "parser/Server.hpp"

class WebServ
{

	public:

		Parser						parser;
		std::map<int, Server *>		servers;
		std::vector<int>			clients;
		std::vector<struct pollfd>	pollfds;

		WebServ( void );
		WebServ( WebServ const &obj );
		WebServ &operator = ( WebServ const &obj );
		virtual ~WebServ( void );

		void	init( char *argv );

	private:

		void	_init_servers( void );

};

#endif
