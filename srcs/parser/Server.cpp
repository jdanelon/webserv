#include "Server.hpp"

Server::Server( void )
{
	this->host = "";
	this->port = "";
	this->root = "";
	this->timeout = 0;
	this->client_max_body_size = -1;
	// this->access_log = "";
	// this->error_log = "";
	this->autoindex = -1;
	this->redirect = std::make_pair(0, "");
	this->upload = -1;
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
		// this->access_log = obj.access_log;
		// this->error_log = obj.error_log;
		this->autoindex = obj.autoindex;
		this->cgi = obj.cgi;
		this->redirect = obj.redirect;
		this->upload = obj.upload;
		this->upload_store = obj.upload_store;
		this->location = obj.location;
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
	if (this->port.empty())
		this->port = "8080";
	if (this->server_name.empty())
	{
		this->server_name.push_back("localhost");
		this->server_name.push_back("www.localhost");
	}
	if (this->root.empty())
		this->root = "/root";
	if (this->index.empty())
	{
		this->index.push_back("index.html");
		this->index.push_back("index.php");
	}
	if (this->error_page.empty())
		this->error_page[404] = "/error_page_404.html";
	if (this->timeout == 0)
		this->timeout = 30000;
	if (this->client_max_body_size == -1)
		this->client_max_body_size = 1000000;
	if (this->autoindex == -1)
		this->autoindex = 0;
	if (this->upload == -1)
		this->upload = 0;
	if (upload_store.empty())
		this->upload_store = "/upload";
	std::map<std::string, Location>::iterator it;
	for (it = this->location.begin(); it != this->location.end(); it++)
		it->second.fill_with_defaults(*this);
}

void	Server::connect_socket( int backlog )
{
	int				status, yes = 1;
	struct addrinfo	hints, *res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	status = getaddrinfo(this->host.c_str(), this->port.c_str(), &hints, &res);
	if (status != 0)
		throw Server::SocketError("getaddrinfo");
	this->sockfd = socket(res->ai_family, res->ai_socktype, 0);
	if (this->sockfd == -1)
		throw Server::SocketError("socket");
	status = fcntl(this->sockfd, F_SETFL, O_NONBLOCK);
	if (status == -1)
		throw Server::SocketError("fcntl");
	status = setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	if (status == -1)
		throw Server::SocketError("setsockopt");
	status = bind(this->sockfd, res->ai_addr, res->ai_addrlen);
	if (status == -1)
		throw Server::SocketError("bind");
	freeaddrinfo(res);
	status = listen(this->sockfd, backlog);
	if (status == -1)
		throw Server::SocketError("listen");
}

Server::SocketError::SocketError( std::string const &str ) : ParserException(str)
{
	this->_msg = "Error: At system call '" + str + "'!";
}

char const	*Server::SocketError::what( void ) const throw()
{
	return (this->_msg.c_str());
}
