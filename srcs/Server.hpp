#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <cstring>
# include <vector>

class Server
{

	public:

		in_addr_t						ip;
		size_t							port;
		std::vector<std::string>		server_name;
		std::string						root;
		std::vector<std::string>		index;
		std::string						error_page;
		size_t							timeout;
		size_t							client_max_body_size;
		std::string						access_log;
		std::string						error_log;
		int								autoindex;
		std::pair<size_t, std::string>	redirect;
		int								upload;
		std::string						upload_store;

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
