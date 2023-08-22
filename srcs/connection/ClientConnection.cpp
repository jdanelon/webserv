#include "ClientConnection.hpp"

ClientConnection::ClientConnection( void ) : host(NULL),
												timestamp(0),
												response(""),
												is_line_request_received(false),
												is_header_received(false),												
												is_request_parsed(false),
												is_request_completed(false) {}

ClientConnection::ClientConnection( Server *server, long long timestamp ) : host(server),
																			timestamp(timestamp),
																			response(""),
																			is_line_request_received(false),
																			is_header_received(false),
																			is_request_parsed(false),
																			is_request_completed(false) {}

ClientConnection::ClientConnection( ClientConnection const &obj )
{
	ClientConnection::operator = (obj);
	return ;
}

ClientConnection::~ClientConnection( void )
{
	return ;
}

ClientConnection &ClientConnection::operator = ( ClientConnection const &obj )
{
	if (this != &obj)
	{
		this->host = obj.host;
		this->timestamp = obj.timestamp;
		this->buffer = obj.buffer;
		this->request = obj.request;
		this->response = obj.response;
		this->is_line_request_received = obj.is_line_request_received;
		this->is_header_received = obj.is_header_received;
		this->is_request_parsed = obj.is_request_parsed;
		this->is_request_completed = obj.is_request_completed;
	}
	return (*this);
}
