#include "HttpRequest.hpp"

bool HttpRequest::debugEnabled = true;
const std::string HttpRequest::className = "HttpRequest";

HttpRequest::HttpRequest( void ) : autoindex(false), query_string(""), is_valid(true), _error_code(0) {
	// Create a new body parser
	std::cout << "Creating new HttpRequest" << std::endl;
	this->body_parser = HttpRequestBody();
}

HttpRequest::HttpRequest( HttpRequest const &obj ) {
	(void) obj;
	// *this = obj;
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
		this->resource = obj.resource;
		this->query_string = obj.query_string;
		this->is_valid = obj.is_valid;
		this->_error_code = obj._error_code;
		this->has_body = obj.has_body;
		this->is_body_parsed = obj.is_body_parsed;
		this->body_parser = obj.body_parser;
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
	else {
		set_error_code(400);
	}

    // Check if the body is chunked
    std::map<std::string, std::string>::iterator transfer_encoding_header = this->headers.find("Transfer-Encoding");
    if (transfer_encoding_header != this->headers.end() && transfer_encoding_header->second == "chunked") {
        this->has_body = true;
    }

	// Check if the body is a file upload
	std::map<std::string, std::string>::iterator content_type_header = this->headers.find("Content-Type");
	if (content_type_header != this->headers.end() && content_type_header->second.find("multipart/form-data") != std::string::npos) {
		this->has_body = true;
	}

    // Check if there is a body and it's not a GET request
    if (this->method != "GET") {
        std::map<std::string, std::string>::iterator content_length_header = this->headers.find("Content-Length");
        if (content_length_header != this->headers.end()) {
            this->has_body = true;
        }
    }
}

void	HttpRequest::parse_request_line( std::string line ) {
	size_t	first_space, second_space;

	first_space = line.find_first_of(" ");
	second_space = line.find_first_of(" ", first_space + 1);
	if (first_space == std::string::npos || second_space == std::string::npos ||
		second_space != line.find_last_of(" "))
	{
		set_error_code(400);
		return ;
	}
	this->method = line.substr(0, first_space);
	this->uri = line.substr(first_space + 1, second_space - first_space - 1);
	this->version = line.substr(second_space + 1, line.length() - second_space - 2);

	// Version is not (HTTP + / + DIGIT + . + DIGIT)? - ERROR CODE 400 (Bad Request)
	if (this->version.length() != 8 || this->version.compare(0, 5, "HTTP/") ||
		!isdigit(this->version[5]) || this->version[6] != '.' || !isdigit(this->version[7]))
	{
		set_error_code(400);
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
		set_error_code(400);
		return ;
	}
	// Remove '\r' at the end of value
	value = value.substr(0, value.length() - 1);

	if (key.empty() || value.empty()) {
		set_error_code(400);
		return ;
	}

	std::pair<std::map<std::string, std::string, CaseInsensitive>::iterator, bool>	ret;
	ret = this->headers.insert(std::make_pair(key, value));
	if (!ret.second) {
		set_error_code(400);
		return ;
	}

	// If header is multipart/form-data, and boundary is not present, set Bad Request error
	if (key == "Content-Type" && value.find("multipart/form-data") != std::string::npos)
	{
		size_t pos = value.find("boundary=");
		if (pos == std::string::npos)
		{
			set_error_code(400);
			return ;
		}
		// set boundary string as value of a new header 'boundary'
		std::string boundary = value.substr(pos + 9);
		this->headers.insert(std::make_pair("boundary", boundary));
		// set the value of Content-Type header to 'multipart/form-data'
		this->headers["Content-Type"] = "multipart/form-data";
		this->body_parser.setBoundary(boundary);
	}
}

