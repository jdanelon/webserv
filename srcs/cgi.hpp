#ifndef CGI_HPP
# define CGI_HPP

# include <cstdio>
# include <sys/wait.h>
# include <csignal>

# include "http/HttpRequest.hpp"
# include "utils.hpp"

std::string	handle_cgi( std::string bin, std::string script, HttpRequest request, std::string root );

#endif
