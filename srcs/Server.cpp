#include "Server.hpp"

Server::Server( void )
{
	return ;
}

Server::Server( Server const &obj )
{
	Server::operator = (obj);
	return ;
}

Server &Server::operator = ( Server const &obj )
{
	if (this != &obj)
	{
		this->host = obj.host;
		this->port = obj.port;
		this->server_name = obj.server_name;
		this->root = obj.root;
		this->index = obj.index;
		this->error_page = obj.error_page;
		this->timeout = obj.timeout;
		this->client_max_body_size = obj.client_max_body_size;
		this->access_log = obj.access_log;
		this->error_log = obj.error_log;
		this->autoindex = obj.autoindex;
		this->redirect = obj.redirect;
		this->upload = obj.upload;
		this->upload_store = obj.upload_store;
	}
	return (*this);
}

Server::~Server( void )
{
	return ;
}

void	Server::fill_with_defaults( void )
{
	return ;
}
