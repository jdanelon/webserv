#include "HttpRequest.hpp"

HttpRequest::HttpRequest( void ) : autoindex(false), _error_code(0) {}

HttpRequest::HttpRequest( HttpRequest const &obj ) {
	*this = obj;
}

HttpRequest &HttpRequest::operator = ( HttpRequest const &obj ) {
	if (this != &obj) {
		this->method = obj.method;
		this->uri = obj.uri;
		this->version = obj.version;
		this->host = obj.host;
		this->headers = obj.headers;
		this->body = obj.body;
		this->raw = obj.raw;
		this->autoindex = obj.autoindex;
		this->_error_code = obj._error_code;
	}
	return (*this);
}

HttpRequest::~HttpRequest( void ) {}

void	HttpRequest::parse( std::string raw ) {
	this->raw = raw;
	std::string line;
	std::istringstream iss(raw);
	std::getline(iss, line);

	// Parsing the request line
	this->parse_request_line(line);

	// Parsing the headers
	while (std::getline(iss, line) && line != "\r") {
		this->parse_header_line(line);
	}

	// Define host as first part before ':' on value of 'Host' header, if present
	// Otherwise, set Bad Request error (removed from validate and brought here)
	std::map<std::string, std::string>::iterator host_header = this->headers.find("Host");
	if (host_header != this->headers.end())
	{
		size_t pos = host_header->second.find(':');
		this->host = host_header->second.substr(0, pos);
	}
	else
		this->set_error_code(400);
}

void	HttpRequest::parse_request_line( std::string line ) {
	size_t	first_space, second_space;

	first_space = line.find_first_of(" ");
	second_space = line.find_first_of(" ", first_space + 1);
	if (first_space == std::string::npos || second_space == std::string::npos ||
		second_space != line.find_last_of(" "))
	{
		this->set_error_code(400);
		return ;
	}
	this->method = line.substr(0, first_space);
	this->uri = line.substr(first_space + 1, second_space - first_space - 1);
	this->version = line.substr(second_space + 1, line.length() - second_space - 2);

	// Version is not (HTTP + / + DIGIT + . + DIGIT)? - ERROR CODE 400 (Bad Request)
	if (this->version.length() != 8 || this->version.compare(0, 5, "HTTP/") ||
		!isdigit(this->version[5]) || this->version[6] != '.' || !isdigit(this->version[7]))
	{
		this->set_error_code(400);
		return ;
	}
}

void	HttpRequest::parse_header_line( std::string line ) {
	std::istringstream	iss(line);
	std::string 		key, value;

	// Read the key (everything before the colon)
	std::getline(iss, key, ':');

	// Use the stream extractor to read the value, skipping leading whitespace
	iss >> std::ws; // Skip any leading whitespace
	std::getline(iss, value);

	if (value[value.length() - 1] != '\r')
	{
		this->set_error_code(400);
		return ;
	}
	// Remove '\r' at the end of value
	value = value.substr(0, value.length() - 1);

	if (key.empty() || value.empty()) {
		this->set_error_code(400);
		return ;
	}

	std::pair<std::map<std::string, std::string, CaseInsensitive>::iterator, bool>	ret;
	ret = this->headers.insert(std::make_pair(key, value));
	if (!ret.second) {
		this->set_error_code(400);
		return ;
	}
}

static std::string	get_matched_location( std::string request_uri, std::map<std::string, Location> locations )
{
	size_t										idx = request_uri.length();
	std::string									path(request_uri + std::string("/"));
	std::map<std::string, Location>::iterator	loc = locations.end();

	while (idx != std::string::npos)
	{
		idx = path.find_last_of("/", idx);
		if (idx == 0)
			idx = 1;
		path = path.substr(0, idx);
		loc = locations.find(path);
		if (idx == 1 || loc != locations.end())
			break ;
	}
	return (path);
}

static bool	is_method_forbidden( std::string method, std::vector<std::string> allowed )
{
	if (method != "HEAD" && method != "GET" && method != "POST" && method != "DELETE")
		return (false);

	std::vector<std::string>::iterator	it;
	for (it = allowed.begin(); it != allowed.end(); it++)
	{
		if (*it == method)
			return (false);
	}
	return (true);
}

static bool	is_server_name_forbidden( std::string hostname, std::string ip, std::vector<std::string> server_names )
{
	if ((hostname == "localhost" || hostname == "127.0.0.1") && ip == "127.0.0.1")
		return (false);
	for (size_t idx = 0; idx < server_names.size(); idx++)
	{
		if (hostname == server_names[idx])
			return (false);
	}
	return (true);
}

static std::string	find_final_root( std::string uri, std::string alias, std::string match )
{
	std::string	tmp, final_root;

	tmp = uri;
	if (match.length() == 1)
		tmp = alias + uri;
	else
		tmp.replace(0, match.length(), alias);
	final_root = tmp.substr(tmp.find_first_not_of("/"));
	return (final_root);
}

