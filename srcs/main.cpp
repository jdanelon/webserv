#include "../includes/main.hpp"
#include "WebServ.hpp"

static int	validateFile( char *file )
{
	std::string			tmp(file);
	std::ifstream		in(file);
	std::stringstream	strFile;
	size_t				left = 0;
	size_t				right = 0;

	if (tmp.empty())
		return (1);
	if (in.fail())
		return (1);
	strFile << in.rdbuf();
	in.close();
	for (size_t i = 0; i < strFile.str().length(); i++)
	{
		if (strFile.str()[i] == '{')
			left++;
		if (strFile.str()[i] == '}')
			right++;
	}
	if (left != right)
		return (1);
	return (0);
}

int	main( int argc, char **argv )
{
	WebServ	webserv;

	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv [configuration file]" << std::endl;
		return (1);
	}
	if (validateFile(argv[1]))
	{
		std::cerr << "Error: file '" << argv[1] << "' is invalid!" << std::endl;
		return (1);
	}
	try
	{
		webserv.init(argv[1]);
		// logica
		while (1)
		{
			// int	num_revents = poll(---, webserv.pollfds.size(), -1);
			// if (num_revents <= 0)
			// 	break ;
			// for (int i = 0; i < webserv.pollfds.size() && num_revents; i++)
			// {
			// 	short revents = webserv.pollfds[i].revents;
			// 	if (revents == 0)
			// 		continue ;
			// 	bool is_server = webserv.servers.count(webserv.pollfds[i].fd);
			// 	if (!is_server && timeout)
			// 	{
			// 		end_client;
			// 		continue ;
			// 	}
			// 	num_revents--;
			// 	if (is_server)
			// 	{
			// 		int client_fd = accept(webserv.pollfds[i].fd, ---, ---);
			// 		if (client_fd == -1)
			// 			break ;
			// 		struct pollfd client;
			// 		client.fd = client_fd;
			// 		client.events = POLLIN;
			// 		webserv.clients.push_back(client_fd);
			// 		webserv.pollfds.push_back(client);
			// 	}
			// 	else
			// 	{
			// 		if (revents & POLLIN)
			// 			---;
			// 		else if (revents & POLLOUT)
			// 			---;
			// 		else
			// 			---;
			// 	}
			// }
			break ;
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	return (0);
}
