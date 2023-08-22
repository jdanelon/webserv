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

		std::string	buffer;
		HttpRequest	request;
		std::string	response;
		bool		is_line_request_received;
		bool		is_header_received;
		bool		is_request_parsed;
		bool		is_request_completed;

		ClientConnection( void );
		ClientConnection( Server *server, long long timestamp );
		ClientConnection( ClientConnection const &obj );
		ClientConnection &operator = ( ClientConnection const &obj );
		virtual ~ClientConnection( void );

	private:

		// TO-DO Create a map with status code and corresponding message
		//
		// I am not sure where it should be configured and if public or private
		// The way I did here gives a memory leak, so I commented everything
		// std::map<int, std::string>	_error_codes_map;

};

#endif
