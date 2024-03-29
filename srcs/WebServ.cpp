#include "WebServ.hpp"

bool WebServ::debugEnabled = false;
const std::string WebServ::className = "Webserv";

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

void	WebServ::init( char const *file )
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
	long long	elapsed_time = timestamp() - this->client_connections[fd].timestamp;
	if (elapsed_time >= timeout)
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

		client_fd = accept(server_fd, NULL, NULL); // ?? again
	}
}

void	WebServ::parse_request_headers( int idx )
{
	int client_fd = this->pollfds[idx].fd;

	// First time reading the request, we create it
	if (this->client_connections[client_fd].is_header_received &&
			!this->client_connections[client_fd].is_request_parsed) {
		HttpRequest request;
		request.parse(this->client_connections[client_fd].buffer);
		this->client_connections[client_fd].is_request_parsed = true;

		// Saved resource full_path to facilitate response
		request.validate_headers(this->client_connections[client_fd].host_server);

		request.print(client_fd);
		this->client_connections[client_fd].request = request;

		// If request has body, we inform the connection so he can start parsing it
		if (request.has_body) {
			int npos = this->client_connections[client_fd].buffer.find("\r\n\r\n");
			this->client_connections[client_fd].body_buffer = this->client_connections[client_fd].buffer.substr(npos + 4);
			this->client_connections[client_fd].request_has_body = true;
			this->client_connections[client_fd].is_request_body_parsing = true;

			// Check if we have the full body in the buffer
			bool continue_reading_body = true;
			std::map<std::string, std::string>::iterator it;
			it = request.headers.find("Content-Length");
			size_t content_length = 0;
			// If content-length is present, we check if the body is complete

			if (it != request.headers.end()) {
				content_length = atoi(it->second.c_str());
				if (content_length == this->client_connections[client_fd].body_buffer.length()) {
					continue_reading_body = false;
				}
			}
			this->client_connections[client_fd].continue_reading_body = continue_reading_body;

			// Check if we have to send 100 continue
			it = request.headers.find("Expect");
			if (it != request.headers.end() && it->second == "100-continue") {
				this->client_connections[client_fd].is_100_continue = true;
			}
		}
		else {
			debug(INFO, "Request has no body");
			this->client_connections[client_fd].is_request_completed = true;
		}
	}
}

void	WebServ::parse_request_body( int idx ) {
	debug(INFO, "Parsing request body");
	int client_fd = this->pollfds[idx].fd;

	// We call the parser function of the request
	this->client_connections[client_fd].request.parse_body(
			this->client_connections[client_fd].body_buffer
		);
	debug(INFO, "Request body parsed");

	// If the request is completed, we inform the connection
	if (client_connections[client_fd].request.is_body_parsed) {
		this->client_connections[client_fd].is_request_body_parsed = true;
		this->client_connections[client_fd].is_request_completed = true;

		Server *srv = this->client_connections[client_fd].host_server;
		std::string bodyBuffer = this->client_connections[client_fd].body_buffer;
		this->client_connections[client_fd].request.validate_body(srv);
	}
}

void	WebServ::create_response( int idx )
{
	int 			client_fd = this->pollfds[idx].fd;
	HttpRequest		request = this->client_connections[client_fd].request;
	HttpResponse	http_response(this->client_connections[client_fd].host_server);

	if (!this->client_connections[client_fd].is_request_completed)
		return ;
	http_response.configureResponse(request);

	this->client_connections[client_fd].response = http_response;

	this->pollfds[idx].events = POLLOUT;
}

void	WebServ::_clear_connection(int const client_fd) {
	this->client_connections[client_fd].buffer = "";
	this->client_connections[client_fd].body_buffer = "";
	this->client_connections[client_fd].request = HttpRequest();
	this->client_connections[client_fd].response = HttpResponse(this->client_connections[client_fd].host_server);
	this->client_connections[client_fd].is_line_request_received = false;
	this->client_connections[client_fd].is_header_received = false;
	this->client_connections[client_fd].is_request_parsed = false;
	this->client_connections[client_fd].is_request_completed = false;
	this->client_connections[client_fd].is_request_body_parsing = false;
	this->client_connections[client_fd].is_request_body_parsed = false;
	this->client_connections[client_fd].request_has_body = false;
	this->client_connections[client_fd].tail_appended_body = false;
	this->client_connections[client_fd].is_100_continue_sent = false;
}

void	WebServ::_send( int idx, std::string const &response )
{
	size_t	total_bytes = 0, nbytes;
	int		client_fd = this->pollfds[idx].fd;

	while (total_bytes < response.length())
	{
		nbytes = send(client_fd, response.c_str(), response.length(), 0);
		if (nbytes <= 0)
		{
			if (nbytes == 0) 
				std::cout << "socket: '" << client_fd << "' hung up" << std::endl;
			else {
				std::cout << "Send error: " << nbytes << std::endl;
			}
			this->end_client_connection(idx);
			this->client_connections.erase(client_fd);
			break ;
		}
		total_bytes += nbytes;
	}
}

