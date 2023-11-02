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
		this->_directive_list = obj._directive_list;
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
	if (tokens[0] == "server")
		return (false);
	if (tokens[0] == "error_page" || tokens[0] == "cgi"
			|| tokens[0] == "return" || tokens[0] == "location")
		ret = this->_directive_list.insert(tokens[0] + tokens[1]);
	else
		ret = this->_directive_list.insert(tokens[0]);
	if (ret.second == false)
		return (true);
	return (false);
}

int	ParserHelper::get_backlog( void )
{
	if (this->_tokens.size() != 2)
		throw ParserHelper::InvalidNumberArgs(this->_tokens[0]);
	if (ft_atoi(this->_tokens[1].c_str()) < 1 || ft_atoi(this->_tokens[1].c_str()) > 1024)
		throw ParserHelper::InvalidValues("worker_connections", this->_tokens[1]);
	return (ft_atoi(this->_tokens[1].c_str()));
}

std::pair<std::string, std::string>	ParserHelper::get_listen( void )
{
	std::string	host, port;

	if (this->_tokens.size() != 2)
		throw InvalidNumberArgs(this->_tokens[0]);
	size_t idx = this->_tokens[1].find(":");
	if (idx != std::string::npos)
	{
		host = this->_tokens[1].substr(0, idx);
		port = this->_tokens[1].substr(idx + 1, this->_tokens[1].length() - idx);
		if (!this->_valid_ip(this->_tokens[1].substr(0, idx)))
			throw InvalidValues("host", this->_tokens[1]);
		if (!this->_valid_port(this->_tokens[1].substr(idx + 1, this->_tokens[1].length() - idx)))
			throw InvalidValues("port", this->_tokens[1]);
		return (std::make_pair(host, port));
	}
	if (this->_valid_ip(this->_tokens[1]) && !this->_valid_port(this->_tokens[1]))
	{
		host = this->_tokens[1];
		port = "8080"; //default
	}
	else if (!this->_valid_ip(this->_tokens[1]) && this->_valid_port(this->_tokens[1]))
	{
		host = "127.0.0.1"; //default
		port = this->_tokens[1];
	}
	else
		throw InvalidValues("host:port", this->_tokens[1]);
	return (std::make_pair(host, port));
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
	struct stat buf;
	if (stat(this->_tokens[1].c_str(), &buf) == -1 || !S_ISDIR(buf.st_mode | S_IRUSR))
		throw ParserHelper::SystemError("root", this->_tokens[1]);
	return (this->_tokens[1]);
}

std::vector<std::string>	ParserHelper::get_index( void )
{
	if (this->_tokens.size() < 2)
		throw ParserHelper::InvalidNumberArgs(this->_tokens[0]);
	for (size_t i = 1; i < this->_tokens.size(); i++)
	{
		size_t idx = this->_tokens[i].find(".");
		if (idx == std::string::npos)
			throw ParserHelper::InvalidValues("index", this->_tokens[i]);
		idx = this->_tokens[i].find(".", idx + 1);
		if (idx != std::string::npos)
			throw ParserHelper::InvalidValues("index", this->_tokens[i]);
	}
	std::vector<std::string> args(this->_tokens.begin() + 1, this->_tokens.end());
	return (args);
}

std::string	ParserHelper::get_error_page( void )
{
	if (this->_tokens.size() != 3)
		throw ParserHelper::InvalidNumberArgs(this->_tokens[0]);
	if (ft_atoi(this->_tokens[1].c_str()) < 400 || ft_atoi(this->_tokens[1].c_str()) > 499)
		throw ParserHelper::InvalidValues("error_page", this->_tokens[1]);
	if (!this->_valid_error_page(this->_tokens[2]))
		throw ParserHelper::InvalidValues("error_page", this->_tokens[2]);

	size_t idx = this->_tokens[2].find_last_of("/");
	if (idx == std::string::npos)
		throw ParserHelper::InvalidValues("error_page", this->_tokens[2]);
	std::string cleaned = this->_tokens[2].substr(idx, this->_tokens[2].size() - idx);
	return (cleaned);
}

size_t	ParserHelper::get_timeout( void )
{
	if (this->_tokens.size() != 2)
		throw ParserHelper::InvalidNumberArgs(this->_tokens[0]);
	if (ft_atoi(this->_tokens[1].c_str()) < 1 || ft_atoi(this->_tokens[1].c_str()) > 3600)
		throw ParserHelper::InvalidValues("timeout", this->_tokens[1]);
	return (ft_atoi(this->_tokens[1].c_str()) * 1000);
}

