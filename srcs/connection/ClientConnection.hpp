#ifndef HPP_CLIENT_CONNECTION
# define HPP_CLIENT_CONNECTION

# include <iostream>

# include "../http/HttpRequest.hpp"
# include "../http/HttpResponse.hpp"
# include "../parser/Server.hpp"

class ClientConnection
{

	public:
		Server		*host;
		long long	timestamp;

		std::string		buffer;
		HttpRequest		request;
		HttpResponse	response;
		bool			is_line_request_received;
		bool			is_header_received;
		bool			is_request_parsed;
		bool			is_request_completed;

		ClientConnection( void );
		ClientConnection( Server *server, long long timestamp );
		ClientConnection( ClientConnection const &obj );
		ClientConnection &operator = ( ClientConnection const &obj );
		virtual ~ClientConnection( void );
};

#endif
