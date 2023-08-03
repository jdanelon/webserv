#include "../includes/main.hpp"
#include "WebServ.hpp"

bool read_client_request(WebServ &webserv, unsigned int i) {
    char buf[16];
    int nbytes = recv(webserv.pollfds[i].fd, buf, sizeof(buf) - 1, 0);

    if (nbytes <= 0) {
        if (nbytes == 0) // TO-DO It is supposed that nbytes == 0 means that the client has hung up?
            std::cout << "socket: '" << webserv.pollfds[i].fd << "' hung up" << std::endl;
        else
            std::cout << "recv error" << std::endl;
        webserv.end_client_connection(i);
		webserv.client_connections.erase(i);
        return false;
    }
    buf[nbytes] = '\0';
    webserv.client_connections[i].buffer.append(buf);

	// This means that the header has been fully received
	if (webserv.client_connections[i].buffer.find("\r\n\r\n") != std::string::npos) {
		webserv.client_connections[i].is_header_received = true;
		return true;
	}

    return true; // Return true if successful, false if connection should be closed
}

void process_server_event(WebServ &webserv, unsigned int i) {
    webserv.accept_queued_connections(i);
}

void process_client_event(WebServ &webserv, unsigned int i) {
    bool is_error = webserv.pollfds[i].revents & (POLLERR | POLLHUP | POLLNVAL);
    bool is_input_ready = webserv.pollfds[i].revents & POLLIN;
    bool is_output_ready = webserv.pollfds[i].revents & POLLOUT;

    if (is_error) {
        webserv.end_client_connection(i);
    }
    else if (is_input_ready) {
		// Read the client request from the socket
        if (!read_client_request(webserv, i)) {
            return;
        }
		if (webserv.client_connections[i].is_header_received) {
			HttpRequest request;
			request.parse(webserv.client_connections[i].buffer);
			request.validate();
			request.print();
			webserv.client_connections[i].request = request;
		}
    }
    else if (is_output_ready) {
        // send response to client
    }
}

void poll_events(WebServ &webserv) {
    while (g_signal_code == 0) {
        int	num_revents = poll(&webserv.pollfds[0], webserv.pollfds.size(), -1);
        if (num_revents < 0)
            break;
        for (unsigned int i = 0; i < webserv.pollfds.size(); i++) {
            bool is_server = webserv.servers.count(webserv.pollfds[i].fd);
            if (!is_server && webserv.client_timeout(i)) {
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

void run_server(WebServ &webserv, char *config_file) {
    webserv.init(config_file);
    poll_events(webserv);
}

// 	--ready to read client socket
// 	*parse client request
// 		The normal procedure for parsing an HTTP message is to read the
// 		start-line into a structure, read each header field into a hash table
// 		by field name until the empty line, and then use the parsed data to
// 		determine if a message body is expected.  If a message body has been
// 		indicated, then it is read as a stream until an amount of octets
// 		equal to the message body length is read or the connection is closed.
// 	*read msg until end
// 	*create and set response
// 	*set event to POLLOUT
// char buf[256];
// int nbytes = recv(webserv.pollfds[i].fd, buf, sizeof(buf), 0);
// if (nbytes <= 0)
// {
// 	if (nbytes == 0)
// 		std::cout << "socket: '" << webserv.pollfds[i].fd << "' hung up" << std::endl;
// 	else
// 		std::cout << "recv error" << std::endl;
// 	webserv.end_client_connection(i);
// }
// else
// {
// 	buf[nbytes] = '\0';
// 	std::cout << "from socket '" << webserv.pollfds[i].fd << "': ";
// 	std::cout << buf << std::endl;
// }
// webserv.pollfds[i].events = POLLOUT;
// }
// else if (revents & POLLOUT)
// {
// 	--ready to write at client socket
// 	*send response message
// 	*set event to POLLIN
// send(webserv.pollfds[i].fd, "msg received\n", 13, 0);
// webserv.pollfds[i].events = POLLIN;
// webserv.clients[webserv.pollfds[i].fd].timestamp = timestamp();