int	ParserHelper::get_client_max_body_size( void )
{
	if (this->_tokens.size() != 2)
		throw ParserHelper::InvalidNumberArgs(this->_tokens[0]);
	char last_char = this->_tokens[1][this->_tokens[1].size() - 1];
	if (last_char == 'm' || last_char == 'M')
	{
		std::string substr = this->_tokens[1].substr(0, this->_tokens.size() - 1);
		if (ft_atoi(substr.c_str()) <= 0 || ft_atoi(substr.c_str()) > 1024)
			throw ParserHelper::InvalidValues("client_max_body_size", this->_tokens[1]);
		return (ft_atoi(substr.c_str()) * 1000000);
	}
	else if (last_char == 'k' || last_char == 'K')
	{
		std::string substr = this->_tokens[1].substr(0, this->_tokens.size() - 1);
		if (ft_atoi(substr.c_str()) <= 0 || ft_atoi(substr.c_str()) > 1024000)
			throw ParserHelper::InvalidValues("client_max_body_size", this->_tokens[1]);
		return (ft_atoi(substr.c_str()) * 1000);
	}
	else
	{
		if (ft_atoi(this->_tokens[1].c_str()) < 0 || ft_atoi(this->_tokens[1].c_str()) > 1024000000)
			throw ParserHelper::InvalidValues("client_max_body_size", this->_tokens[1]);
		return (ft_atoi(this->_tokens[1].c_str()));
	}
}

bool	ParserHelper::get_autoindex( void )
{
	if (this->_tokens.size() != 2)
		throw ParserHelper::InvalidNumberArgs(this->_tokens[0]);
	if (this->_tokens[1] == "on")
		return (true);
	else if (this->_tokens[1] == "off")
		return (false);
	else
		throw ParserHelper::InvalidValues("autoindex", this->_tokens[1]);
}

std::string	ParserHelper::get_cgi( void )
{
	if (this->_tokens.size() != 3)
		throw ParserHelper::InvalidNumberArgs(this->_tokens[0]);
	if (!this->_valid_cgi_extension(this->_tokens[1]))
		throw ParserHelper::InvalidValues("cgi", this->_tokens[1]);

	std::string cgi_binary = this->_valid_cgi_binary(this->_tokens[2]);
	if (cgi_binary.empty())
		throw ParserHelper::SystemError("cgi", this->_tokens[2]);
	return (cgi_binary);
}

static bool	is_code_valid( int code )
{
	int	arr[] = { 301, 302, 303, 307, 308 };
	int	n = sizeof(arr) / sizeof(arr[0]);
	std::vector<int>	valid_codes(arr, arr + n);

	if (std::find(valid_codes.begin(), valid_codes.end(), code) != valid_codes.end())
		return (true);
	return (false);
}

std::pair<size_t, std::string>	ParserHelper::get_return( void )
{
	if (this->_tokens.size() != 3)
		throw ParserHelper::InvalidNumberArgs(this->_tokens[0]);
	if (!is_code_valid(ft_atoi(this->_tokens[1].c_str())))
		throw ParserHelper::InvalidValues("return", this->_tokens[1]);
	return (std::make_pair(ft_atoi(this->_tokens[1].c_str()), this->_tokens[2]));
}

bool	ParserHelper::get_upload( void )
{
	if (this->_tokens.size() != 2)
		throw ParserHelper::InvalidNumberArgs(this->_tokens[0]);
	if (this->_tokens[1] == "on")
		return (true);
	else if (this->_tokens[1] == "off")
		return (false);
	else
		throw ParserHelper::InvalidValues("upload", this->_tokens[1]);
}

std::string	ParserHelper::get_upload_store( void )
{
	if (this->_tokens.size() != 2)
		throw ParserHelper::InvalidNumberArgs(this->_tokens[0]);
	// struct stat buf;
	// if (stat(this->_tokens[1].c_str(), &buf) == -1 || !S_ISDIR(buf.st_mode | S_IRUSR))
	// 	throw ParserHelper::SystemError("upload_store", this->_tokens[1]);
	return (this->_tokens[1]);
}

std::vector<std::string>	ParserHelper::get_limit_except( void )
{
	if (this->_tokens.size() == 1)
		throw ParserHelper::InvalidNumberArgs(this->_tokens[0]);

	std::vector<std::string> args(this->_tokens.begin() + 1, this->_tokens.end());

	for (size_t i = 0; i < args.size(); i++)
		std::transform(args[i].begin(), args[i].end(), args[i].begin(), ft_toupper);

	std::vector<std::string> cmp;

	cmp.push_back("GET");
	cmp.push_back("POST");
	cmp.push_back("DELETE");

	for (size_t i = 0; i < args.size(); i++)
	{
		for (size_t j = 0; j < cmp.size(); j++)
		{
			if (args[i] == cmp[j])
				break ;
			if (j == cmp.size())
				throw ParserHelper::InvalidValues("limit_except", args[i]);
		}
	}
	return (args);
}

