#include "../includes/main.hpp"
#include "WebServ.hpp"

void process_server_event(WebServ &webserv, unsigned int i) {
	if (webserv.pollfds[i].revents == 0)
        return;
    webserv.accept_queued_connections(i);
}

void process_client_event(WebServ &webserv, unsigned int i) {
    if (webserv.pollfds[i].revents == 0)
        return;
    bool is_error = webserv.pollfds[i].revents & (POLLERR | POLLHUP | POLLNVAL);
    bool is_input_ready = webserv.pollfds[i].revents & POLLIN;
    bool is_output_ready = webserv.pollfds[i].revents & POLLOUT;

    if (is_error) {
        webserv.end_client_connection(i);
    }
    else if (is_input_ready) {
        // process input from client
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