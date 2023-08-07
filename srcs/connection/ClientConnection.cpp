#include "ClientConnection.hpp"

ClientConnection::ClientConnection( void ) : is_request_parsed(false) , is_request_completed(false) , is_header_received(false)
{
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
		this->buffer = obj.buffer;
		this->request = obj.request;
		this->response = obj.response;
		this->is_header_received = obj.is_header_received;
		this->is_request_parsed = obj.is_request_parsed;
		this->is_request_completed = obj.is_request_completed;
	}
	return (*this);
}