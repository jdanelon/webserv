#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>

# include <poll.h>
# include <csignal>

# include "parser/Parser.hpp"
# include "parser/Server.hpp"
# include "connection/ClientConnection.hpp"
# include "utils.hpp"

extern int g_signal_code;

#define RESPONSE_CHUNK_SIZE 2048

class WebServ
{

	public:
		static bool debugEnabled;
		static const std::string className; 

		Parser								parser;
		std::map<int, Server *>				servers;
		std::vector<struct pollfd>			pollfds;
		std::map<int, ClientConnection>		client_connections;
		bool								has_closed_connections;

		WebServ( void );
		WebServ( WebServ const &obj );
		WebServ &operator = ( WebServ const &obj );
		virtual ~WebServ( void );

		void	init( char const *argv );
		bool	client_timeout( int idx );
		void	end_client_connection( int idx );
		void	accept_queued_connections( int idx );
		void	parse_request_headers( int idx );
		void	parse_request_body( int idx );
		void	create_response( int idx );
		void	send_response( int idx );
		void	purge_connections( void );
		void	handle_100_continue( int idx );
		void	send_100_continue( int idx );

		// Debug
		void	print( void );
		static void debug(LogLevel level, const std::string& message);
	private:

		unsigned int	_backlog;

		void		_catch_signals( void );
		static void	_signal_handler( int const code );
		void		_init_servers( void );
		void		_clear_connection( int const client_fd );
		void		_clear_100_continue( int const client_fd );
};

#endif