std::string	ParserHelper::get_alias( void )
{
	if (this->_tokens.size() != 2)
		throw ParserHelper::InvalidNumberArgs(this->_tokens[0]);
	struct stat buf;
	if (stat(this->_tokens[1].c_str(), &buf) == -1 || !S_ISDIR(buf.st_mode | S_IRUSR))
		throw ParserHelper::SystemError("alias", this->_tokens[1]);
	return (this->_tokens[1]);
}

bool	ParserHelper::_valid_ip( std::string const &host )
{
	size_t						begin;
	size_t						end;
	std::vector<std::string>	split;

	begin = 0;
	while (begin < host.size())
	{
		end = host.find(".", begin);
		if (end == std::string::npos)
			end = host.size();
		split.push_back(host.substr(begin, end - begin));
		begin = end + 1;
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

bool	ParserHelper::_valid_error_page( std::string const &str )
{
	if ((!ft_isalnum(str[0]) && str[0] != '/') || !ft_isalnum(str[str.size() - 1]))
		return (false);
	for (size_t i = 1; i < str.size() - 1; i++)
	{
		if (!ft_isalnum(str[i]) && str[i] != '/' && str[i] != '.' && str[i] != '-' && str[i] != '_')
			return (false);
	}
	if (str.find("/") == std::string::npos || str.find(".") == std::string::npos)
		return (false);
	return (true);
}

bool	ParserHelper::_valid_log( std::string const &log )
{
	if ((!ft_isalnum(log[0]) && log[0] != '/') || !ft_isalnum(log[log.size() - 1]))
		return (false);
	for (size_t i = 1; i < log.size() - 1; i++)
	{
		if (!ft_isalnum(log[i]) && log[i] != '/' && log[i] != '.' && log[i] != '-' && log[i] != '_')
			return (false);
	}
	return (true);
}

bool	ParserHelper::_valid_cgi_extension( std::string const &ext )
{
	if (ext[0] != '.')
		return (false);
	for (size_t i = 1; i < ext.size(); i++)
	{
		if (!ft_isalnum(ext[i]) || (ext[i] >= '0' && ext[i] <= '9'))
			return (false);
	}
	return (true);
}

std::string	ParserHelper::_valid_cgi_binary( std::string const &bin )
{
	std::vector<std::string>	bin_paths;
	struct stat 				buf;

	// set a vector with possible file paths: bin and each locale in PATH + bin
	bin_paths = ParserHelper::_get_path_vector(bin);
	// loop over paths searching for bin
	for (size_t i = 0; i < bin_paths.size(); i++)
	{
		// if file is found and it is executable -> valid binary
		if ((stat(bin_paths[i].c_str(), &buf) == 0) && (buf.st_mode & S_IXUSR))
			return (bin_paths[i]);
		// if file is found and it is not executable -> invalid binary
		if ((stat(bin_paths[i].c_str(), &buf) == 0) && !(buf.st_mode & S_IXUSR))
		{
			errno = EACCES;
			return ("");
		}
	}
	// if loop is over (file not found) -> invalid binary
	return ("");
}

std::vector<std::string>	ParserHelper::_get_path_vector( std::string const &bin )
{
	std::string					path;
	size_t						begin_idx, end_idx;
	std::vector<std::string>	separate_path;

	separate_path.push_back(bin);
	path = std::getenv("PATH");
	begin_idx = 0;
	end_idx = path.find(':');
	while (end_idx != std::string::npos)
	{
		separate_path.push_back(path.substr(begin_idx, end_idx - begin_idx) + "/" + bin);
		begin_idx = end_idx + 1;
		end_idx = path.find(':', begin_idx);
	}
	separate_path.push_back(path.substr(begin_idx, path.size() - begin_idx) + "/" + bin);
	return (separate_path);
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

ParserHelper::SystemError::SystemError( std::string const &field,
										std::string const &value ) : ParserException("")
{
	this->_msg = "Error: Invalid value '" + value + "' for field '" + field + "' (" + strerror(errno) + ")!";
}

char const	*ParserHelper::SystemError::what( void ) const throw()
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

ParserHelper::MissingDirectives::MissingDirectives( std::string const &str ) : ParserException(str)
{
	this->_msg = "Error: Directive '" + str + "' was not specified!";
}

char const	*ParserHelper::MissingDirectives::what( void ) const throw()
{
	return (this->_msg.c_str());
}

char const	*ParserHelper::EmptyLocationBlock::what( void ) const throw()
{
	return ("Error: Location block cannot be empty!");
}
