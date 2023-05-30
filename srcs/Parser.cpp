#include "Parser.hpp"

Parser::Parser( void )
{
	return ;
}

Parser::Parser( Parser const &obj )
{
	Parser::operator = (obj);
	return ;
}

Parser &Parser::operator = ( Parser const &obj )
{
	if (this != &obj)
	{
		this->_cleanFile = obj._cleanFile;
		this->_servers = obj._servers;
	}
	return (*this);
}

Parser::~Parser( void )
{
	return ;
}

void	Parser::load( char *file )
{
	this->_readFile(file);
	std::istringstream	istr(_cleanFile);
	this->_parse(&istr);
}

void	Parser::_readFile( char *file )
{
	std::ifstream		in;
	std::stringstream	strFile;
	size_t				idx_comment;
	size_t				idx_endl;
	std::string			inFile;

	in.open(file);
	strFile << in.rdbuf();
	in.close();
	inFile = strFile.str();
	idx_comment = inFile.find('#');
	while (idx_comment != std::string::npos)
	{
		idx_endl = inFile.find('\n', idx_comment);
		inFile.erase(idx_comment, idx_endl - idx_comment);
		idx_comment = inFile.find('#');
	}
	this->_cleanFile = inFile;
}

std::vector<std::string>	Parser::_get_split_lines( std::string line )
{
	size_t						end_line, lim, end_field;
	std::vector<std::string>	vecDir;

	end_line = line.find(';');
	lim = end_line < line.length() ? end_line : line.length();
	for (size_t i = 0; i < lim; i++)
	{
		while (ft_isspace(line[i]))
			i++;
		end_field = line.find_first_of(" \f\n\r\t\v", i);
		if (end_line < end_field)
			vecDir.push_back(line.substr(i, end_line - i));
		else
			vecDir.push_back(line.substr(i, end_field - i));
		i += vecDir.back().length();
		if (vecDir.back().length() == 0)
			vecDir.pop_back();
	}
	if (end_line == std::string::npos && vecDir[0].compare("server") &&
			vecDir[0].compare("location") && vecDir[0].compare("}") &&
			vecDir[0].compare("http"))
		vecDir.clear();
	return (vecDir);
}

Location	Parser::_parse_location( std::istringstream *istr )
{
	std::string					line, directive;
	std::vector<std::string>	tokens;
	Location					loc;
	ParserHelper				helper;

	while (std::getline(*istr, line))
	{
		if (line.length() == 0)
			continue ;
		tokens = this->_get_split_lines(line);
		if (tokens.empty())
			throw ParserHelper::InvalidLine(line);
		directive = tokens[0];
		if (helper.duplicated_directives(tokens))
			throw ParserHelper::DuplicatedDirectives(directive);
		if (!directive.compare("root"))
			loc.root = helper.get_root();
		// else if (!directive.compare("index"))
		// 	loc.index = helper.get_index();
		// else if (!directive.compare("limit_except"))
		// 	loc.limit_except = helper.get_limit_except();
		// else if (!directive.compare("client_max_body_size"))
		// 	loc.client_max_body_size = helper.get_client_max_body_size();
		// else if (!directive.compare("autoindex"))
		// 	loc.autoindex = helper.get_autoindex();
		// else if (!directive.compare("cgi"))
		// else if (!directive.compare("return"))
		// 	loc.redirect = helper.get_redirect();
		// else if (!directive.compare("upload"))
		// 	loc.upload = helper.get_upload();
		// else if (!directive.compare("upload_store"))
		// 	loc.upload_store = helper.get_upload_store();
		else if (!directive.compare("}"))
			break ;
		else
			throw ParserHelper::UnknownDirective(directive);
	}
	return (loc);
}

Server	Parser::_parse_servers( std::istringstream *istr )
{
	std::string					line, directive;
	std::vector<std::string>	tokens;
	Server						srv;
	ParserHelper				helper;

	while (std::getline(*istr, line))
	{
		if (line.length() == 0)
			continue ;
		tokens = this->_get_split_lines(line);
		if (tokens.empty())
			throw ParserHelper::InvalidLine(line);
		directive = tokens[0];
		if (helper.duplicated_directives(tokens))
			throw ParserHelper::DuplicatedDirectives(directive);
		if (!directive.compare("listen"))
		{
			srv.ip = helper.get_listen().first;
			srv.port = helper.get_listen().second;
		}
		// else if (!directive.compare("server_name"))
		// 	srv.server_name = helper.get_server_name();
		else if (!directive.compare("root"))
			srv.root = helper.get_root();
		// else if (!directive.compare("index"))
		// 	srv.index = helper.get_index();
		// else if (!directive.compare("error_page"))
		// {}
		// else if (!directive.compare("timeout"))
		// 	srv.timeout = helper.get_timeout();
		// else if (!directive.compare("client_max_body_size"))
		// 	srv.client_max_body_size = helper.get_client_max_body_size();
		// else if (!directive.compare("access_log"))
		// 	srv.access_log = helper.get_access_log();
		// else if (!directive.compare("error_log"))
		// 	srv.error_log = helper.get_error_log();
		// else if (!directive.compare("autoindex"))
		// 	srv.autoindex = helper.get_autoindex();
		// else if (!directive.compare("cgi"))
		// else if (!directive.compare("return"))
		// 	srv.redirect = helper.get_redirect();
		// else if (!directive.compare("upload"))
		// 	srv.upload = helper.get_upload();
		// else if (!directive.compare("upload_store"))
		// 	srv.upload_store = helper.get_upload_store();
		// else if (!directive.compare("location"))
		// 	this->_parse_location(istr);
		else if (!directive.compare("}"))
			break ;
		else
			throw ParserHelper::UnknownDirective(directive);
	}
	return (srv);
}

void	Parser::_parse( std::istringstream *istr )
{
	std::string					line, directive;
	std::vector<std::string>	tokens;
	ParserHelper				helper;

	while (std::getline(*istr, line))
	{
		if (line.length() == 0)
			continue ;
		tokens = this->_get_split_lines(line);
		if (tokens.empty())
			throw ParserHelper::InvalidLine(line);
		directive = tokens[0];
		if (helper.duplicated_directives(tokens))
			throw ParserHelper::DuplicatedDirectives(directive);
		if (!directive.compare("server"))
			this->_servers.push_back(this->_parse_servers(istr));
	}
}
