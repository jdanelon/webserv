#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>

# include <poll.h>
# include <csignal>
# include <unordered_map>

# include "parser/Parser.hpp"
# include "parser/Server.hpp"
# include "connection/ClientConnection.hpp"
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
		std::map<int, t_client>		clients; // can be merged with clients_connections
		std::vector<struct pollfd>	pollfds;
		std::unordered_map<int, ClientConnection>		client_connections; // can be merged with clients
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

		unsigned int	_backlog;

		void		_catch_signals( void );
		static void	_signal_handler( int const code );
		void		_init_servers( void );

};

#endif
