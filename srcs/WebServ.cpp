#include "WebServ.hpp"

WebServ::WebServ( void ) : has_closed_connections(false)
{
	this->_error_codes_map[100] = "Continue";
	this->_error_codes_map[200] = "OK";
	this->_error_codes_map[201] = "Created";
	this->_error_codes_map[202] = "Accepted";
	this->_error_codes_map[204] = "No Content";
	this->_error_codes_map[301] = "Moved Permanently";
	this->_error_codes_map[302] = "Found";
	this->_error_codes_map[400] = "Bad Request";
	this->_error_codes_map[401] = "Unauthorized";
	this->_error_codes_map[403] = "Forbidden";
	this->_error_codes_map[404] = "Not Found";
	this->_error_codes_map[405] = "Method Not Allowed";
	this->_error_codes_map[408] = "Request Timeout";
	this->_error_codes_map[410] = "Gone";
	this->_error_codes_map[411] = "Length Required";
	this->_error_codes_map[413] = "Payload Too Large";
	this->_error_codes_map[414] = "URI Too Long";
	this->_error_codes_map[415] = "Unsupported Media Type";
	this->_error_codes_map[429] = "Too Many Requests";
	this->_error_codes_map[500] = "Internal Server Error"; // EXAMPLE: TIMEOUT ON CGI
	this->_error_codes_map[501] = "Not Implemented";
	this->_error_codes_map[505] = "HTTP Version Not Supported";

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
		catch(const Server::SocketError& e)
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
	int timeout = this->client_connections[fd].host_server->timeout;

	if (fd == -1)
		return (false);
	if (timestamp() - this->client_connections[fd].timestamp >= timeout)
		return (true);
	return (false);
}

void	WebServ::end_client_connection( int idx )
{
	std::cout << "Closing connection for fd: " << this->pollfds[idx].fd << std::endl;
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

		client_fd = accept(server_fd, NULL, NULL); // ?? again
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

		// Saved resource full_path to facilitate response
		std::string resource_path = request.validate(this->client_connections[client_fd].host_server);

		request.print(client_fd);
		this->client_connections[client_fd].request = request;
		this->client_connections[client_fd].is_request_completed = true;
	}
}

void	WebServ::create_response( int idx )
{
	int 			client_fd = this->pollfds[idx].fd;
	HttpRequest		request = this->client_connections[client_fd].request;
	HttpResponse	http_response;

	if (!this->client_connections[client_fd].is_request_completed)
		return ;
	http_response.generateResponse(request);
	http_response.print(client_fd);

	this->client_connections[client_fd].response = http_response;
	this->pollfds[idx].events = POLLOUT;
}

void	WebServ::_clear_connection( int const client_fd )
{
	this->client_connections[client_fd].buffer = "";
	this->client_connections[client_fd].request = HttpRequest();
	this->client_connections[client_fd].response = HttpResponse();
	this->client_connections[client_fd].is_line_request_received = false;
	this->client_connections[client_fd].is_header_received = false;
	this->client_connections[client_fd].is_request_parsed = false;
	this->client_connections[client_fd].is_request_completed = false;
}

void	WebServ::send_response( int idx )
{
	int 		client_fd = this->pollfds[idx].fd;
	std::string response = this->client_connections[client_fd].response.getResponse();

	send(client_fd, response.c_str(), response.length(), 0);
	this->pollfds[idx].events = POLLIN;
	this->client_connections[client_fd].timestamp = timestamp();
	this->_clear_connection(client_fd);
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

// Debug
void	WebServ::print(void) {
	std::cout << "------------------" << std::endl;
	std::cout << "WebServ" << std::endl;
	std::cout << "Pollfds: " << std::endl;
	for (unsigned int i = 0; i < this->pollfds.size(); i++) {
		std::cout << "fd: " << this->pollfds[i].fd << std::endl;
		std::cout << "events: " << this->pollfds[i].events << std::endl;
		std::cout << "revents: " << this->pollfds[i].revents << std::endl;
	}
	std::cout << "Client connections size: " << this->client_connections.size() << std::endl;
	std::cout << "------------------" << std::endl;
}