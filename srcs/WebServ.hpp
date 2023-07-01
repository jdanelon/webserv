#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>

# include <poll.h>
# include <csignal>

# include "parser/Parser.hpp"
# include "parser/Server.hpp"
# include "utils.hpp"

typedef struct	s_client
{
	Server		*host;
	long long	timestamp;
}				t_client;

class WebServ
{

	public:

		Parser						parser;
		std::map<int, Server *>		servers;
		std::map<int, t_client>		clients;
		std::vector<struct pollfd>	pollfds;

		WebServ( void );
		WebServ( WebServ const &obj );
		WebServ &operator = ( WebServ const &obj );
		virtual ~WebServ( void );

		void	init( char *argv );
		bool	client_timeout( int idx );
		void	end_client_connection( int idx );
		void	accept_queued_connections( int idx );

	private:

		void	_catch_signals( void );
		void	_init_servers( void );

};

#endif
