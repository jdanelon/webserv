#ifndef HPP_CLIENT_CONNECTION
# define HPP_CLIENT_CONNECTION

# include <iostream>

# include "../http/HttpRequest.hpp"
# include "../parser/Server.hpp"

class ClientConnection
{

	public:
		Server		*host;
		long long	timestamp;

		std::string buffer;
		HttpRequest request;
		HttpRequest response;
		bool		is_request_parsed;
		bool		is_request_completed;
		bool		is_header_received;
		bool		is_line_request_received;

		ClientConnection( void );
		ClientConnection( Server *server, long long timestamp );
		ClientConnection( ClientConnection const &obj );
		ClientConnection &operator = ( ClientConnection const &obj );
		virtual ~ClientConnection( void );

	private:

};

#endif