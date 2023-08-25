#include "ClientConnection.hpp"

ClientConnection::ClientConnection( void ) : host(NULL),
												timestamp(0),
												buffer(""),
												request(HttpRequest()),
												response(""),
												is_line_request_received(false),
												is_header_received(false),												
												is_request_parsed(false),
												is_request_completed(false) {}

ClientConnection::ClientConnection( Server *server, long long timestamp ) : host(server),
																			timestamp(timestamp),
																			buffer(""),
																			request(HttpRequest()),
																			response(""),
																			is_line_request_received(false),
																			is_header_received(false),
																			is_request_parsed(false),
																			is_request_completed(false) {
	// https://developer.mozilla.org/en-US/docs/Web/HTTP/Status#information_responses
	// this->_error_codes_map[100] = "Continue";
	// this->_error_codes_map[200] = "OK";
	// this->_error_codes_map[201] = "Created";
	// this->_error_codes_map[202] = "Accepted";
	// this->_error_codes_map[204] = "No Content";
	// this->_error_codes_map[301] = "Moved Permanently";
	// this->_error_codes_map[302] = "Found";
	// this->_error_codes_map[400] = "Bad Request";
	// this->_error_codes_map[401] = "Unauthorized";
	// this->_error_codes_map[403] = "Forbidden";
	// this->_error_codes_map[404] = "Not Found";
	// this->_error_codes_map[405] = "Method Not Allowed";
	// this->_error_codes_map[408] = "Request Timeout";
	// this->_error_codes_map[410] = "Gone";
	// this->_error_codes_map[411] = "Length Required";
	// this->_error_codes_map[413] = "Payload Too Large";
	// this->_error_codes_map[414] = "URI Too Long";
	// this->_error_codes_map[415] = "Unsupported Media Type";
	// this->_error_codes_map[429] = "Too Many Requests";
	// this->_error_codes_map[500] = "Internal Server Errror"; // EXAMPLE: TIMEOUT ON CGI
	// this->_error_codes_map[501] = "Not Implemented";
	// this->_error_codes_map[505] = "HTTP Version Not Supported";
}

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
