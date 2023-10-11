#include "../includes/main.hpp"
#include "WebServ.hpp"
#include "serverFunctions.hpp"

int	g_signal_code = 0;

static int	validateFile( std::string file )
{
	std::ifstream		in(file.c_str());
	std::stringstream	strFile;
	size_t				left = 0;
	size_t				right = 0;

	if (file.empty())
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

	if (argc > 2)
	{
		std::cerr << "Usage: ./webserv [configuration file]" << std::endl;
		return (1);
	}
	std::string	file = (argc == 1) ? "./conf-files/oneServer.conf" : argv[1];
	if (validateFile(file))
	{
		std::cerr << "Error: file '" << file << "' not found or could not be opened!" << std::endl;
		return (1);
	}
	try
	{
		run_server(webserv, file.c_str());
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
