#include "../includes/webserv.hpp"
#include "Parser.hpp"

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
	Parser	parser;

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
		parser.load(argv[1]);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	return (0);
}
