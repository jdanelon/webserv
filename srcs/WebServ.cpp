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
		this->pollfds = obj.pollfds;
		this->client_connections = obj.client_connections;
		this->has_closed_connections = obj.has_closed_connections;
		this->_backlog = obj._backlog;
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
	int timeout = this->client_connections[fd].host->timeout;

	if (fd == -1)
		return (false);
	if (timestamp() - this->client_connections[fd].timestamp >= timeout)
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
		if (this->client_connections.size() == this->_backlog)
		{
			close(client_fd);
			return ;
		}

		struct pollfd poll_client;
		poll_client.fd = client_fd;
		poll_client.events = POLLIN;
		poll_client.revents = 0;
		this->pollfds.push_back(poll_client);

		ClientConnection client_connection(this->servers[server_fd], timestamp());
		this->client_connections[client_fd] = client_connection;

		client_fd = accept(server_fd, NULL, NULL);
	}
}

void	WebServ::parse_request( int idx )
{
	int client_fd = this->pollfds[idx].fd;
	if (this->client_connections[client_fd].is_header_received &&
			!this->client_connections[client_fd].is_request_parsed) {
		HttpRequest request;
		request.parse(this->client_connections[client_fd].buffer);
		this->client_connections[client_fd].is_request_parsed = true;

		// TO-DO: Check Redirections
		request.validate(std::getenv("PWD") + this->client_connections[client_fd].host->root);

		request.print();
		this->client_connections[client_fd].request = request;
		this->client_connections[client_fd].is_request_completed = true;
	}
}

void	WebServ::create_response( int idx )
{
	int 		client_fd = this->pollfds[idx].fd;
	HttpRequest request = this->client_connections[client_fd].request;
	int			code = request.get_error_code();

	// TO-DO: Create response class with its message and member functions for each allowed method
	std::string response;

	if (!this->client_connections[client_fd].is_request_completed)
		return ;

	// TO-DO: Set response messages where errors happen on parsing and validation
	//
	// Afterwards each method function shall be caught and redirected
	// as it should and responses with error or not will be set
	if (code != 0)
	{}
	if (request.method == "HEAD")
	{
		response += request.version + " 200 OK\r\n";
		std::map<std::string, std::string, CaseInsensitive>::iterator it;
		for (it = request.headers.begin(); it != request.headers.end(); ++it)
			response += it->first + ": " + it->second + "\r\n";
		response += "\r\n";
	}
	if (request.method == "GET")
	{}
	if (request.method == "POST")
	{}
	if (request.method == "DELETE")
	{}

	// VALID MESSAGE JUST TO CLOSE CURL CONNECTION
	if (response.empty())
	{
		response += "HTTP/1.1 200 OK\r\n";
		std::map<std::string, std::string, CaseInsensitive>::iterator it;
		for (it = request.headers.begin(); it != request.headers.end(); ++it)
			response += it->first + ": " + it->second + "\r\n";
		response += "Content-Length: 25\r\n";
		response += "\r\n";
		response += "RESPONSE NOT CONFIGURED\r\n";
	}

	this->client_connections[client_fd].response = response;
	this->pollfds[idx].events = POLLOUT;
}

void	WebServ::send_response( int idx )
{
	int 		client_fd = this->pollfds[idx].fd;
	std::string response = this->client_connections[client_fd].response;

	send(client_fd, response.c_str(), response.length(), 0);
	this->pollfds[idx].events = POLLIN;
	this->client_connections[client_fd].timestamp = timestamp();

	// TO-DO: Function to clear buffer, request, response, bool checks as below
	//
	// I considered clearing with the second constructor, but I do not know if we
	// could have memory leaks due to the allocated Server in host
	this->client_connections[client_fd].buffer = "";
	this->client_connections[client_fd].request = HttpRequest();
	this->client_connections[client_fd].response = "";
	this->client_connections[client_fd].is_line_request_received = false;
	this->client_connections[client_fd].is_header_received = false;
	this->client_connections[client_fd].is_request_parsed = false;
	this->client_connections[client_fd].is_request_completed = false;
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
