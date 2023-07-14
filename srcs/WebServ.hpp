#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>

# include <poll.h>
# include <csignal>

# include "parser/Parser.hpp"
# include "parser/Server.hpp"
# include "utils.hpp"

extern int g_signal_code;

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
		bool						has_closed_connections;

		WebServ( void );
		WebServ( WebServ const &obj );
		WebServ &operator = ( WebServ const &obj );
		virtual ~WebServ( void );

		void	init( char *argv );
		bool	client_timeout( int idx );
		void	end_client_connection( int idx );
		void	accept_queued_connections( int idx );
		void	purge_connections( void );

	private:

		int			_backlog;

		void		_catch_signals( void );
		static void	_signal_handler( int const code );
		void		_init_servers( void );

};

#endif