void	HttpRequest::parse_body( std::string partial_body ) {
	debug(INFO, "Parsing body");
	// We parse differently depending on whether the body is chunked or not
	if (this->headers.find("Transfer-Encoding") != this->headers.end() &&
		this->headers["Transfer-Encoding"] == "chunked")
	{
		// If the body is chunked, we need to parse the chunks
		this->body_parser.parseChunkedBody(partial_body);
	}
	// Check if the body is a file upload
	else if (this->headers.find("Content-Type") != this->headers.end() &&
			this->headers["Content-Type"].find("multipart/form-data") != std::string::npos)
	{
		// If the body is a file upload, we need to parse the multipart body
		this->body_parser.parseMultipartBody(partial_body);
	}
	else {
		// If the body is not chunked, we just append the partial body to the body
		// We need to check if we finish reading the body, so we check the Content-Length header
		std::map<std::string, std::string>::iterator content_length_header = this->headers.find("Content-Length");
		if (content_length_header == this->headers.end())
		{
			set_error_code(400);
			this->is_body_parsed = true;
			return ;
		}
		int content_length = atoi(content_length_header->second.c_str());
		if ((int)this->body.length() + (int)partial_body.length() > content_length)
		{
			set_error_code(400);
			this->is_body_parsed = true;
			return ;
		}
		this->body.append(partial_body);
		// We set the body as parsed if we have read the whole body
		if ((int)this->body.length() == content_length)
			this->is_body_parsed = true;
		else {
			this->is_body_parsed = false;
		}
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
		set_error_code(400);

	std::map<std::string, Location> locations = srv->location;
	std::map<std::string, Location>::iterator loc = locations.find(get_matched_location(this->uri, locations));
	if (loc != locations.end() && is_method_forbidden(this->method, loc->second.limit_except))
		set_error_code(405);

	if (this->method == "PUT" || this->method == "CONNECT" || this->method == "OPTIONS" ||
			this->method == "TRACE" || this->method == "PATCH")
		set_error_code(405);

	if (this->method != "HEAD" && this->method != "GET" && this->method != "POST" && this->method != "DELETE")
		set_error_code(501);

	if (this->uri.length() > 8000)
		set_error_code(414);

	// Return directive on server block
	if (srv->redirect.first != 0 && !srv->redirect.second.empty())
	{
		set_error_code(srv->redirect.first);
		if (srv->redirect.first >= 300 && srv->redirect.first < 400)
			this->headers.insert(std::make_pair("Location", "/" + srv->redirect.second));
		return ("");
	}

	size_t	query_idx = this->uri.find("?");
	size_t	resource_idx = this->uri.find_last_of("/") + 1;
	std::string	resource;
	if (query_idx != std::string::npos) {
		this->query_string = this->uri.substr(query_idx);
		resource = this->uri.substr(resource_idx, query_idx - resource_idx);
	}
	else
		resource = this->uri.substr(resource_idx);
	std::string	final_root, new_uri, full_path;
	struct stat	buf;
	// If request is for file: set root from redirection, full_path and set 404 if not found
	if (resource.length() != 0)
	{
		new_uri = this->uri.substr(0, query_idx);
		// Return directive on matched location for file request
		if (loc != locations.end() && loc->second.redirect.first != 0 && !loc->second.redirect.second.empty())
		{
			set_error_code(loc->second.redirect.first);
			if (loc->second.redirect.first >= 300 && loc->second.redirect.first < 400)
				this->headers.insert(std::make_pair("Location", "/" + loc->second.redirect.second));
			return ("");
		}
		if (loc != locations.end() && !loc->second.alias.empty())
			final_root = find_final_root(new_uri, loc->second.alias, loc->first);
		else
			final_root = srv->root + new_uri;
		full_path = std::getenv("PWD") + std::string("/") + final_root;
		if ((stat(full_path.c_str(), &buf) == -1) || !S_ISREG(buf.st_mode))
			set_error_code(404);
	}
	// If request is for folder: search for index files with possible multiple redirections
	else
	{
		size_t	i = 0;
		std::vector<std::string> index_files = (loc != locations.end()) ? loc->second.index : srv->index;
		for (; i < index_files.size(); i++)
		{
			new_uri = this->uri.substr(0, query_idx);
			new_uri += (index_files[i][0] == '/') ? index_files[i].substr(1) : index_files[i];
			loc = locations.find(get_matched_location(new_uri, locations));
			// Return directive on matched location for folder request
			if (loc != locations.end() && loc->second.redirect.first != 0 && !loc->second.redirect.second.empty())
			{
				set_error_code(loc->second.redirect.first);
				if (loc->second.redirect.first >= 300 && loc->second.redirect.first < 400)
					this->headers.insert(std::make_pair("Location", "/" + loc->second.redirect.second));
				return ("");
			}
			if (loc != locations.end() && !loc->second.alias.empty())
				final_root = find_final_root(new_uri, loc->second.alias, loc->first);
			else
				final_root = srv->root + new_uri;
			resource = new_uri.substr(new_uri.find_last_of("/") + 1);
			full_path = std::getenv("PWD") + std::string("/") + final_root;
			// Check if index file is found
			if ((stat(full_path.c_str(), &buf) != -1) && !S_ISREG(buf.st_mode))
				break ;
		}
		// No index files are found
		if (i == index_files.size())
		{
			// if autoindex is true, does not set 403
			bool parsed_autoindex = (loc != locations.end()) ? loc->second.autoindex : srv->autoindex;
			if (parsed_autoindex == true)
			{
				full_path = full_path.substr(0, full_path.find_last_of("/") + 1);
				struct stat buf;
				if (stat(full_path.c_str(), &buf) != 0 || !S_ISDIR(buf.st_mode))
				{
					set_error_code(404);
					return (full_path);
				}
				this->autoindex = true;
			}
			else
				set_error_code(403);
		}
	}

	if (this->version != "HTTP/1.1")
		set_error_code(505);

	// ATTENTION: NEED TO CHANGE /ETC/HOSTS FILE TO INCLUDE OTHER SERVER_NAMES
	if (is_server_name_forbidden(this->host, srv->ip, srv->server_name))
		set_error_code(404);

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

	size_t	headers_end = this->raw.find("\r\n\r\n") + 4;
	this->body = this->raw.length() != headers_end ? this->raw.substr(headers_end) : "";
	if ((srv->client_max_body_size != -1 && (int)this->body.length() > srv->client_max_body_size) ||
			(loc != locations.end() && loc->second.client_max_body_size != -1 &&
			(int)this->body.length() > loc->second.client_max_body_size))
		set_error_code(413);

	return (full_path);
}

int	HttpRequest::get_error_code( void ) const
{
	return (this->_error_code);
}

void	HttpRequest::set_error_code( int const &code )
{
	this->is_valid = false;
	if (this->_error_code == 0) {
		this->_error_code = code;
	}
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

void HttpRequest::debug(LogLevel level, const std::string& message) {
    if (!debugEnabled) {
        return;
    }

    std::string prefix;
    std::string colorCode;
    switch (level) {
    case INFO:
        prefix = "[INFO] ";
        colorCode = "\033[1;34m";  // Blue
        break;
    case WARNING:
        prefix = "[WARNING] ";
        colorCode = "\033[1;33m";  // Yellow
        break;
    case ERROR:
        prefix = "[ERROR] ";
        colorCode = "\033[1;31m";  // Red
        break;
    }

    std::cout << colorCode << className << " " << prefix << message << "\033[0m" << std::endl;  // \033[0m resets the color
}