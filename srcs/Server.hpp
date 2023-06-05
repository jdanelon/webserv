#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <cstring>
# include <vector>
# include <map>
# include <climits>

# include "Location.hpp"

class Location;

class Server
{

	public:

		in_addr_t						host;
		int								port;
		std::vector<std::string>		server_name;
		std::string						root;
		std::vector<std::string>		index;
		std::map<int, std::string>		error_page;
		size_t							timeout;
		size_t							client_max_body_size;
		std::string						access_log;
		std::string						error_log;
		int								autoindex;
		std::pair<size_t, std::string>	redirect;
		int								upload;
		std::string						upload_store;
		std::map<std::string, Location>	location;

		Server( void );
		Server( Server const &obj );
		Server &operator = ( Server const &obj );
		virtual ~Server( void );

		void	fill_with_defaults( void );

	private:
	// Attributes

	// Methods

};

#endif
