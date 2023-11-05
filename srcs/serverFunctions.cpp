#include "../includes/main.hpp"
#include "WebServ.hpp"

bool	read_client_request_headers( WebServ &webserv, unsigned int i ) {
	int	client_fd = webserv.pollfds[i].fd;

	char	buf[16];
	std::memset(buf, '\0', 16);

	int	nbytes = recv(client_fd, buf, sizeof(buf) - 1, 0);

	if (nbytes <= 0) {
		if (nbytes == 0) 
			std::cout << "socket: '" << client_fd << "' hung up" << std::endl;
		else {
			std::cout << "Recv error: " << nbytes << std::endl;
		}
		webserv.end_client_connection(i);
		webserv.client_connections.erase(client_fd);
		return (false);
	}
	buf[nbytes] = '\0';
	webserv.client_connections[client_fd].buffer.append(buf);

	// This means that the header has been fully received
	if (webserv.client_connections[client_fd].buffer.find("\r\n\r\n") != std::string::npos) {
		webserv.client_connections[client_fd].is_header_received = true;
		return (true);
	}

	webserv.client_connections[client_fd].timestamp = timestamp();

	return (true); // Return true if successful, false if connection should be closed
}

bool	read_client_request_body( WebServ &webserv, unsigned int i ) {
	int	client_fd = webserv.pollfds[i].fd;

	char	buf[256];
	std::memset(buf, '\0', 256);

	int	nbytes = recv(client_fd, buf, sizeof(buf) - 1, 0);

	if (nbytes <= 0) {
		if (nbytes == 0)
			std::cout << "socket: '" << client_fd << "' hung up" << std::endl;
		else {
			std::cout << "Recv error: " <<  nbytes<< std::endl;
		}
		webserv.end_client_connection(i);
		webserv.client_connections.erase(client_fd);
		return (false);
	}
	buf[nbytes] = '\0';
	if (webserv.client_connections[client_fd].tail_appended_body) {
		webserv.client_connections[client_fd].body_buffer = std::string(buf, nbytes);
	}
	else {
		webserv.client_connections[client_fd].body_buffer.append(std::string(buf, nbytes));
		webserv.client_connections[client_fd].tail_appended_body = true;
	}

	webserv.client_connections[client_fd].timestamp = timestamp();

	return (true); // Return true if successful, false if connection should be closed
}

void	process_server_event( WebServ &webserv, unsigned int i ) {
	webserv.accept_queued_connections(i);
}

void	process_client_event( WebServ &webserv, unsigned int i ) {
	bool	is_error = webserv.pollfds[i].revents & (POLLERR | POLLHUP | POLLNVAL);
	bool	is_input_ready = webserv.pollfds[i].revents & POLLIN;
	bool	is_output_ready = webserv.pollfds[i].revents & POLLOUT;

	if (is_error) {
		webserv.end_client_connection(i);
	}
	else if (is_input_ready) {
		int client_fd = webserv.pollfds[i].fd;
		// If header was not received yet, read it
		if (!webserv.client_connections[client_fd].is_header_received) {
			read_client_request_headers(webserv, i);
		}
		// If header was received, but request was not parsed yet, parse it
		// Parse check if there if request has body
		if (webserv.client_connections[client_fd].is_header_received 
			&& !webserv.client_connections[client_fd].is_request_body_parsing
			&& !webserv.client_connections[client_fd].is_request_completed) {
			webserv.parse_request_headers(i);
		}

		// If is 100 continue, and we have not sent 100 continue yet, send it
		if (webserv.client_connections[client_fd].is_100_continue 
			&& !webserv.client_connections[client_fd].is_100_continue_sent) {
			webserv.handle_100_continue(i);
			return;
		}

		// If request has body, read and parse it little by little 
		if (webserv.client_connections[client_fd].is_header_received 
			&& webserv.client_connections[client_fd].request_has_body) {
			if (!webserv.client_connections[client_fd].is_request_body_parsed
				&& webserv.client_connections[client_fd].continue_reading_body)
				read_client_request_body(webserv, i);
			webserv.parse_request_body(i);
		}
		if (webserv.client_connections[client_fd].is_request_completed) {
			webserv.create_response(i);
		}
	}
	else if (is_output_ready) {
		int client_fd = webserv.pollfds[i].fd;

		if (webserv.client_connections[client_fd].is_100_continue
			&& !webserv.client_connections[client_fd].is_100_continue_sent) {
			webserv.send_100_continue(i);
		}
		else {
			webserv.send_response(i);
		}
	}
}

void	poll_events( WebServ &webserv ) {
	while (g_signal_code == 0) {
		int	num_revents = poll(&webserv.pollfds[0], webserv.pollfds.size(), -1);
		if (num_revents < 0) {
			std::cout << "poll error" << std::endl;
			break;
		}
		// webserv.print();
		for (unsigned int i = 0; i < webserv.pollfds.size(); i++) {
			bool is_server = webserv.servers.count(webserv.pollfds[i].fd);
			if (!is_server && webserv.client_timeout(i)) {
				std::cout << "Client timeout" << std::endl;
				webserv.end_client_connection(i);
				continue;
			}
			if (webserv.pollfds[i].revents == 0)
				continue;
			if (is_server)
				process_server_event(webserv, i);
			else
				process_client_event(webserv, i);
		}
		if (webserv.has_closed_connections)
			webserv.purge_connections();
	}
}

void	run_server( WebServ &webserv, char const *config_file ) {
	webserv.init(config_file);
	poll_events(webserv);
}
