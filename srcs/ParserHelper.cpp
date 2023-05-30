#include "ParserHelper.hpp"

ParserHelper::ParserHelper( void )
{
	return ;
}

ParserHelper::ParserHelper( ParserHelper const &obj )
{
	ParserHelper::operator = (obj);
	return ;
}

ParserHelper &ParserHelper::operator = ( ParserHelper const &obj )
{
	if (this != &obj)
	{
		this->_tokens = obj._tokens;
		this->_list = obj._list;
	}
	return (*this);
}

ParserHelper::~ParserHelper( void )
{
	return ;
}

bool	ParserHelper::duplicated_directives( std::vector<std::string> const &tokens )
{
	std::pair<std::set<std::string>::iterator, bool>	ret;

	this->_tokens = tokens;
	if (!tokens[0].compare("server"))
		return (false);
	if (!tokens[0].compare("error_page") || !tokens[0].compare("cgi")
			|| !tokens[0].compare("return") || !tokens[0].compare("location"))
		ret = this->_list.insert(tokens[0] + tokens[1]);
	else
		ret = this->_list.insert(tokens[0]);
	if (ret.second == false)
		return (true);
	return (false);
}

std::pair<in_addr_t, size_t>	ParserHelper::get_listen( void )
{
	in_addr_t	ip;
	size_t		port;

	if (this->_tokens.size() != 2)
		throw InvalidNumberArgs(this->_tokens[0]);
	size_t idx = this->_tokens[1].find(":");
	if (idx != std::string::npos)
	{
		ip = inet_addr(this->_tokens[1].substr(0, idx).c_str());
		port = htons(ft_atoi(this->_tokens[1].substr(idx, this->_tokens[1].length() - idx).c_str()));
		if (!this->_valid_ip(this->_tokens[1].substr(0, idx).c_str()))
			throw InvalidValues("ip", this->_tokens[1]);
		if (!this->_valid_port(this->_tokens[1].substr(idx, this->_tokens[1].length() - idx).c_str()))
			throw InvalidValues("port", this->_tokens[1]);
		return (std::make_pair(ip, port));
	}
	if (this->_valid_ip(this->_tokens[1]) && !this->_valid_port(this->_tokens[1]))
	{
		ip = inet_addr(this->_tokens[1].c_str());
		port = htons(80); //default
	}
	else if (!this->_valid_ip(this->_tokens[1]) && this->_valid_port(this->_tokens[1]))
	{
		ip = inet_addr("127.0.0.1"); //default
		port = htons(ft_atoi(this->_tokens[1].c_str()));
	}
	else
		throw InvalidValues("ip:port", this->_tokens[1]);
	return (std::make_pair(ip, port));
}

std::vector<std::string>	ParserHelper::get_server_name( void )
{
	if (this->_tokens.size() == 1)
		throw ParserHelper::InvalidNumberArgs(this->_tokens[0]);
	for (size_t i = 1; i < this->_tokens.size(); i++)
	{
		if (!this->_valid_server_name(this->_tokens[i]))
			throw ParserHelper::InvalidValues("server_name", this->_tokens[i]);
	}
	std::vector<std::string> args(this->_tokens.begin() + 1, this->_tokens.end());
	for (size_t i = 0; i < args.size(); i++)
		std::transform(args[i].begin(), args[i].end(), args[i].begin(), ft_tolower);
	return (args);
}

std::string	ParserHelper::get_root( void )
{
	if (this->_tokens.size() != 2)
		throw ParserHelper::InvalidNumberArgs(this->_tokens[0]);
	//
	return (std::string(this->_tokens[1]));
}

// std::vector<std::string>	ParserHelper::get_index( void );

// std::string	ParserHelper::get_error_page( void );

// size_t	ParserHelper::get_timeout( void );

// size_t	ParserHelper::get_client_max_body_size( void );

// std::string	ParserHelper::get_access_log( void );

// std::string	ParserHelper::get_error_log( void );

// bool	ParserHelper::get_autoindex( void );

// std::string	ParserHelper::get_cgi( void );

// std::pair<size_t, std::string>	ParserHelper::get_return( void );

// bool	ParserHelper::get_upload( void );

// std::string	ParserHelper::get_upload_store( void );

// std::vector<std::string>	ParserHelper::get_limit_except( void );

bool	ParserHelper::_valid_ip( std::string const &ip )
{
	size_t						begin;
	size_t						end;
	std::vector<std::string>	split;

	begin = 0;
	while (begin < ip.size())
	{
		end = ip.find(".", begin);
		if (end == std::string::npos)
			end = ip.size();
		split.push_back(ip.substr(begin, end - begin));
		begin = end;
	}
	if (split.size() != 4)
		return (false);
	for (size_t i = 0; i < split.size(); i++)
	{
		if (ft_atoi(split[i].c_str()) < 0 || ft_atoi(split[i].c_str()) > 255)
			return (false);
	}
	return (true);
}

bool	ParserHelper::_valid_port( std::string const &port )
{
	if (ft_atoi(port.c_str()) < 1 || ft_atoi(port.c_str()) > 65535)
		return (false);
	return (true);
}

bool	ParserHelper::_valid_server_name( std::string const &str )
{
	if (!ft_isalnum(str[0]) || !ft_isalnum(str[str.size() - 1]))
		return (false);
	for (size_t i = 1; i < str.size() - 1; i++)
	{
		if (!ft_isalnum(str[i]) && str[i] != '.' && str[i] != '-')
			return (false);
	}
	return (true);
}

ParserHelper::InvalidLine::InvalidLine( std::string const &str ) : ParserException(str)
{
	this->_msg = "Error: Invalid line in '" + str + "'!";
}

char const	*ParserHelper::InvalidLine::what( void ) const throw()
{
	return (this->_msg.c_str());
}

ParserHelper::DuplicatedDirectives::DuplicatedDirectives( std::string const &str ) : ParserException(str)
{
	this->_msg = "Error: Directive '" + str + "' is defined multiple times!";
}

char const	*ParserHelper::DuplicatedDirectives::what( void ) const throw()
{
	return (this->_msg.c_str());
}

ParserHelper::InvalidNumberArgs::InvalidNumberArgs( std::string const &str ) : ParserException(str)
{
	this->_msg = "Error: Invalid number of arguments in '" + str + "'!";
}

char const	*ParserHelper::InvalidNumberArgs::what( void ) const throw()
{
	return (this->_msg.c_str());
}

ParserHelper::InvalidValues::InvalidValues( std::string const &field,
											std::string const &value ) : ParserException("")
{
	this->_msg = "Error: Invalid value '" + value + "' for field '" + field + "'!";
}

char const	*ParserHelper::InvalidValues::what( void ) const throw()
{
	return (this->_msg.c_str());
}

ParserHelper::UnknownDirective::UnknownDirective( std::string const &str ) : ParserException(str)
{
	this->_msg = "Error: Directive '" + str + "' is unknown!";
}

char const	*ParserHelper::UnknownDirective::what( void ) const throw()
{
	return (this->_msg.c_str());
}
