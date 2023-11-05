#include "ClientConnection.hpp"

ClientConnection::ClientConnection( void ) : host_server(NULL),
												timestamp(0),
												buffer(""),
												body_buffer(""),
												request(HttpRequest()),
												response(HttpResponse(NULL)),
												is_line_request_received(false),
												is_header_received(false),												
												is_request_parsed(false),
												is_request_completed(false),
												is_request_body_parsing(false),
												is_request_body_parsed(false),
												request_has_body(false),
												tail_appended_body(false),
												continue_reading_body(false),
												is_100_continue(false),
												is_100_continue_sent(false)
												{}

ClientConnection::ClientConnection( Server *server, long long timestamp ) : host_server(server),
																			timestamp(timestamp),
																			buffer(""),
																			body_buffer(""),
																			request(HttpRequest()),
																			response(HttpResponse(NULL)),
																			is_line_request_received(false),
																			is_header_received(false),
																			is_request_parsed(false),
																			is_request_completed(false),
																			is_request_body_parsing(false),
																			is_request_body_parsed(false),
																			request_has_body(false),
																			tail_appended_body(false),
																			continue_reading_body(false),
																			is_100_continue(false),
																			is_100_continue_sent(false) {
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
		this->host_server = obj.host_server;
		this->timestamp = obj.timestamp;
		this->buffer = obj.buffer;
		this->request = obj.request;
		this->response = obj.response;
		this->is_line_request_received = obj.is_line_request_received;
		this->is_header_received = obj.is_header_received;
		this->is_request_parsed = obj.is_request_parsed;
		this->is_request_completed = obj.is_request_completed;
		this->is_request_body_parsing =	obj.is_request_body_parsing;
		this->request_has_body = obj.request_has_body;
		this->body_buffer = obj.body_buffer;
		this->is_request_body_parsed = obj.is_request_body_parsed;
		this->tail_appended_body = obj.tail_appended_body;
		this->continue_reading_body = obj.continue_reading_body;
		this->is_100_continue = obj.is_100_continue;
		this->is_100_continue_sent = obj.is_100_continue_sent;
	}
	return (*this);
}
