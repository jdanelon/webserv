#ifndef SERVER_FUNCTIONS_HPP
# define SERVER_FUNCTIONS_HPP

# include "WebServ.hpp"

void process_server_event(WebServ &webserv, unsigned int i);
void process_client_event(WebServ &webserv, unsigned int i);
void poll_events(WebServ &webserv);
void run_server(WebServ &webserv, char *config_file);

#endif