std::string	HttpRequest::validate( Server *srv ) {
	std::cout << "Validating request:" << std::endl;

	if (this->method.empty() || this->uri.empty() || this->version.empty())
		this->set_error_code(400);

	std::map<std::string, Location> locations = srv->location;
	std::map<std::string, Location>::iterator loc = locations.find(get_matched_location(this->uri, locations));
	if (loc != locations.end() && is_method_forbidden(this->method, loc->second.limit_except))
		this->set_error_code(405);

	if (this->method == "PUT" || this->method == "CONNECT" || this->method == "OPTIONS" ||
			this->method == "TRACE" || this->method == "PATCH")
		this->set_error_code(405);

	if (this->method != "HEAD" && this->method != "GET" && this->method != "POST" && this->method != "DELETE")
		this->set_error_code(501);

	if (this->uri.length() > 8000)
		this->set_error_code(414);

	// TO-DO: Validate return directive (redirect variable)
	std::string resource = this->uri.substr(this->uri.find_last_of("/") + 1);
	std::string	final_root, new_uri, full_path;
	struct stat	buf;
	// If request is for file: set root from redirection, full_path and set 404 if not found
	if (resource.length() != 0)
	{
		if (loc != locations.end() && !loc->second.alias.empty())
			final_root = find_final_root(this->uri, loc->second.alias, loc->first);
		else
			final_root = srv->root + this->uri;
		full_path = std::getenv("PWD") + std::string("/") + final_root;
		if ((stat(full_path.c_str(), &buf) == -1) || !(buf.st_mode & S_IXUSR))
			this->set_error_code(404);
	}
	// If request is for folder: search for index files with possible multiple redirections
	else
	{
		size_t	i = 0;
		std::vector<std::string> index_files = (loc != locations.end()) ? loc->second.index : srv->index;
		for (; i < index_files.size(); i++)
		{
			new_uri = this->uri;
			new_uri += (index_files[i][0] == '/') ? index_files[i].substr(1) : index_files[i];
			loc = locations.find(get_matched_location(new_uri, locations));
			if (loc != locations.end() && !loc->second.alias.empty())
				final_root = find_final_root(new_uri, loc->second.alias, loc->first);
			else
				final_root = srv->root + new_uri;
			resource = new_uri.substr(new_uri.find_last_of("/") + 1);
			full_path = std::getenv("PWD") + std::string("/") + final_root;
			// Check if index file is found
			if ((stat(full_path.c_str(), &buf) != -1) && (buf.st_mode & S_IXUSR))
				break ;
		}
		// No index files are found
		if (i == index_files.size())
		{
			// if autoindex is true, does not set 404
			bool parsed_autoindex = (loc != locations.end()) ? loc->second.autoindex : srv->autoindex;
			if (parsed_autoindex == true)
			{
				full_path = full_path.substr(0, full_path.find_last_of("/"));
				this->autoindex = true;
			}
			else
				this->set_error_code(404);
		}
	}

	if (this->version != "HTTP/1.1")
		this->set_error_code(505);

	// ATTENTION: NEED TO CHANGE /ETC/HOSTS FILE TO INCLUDE OTHER SERVER_NAMES
	if (is_server_name_forbidden(this->host, srv->ip, srv->server_name))
		this->set_error_code(404);

	int	code = this->get_error_code();
	std::map<int, std::string>::iterator it;
	for (it = srv->error_page.begin(); it != srv->error_page.end(); it++)
	{
		if (it->first == code)
		{
			full_path = std::getenv("PWD") + std::string("/") + srv->root + it->second;
			break ;
		}
	}

	return (full_path);
}

int	HttpRequest::get_error_code( void ) const
{
	return (this->_error_code);
}

void	HttpRequest::set_error_code( int const &code )
{
	if (this->_error_code == 0)
		this->_error_code = code;
}

// Debug
void	HttpRequest::print( int client_fd ) {
	// Print a header to see where the request starts
	std::cout << "------------------" << std::endl;
	std::cout << "Client fd: " << client_fd << std::endl;
	std::cout << "Parsed request:" << std::endl;
	std::cout << "method: '" << this->method << "'" << std::endl;
	std::cout << "uri: '" << this->uri << "'" << std::endl;
	std::cout << "version: '" << this->version << "'" << std::endl;
	std::cout << "host: " << this->host << std::endl;
	std::cout << "headers: " << std::endl;
	std::map<std::string, std::string, CaseInsensitive>::iterator it;
	for (it = this->headers.begin(); it != this->headers.end(); ++it) {
		std::cout << "\t" << it->first << ": " << it->second << std::endl;
	}
	std::cout << "body: " << this->body << std::endl;
	std::cout << "error_code: " << this->_error_code << std::endl;
	std::cout << "------------------" << std::endl;
}