void	WebServ::send_response( int idx )
{
	int				client_fd = this->pollfds[idx].fd;
	HttpResponse	http_response = this->client_connections[client_fd].response;

	// 1) If request is not valid, send error response
	if (!this->client_connections[client_fd].response.is_request_valid) {
		debug(INFO, "Sending error response...");
		HttpResponse http_response = this->client_connections[client_fd].response;
		http_response.prepareErrorResponse(this->client_connections[client_fd].request);
		this->_send(idx, http_response.getResponse());
	}
	// 2) If the response is not too big, send the full response
	else if (this->client_connections[client_fd].response.fileSize < RESPONSE_CHUNK_SIZE) {
		HttpResponse http_response = this->client_connections[client_fd].response;
		try {
			http_response.prepareFullResponse();
			this->_send(idx, http_response.getResponse());
		}
		catch(const std::exception& e) {
			std::cerr << e.what() << '\n';
			http_response.prepareErrorResponse(this->client_connections[client_fd].request);
			this->_send(idx, http_response.getResponse());
		}
		this->pollfds[idx].events = POLLIN;
	}
	// 3) If the response is big, send it in chunks so we do not block the socket
	else {
		try {
			std::string chunkData = this->client_connections[client_fd].response.readChunkAndUpdateResponse(RESPONSE_CHUNK_SIZE);

			// Check if we send the first chunk with the headers or we just send the other chunks (no headers)
			if (this->client_connections[client_fd].response.fileOffset <= RESPONSE_CHUNK_SIZE) {
				std::string fullResponse = this->client_connections[client_fd].response.getResponse() + chunkData;
				this->_send(idx, fullResponse);
				debug(INFO, "Sending first chunk response...");
			} else {
				// Otherwise, just send the chunk
				debug(INFO, "Sending chunk...");
				this->_send(idx, chunkData);
			}
		}
		catch (const std::exception& e) {
			std::cerr << e.what() << '\n';
			HttpResponse http_response = this->client_connections[client_fd].response;
			http_response.prepareErrorResponse(this->client_connections[client_fd].request);
			this->_send(idx, http_response.getResponse());
			this->client_connections[client_fd].response.fileOffset = this->client_connections[client_fd].response.fileSize;
		}

		// Check if we have sent the full file, If not we go back to the poll loop
		unsigned int file_offset = this->client_connections[client_fd].response.fileOffset;
		if (client_fd != -1 && file_offset != 0 && file_offset < this->client_connections[client_fd].response.fileSize) {
			this->client_connections[client_fd].timestamp = timestamp();
			this->pollfds[idx].events = POLLOUT;  // Set to send more data
			return ;
		} else {
			this->client_connections[client_fd].response.fileHandle.close();
		}
	}

	size_t	last_bar = this->client_connections[client_fd].response.resourceFullPath.find_last_of("/");
	size_t	extension_idx = this->client_connections[client_fd].response.resourceFullPath.find_last_of(".");
	if (extension_idx == std::string::npos)
		extension_idx = 0;
	std::string	extension = this->client_connections[client_fd].response.resourceFullPath.substr(extension_idx);
	if (last_bar != std::string::npos && extension == ".tmp")
		std::remove(this->client_connections[client_fd].response.resourceFullPath.c_str());

	// Reset connection if it has not been closed
	if (this->pollfds[idx].fd != -1)
	{
		this->client_connections[client_fd].timestamp = timestamp();
		this->pollfds[idx].events = POLLIN;
		this->_clear_connection(client_fd);
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

void	WebServ::_clear_100_continue(int const client_fd) {
	this->client_connections[client_fd].response = HttpResponse(this->client_connections[client_fd].host_server);
	this->client_connections[client_fd].is_100_continue = false;
	this->client_connections[client_fd].is_100_continue_sent = true;
}

void	WebServ::handle_100_continue( int idx ) {
	int client_fd = this->pollfds[idx].fd;

	HttpResponse http_response(this->client_connections[client_fd].host_server);
	http_response.prepare100ContinueResponse();

	this->client_connections[client_fd].response = http_response;	
	this->client_connections[client_fd].is_100_continue_sent = false;
	this->pollfds[idx].events = POLLOUT;
}

void	WebServ::send_100_continue( int idx ) {
	int client_fd = this->pollfds[idx].fd;

	// 1) If request is not valid, send error response
	if (!this->client_connections[client_fd].response.is_request_valid) {
		HttpResponse http_response = this->client_connections[client_fd].response;
		http_response.prepareErrorResponse(this->client_connections[client_fd].request);
		send(client_fd, http_response.getResponse().c_str(), http_response.getResponse().length(), 0);
		this->_clear_connection(client_fd);
	}
	// 2) We send the response already configured
	else {
		HttpResponse http_response = this->client_connections[client_fd].response;
		http_response.prepare100ContinueResponse();
		send(client_fd, http_response.getResponse().c_str(), http_response.getResponse().length(), 0);
		this->_clear_100_continue(client_fd);
	}

	this->pollfds[idx].events = POLLIN;

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

void WebServ::debug(LogLevel level, const std::string& message) {
	if (!debugEnabled) {
		return;
	}

	std::string prefix;
	std::string colorCode;
	switch (level) {
	case INFO:
		prefix = "[INFO] ";
		colorCode = "\033[1;34m";  // Blue
		break;
	case WARNING:
		prefix = "[WARNING] ";
		colorCode = "\033[1;33m";  // Yellow
		break;
	case ERROR:
		prefix = "[ERROR] ";
		colorCode = "\033[1;31m";  // Red
		break;
	}

	std::cout << colorCode << className << " " << prefix << message << "\033[0m" << std::endl;  // \033[0m resets the color
}
