#ifndef MAIN_HPP
# define MAIN_HPP

# include <iostream>
# include <cstring>
# include <locale>
# include <vector>
# include <map>
# include "../srcs/WebServ.hpp"

void process_server_event(WebServ &webserv, unsigned int i);
void process_client_event(WebServ &webserv, unsigned int i);
void poll_events(WebServ &webserv);
void run_server(WebServ &webserv, char *config_file);

#endif
