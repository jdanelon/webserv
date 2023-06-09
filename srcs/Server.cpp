#include "Server.hpp"

Server::Server( void )
{
	this->host = "";
	this->port = -1;
	this->root = "";
	this->timeout = 0;
	this->client_max_body_size = -1;
	this->access_log = "";
	this->error_log = "";
	this->autoindex = -1;
	this->upload_store = "";
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
	if (this->host.empty())
		this->host = "127.0.0.1";
	if (this->port == -1)
		this->port = htons(80);
	if (this->root.empty())
		this->root = "";
	if (this->index.empty())
	{
		this->index.push_back("index.html");
		this->index.push_back("index.php");
	}
	// error_page;
	if (this->timeout == 0)
		this->timeout = 30000;
	if (this->client_max_body_size)
		this->client_max_body_size = 1000000;
	if (this->autoindex == -1)
		this->autoindex = 0;
	// redirect;
	std::map<std::string, Location>::iterator it;
	for (it = location.begin(); it != location.end(); it++)
		it->second.fill_with_defaults(*this);
	if (upload_store.empty())
		this->upload_store = "";
	return ;
}

bool	Server::missing_directives( void )
{
	if (this->server_name.empty())
	{
		this->err = "server_name";
		return (true);
	}
	if (this->location.empty())
	{
		this->err = "location";
		return (true);
	}
	return (false);
}
