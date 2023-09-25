#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <cstring>
# include <vector>
# include <map>
# include <climits>
# include <unistd.h>
# include <fcntl.h>

# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>

# include "Location.hpp"
# include "ParserException.hpp"
# include "../utils.hpp"

class Location;

class Server
{

	public:

		std::string							ip;
		std::string							port;
		std::vector<std::string>			server_name;
		std::string							root;
		std::vector<std::string>			index;
		std::map<int, std::string>			error_page;
		size_t								timeout;
		int									client_max_body_size;
		// std::string							access_log;
		// std::string							error_log;
		int									autoindex;
		std::map<std::string, std::string>	cgi;
		std::pair<size_t, std::string>		redirect;
		int									upload;
		std::string							upload_store;
		std::map<std::string, Location>		location;

		int									sockfd;

		Server( void );
		Server( Server const &obj );
		Server &operator = ( Server const &obj );
		virtual ~Server( void );

		void	fill_with_defaults( void );
		void	connect_socket( int backlog );

	private:
	// Attributes

	// Methods

	public:

		class SocketError : public ParserException
		{
			public:

				explicit SocketError( std::string const &str );
				char const *what( void ) const throw();
		};

};

#endif
