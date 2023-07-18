#include "../includes/main.hpp"
#include "WebServ.hpp"

int	g_signal_code = 0;

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
		std::cerr << "Error: file '" << argv[1] << "' not found or could not be opened!" << std::endl;
		return (1);
	}
	try
	{
		webserv.init(argv[1]);
		// while there are no signals to interrupt, keep loop and server alive
		// this approach allows the destructor to be called at the end of the function
		while (g_signal_code == 0)
		{
			int	num_revents = poll(&webserv.pollfds[0], webserv.pollfds.size(), -1);
			if (num_revents < 0)
				break ;
			for (unsigned int i = 0; i < webserv.pollfds.size(); i++)
			{
				short revents = webserv.pollfds[i].revents;
				if (revents == 0)
					continue ;
				bool is_server = webserv.servers.count(webserv.pollfds[i].fd);
				if (!is_server && webserv.client_timeout(i))
				{
					webserv.end_client_connection(i);
					continue ;
				}
				if (is_server)
					webserv.accept_queued_connections(i);
				else
				{
					if (revents & (POLLERR | POLLHUP | POLLNVAL))
						webserv.end_client_connection(i);
					// else if (revents & POLLIN)
					// {
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
					// }
					// else if (revents & POLLOUT)
					// {
					// 	--ready to write at client socket
					// 	*send response message
					// 	*set event to POLLIN
					// }	
					// else
					// 	---;
				}
			}
			if (webserv.has_closed_connections)
				webserv.purge_connections();
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	// as the program must end with a signal, there is no return(0)
	// 128 + g_signal_code to have the return code as expected
	// not sure if it is Windows related
	return (128 + g_signal_code);
}
