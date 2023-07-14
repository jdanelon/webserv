#include "WebServ.hpp"

WebServ::WebServ( void ) : has_closed_connections(false)
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
	this->_backlog = this->parser.backlog;
	this->_catch_signals();
	this->_init_servers();
}

void	WebServ::_catch_signals( void )
{
	// in case of signal to interrupt (CTRL^C)/end program (CTRL^\), call function to handle signal 
	std::signal(SIGINT, WebServ::_signal_handler);
	std::signal(SIGQUIT, WebServ::_signal_handler);
}

void	WebServ::_signal_handler( int const code )
{
	// receive signal code from std::signal and assign it to global variable g_signal_code
	g_signal_code = code;
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

	if (fd == -1)
		return (false);
	if (timestamp() - this->clients[fd].timestamp >= timeout)
		return (true);
	return (false);
}

void	WebServ::end_client_connection( int idx )
{
	close(this->pollfds[idx].fd);
	// remove client and delete reserved client memory from this->clients (?)
	// ...
	// set poll struct array to ignore client_fd and later clear every closed connection (*)
	this->pollfds[idx].fd = -1;
	this->has_closed_connections = true;
}

void	WebServ::accept_queued_connections( int idx )
{
	int server_fd = this->pollfds[idx].fd;
	int client_fd = accept(server_fd, NULL, NULL);
	while (client_fd != -1)
	{
		if (this->clients.size() == this->_backlog)
		{
			close(client_fd);
			return ;
		}

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

void	WebServ::purge_connections( void )
{
	std::vector<struct pollfd>::iterator	it;
	for (it = this->pollfds.begin(); it != this->pollfds.end();)
	{
		if (it->fd == -1)
			it = this->pollfds.erase(it);
		else
			it++;
	}
}
