#include "WebServ.hpp"

WebServ::WebServ( void )
{
	return ;
}

WebServ::WebServ( WebServ const &obj )
{
	WebServ::operator = (obj);
	return ;
}

WebServ &WebServ::operator = ( WebServ const &obj )
{
	if (this != &obj)
	{
		this->parser = obj.parser;
		this->servers = obj.servers;
		this->clients = obj.clients;
		this->pollfds = obj.pollfds;
	}
	return (*this);
}

WebServ::~WebServ( void )
{
	std::map<int, Server *>::iterator it;
	for (it = this->servers.begin(); it != this->servers.end(); it++)
		delete it->second;
	return ;
}

void	WebServ::init( char *file )
{
	this->parser.load(file);
	this->_catch_signals();
	this->_init_servers();
}

void	WebServ::_catch_signals( void )
{
	// *in case of signal to interrupt/end program, free allocated memory and exit program 
	// signal(SIGINT, signal_handler);
	// signal(SIGQUIT, signal_handler);
	// exit(128 + signal_code);
}

void	WebServ::_init_servers( void )
{
	for (int i = 0; i < this->parser.size(); i++)
	{
		Server *srv = new Server(this->parser[i]);
		try
		{
			srv->connect_socket(this->parser.backlog);
		}
		catch(const std::exception& e)
		{
			delete srv;
			throw e;
		}
		this->servers[srv->sockfd] = srv;
		struct pollfd server;
		server.fd = srv->sockfd;
		server.events = POLLIN;
		server.revents = 0;
		this->pollfds.push_back(server);
	}
}

bool	WebServ::client_timeout( int idx )
{
	int	fd = this->pollfds[idx].fd;
	int	timeout = this->clients[fd].host->timeout;
	if (timestamp() - this->clients[fd].timestamp >= timeout)
		return (true);
	return (false);
}

void	WebServ::end_client_connection( int idx )
{
	this->pollfds[idx].fd = -1;
	// close(this->pollfds[idx].fd);
}

void	WebServ::accept_queued_connections( int idx )
{
	int server_fd = this->pollfds[idx].fd;
	int client_fd = accept(server_fd, NULL, NULL);
	while (client_fd != -1)
	{
		struct pollfd poll_client;
		poll_client.fd = client_fd;
		poll_client.events = POLLIN;
		poll_client.revents = 0;
		this->pollfds.push_back(poll_client);

		t_client cli;
		cli.host = this->servers[server_fd];
		cli.timestamp = timestamp();
		this->clients[client_fd] = cli;

		client_fd = accept(server_fd, NULL, NULL);
	}
}
