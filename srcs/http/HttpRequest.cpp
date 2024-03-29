#include "HttpRequest.hpp"

bool HttpRequest::debugEnabled = true;
const std::string HttpRequest::className = "HttpRequest";

HttpRequest::HttpRequest( void ) : autoindex(false), path_info(""), query_string(""), is_valid(true), _error_code(0) {
	// Create a new body parser
	this->full_resource_path = "";
	this->full_upload_path = "";
	this->has_body = false;
	this->is_body_parsed = false;
	this->body_parser = HttpRequestBody();
	this->full_upload_path = "";
}

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
		this->full_resource_path = obj.full_resource_path;
		this->full_upload_path = obj.full_upload_path;
		this->path_info = obj.path_info;
		this->query_string = obj.query_string;
		this->is_valid = obj.is_valid;
		this->_error_code = obj._error_code;
		this->has_body = obj.has_body;
		this->is_body_parsed = obj.is_body_parsed;
		this->body_parser = obj.body_parser;
		this->full_upload_path = obj.full_upload_path;
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

	// Check if there is a body and it's not a GET or DELETE request
	if (this->method != "GET" && this->method != "DELETE") {
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
		set_error_code(httpStatusCodes.BadRequest.code);
		return ;
	}
	this->method = line.substr(0, first_space);
	this->uri = line.substr(first_space + 1, second_space - first_space - 1);
	this->version = line.substr(second_space + 1, line.length() - second_space - 2);

	// Version is not (HTTP + / + DIGIT + . + DIGIT)? - ERROR CODE 400 (Bad Request)
	if (this->version.length() != 8 || this->version.compare(0, 5, "HTTP/") ||
		!isdigit(this->version[5]) || this->version[6] != '.' || !isdigit(this->version[7]))
	{
		set_error_code(httpStatusCodes.BadRequest.code);
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
		set_error_code(httpStatusCodes.BadRequest.code);
		return ;
	}
	// Remove '\r' at the end of value
	value = value.substr(0, value.length() - 1);

	if (key.empty() || value.empty()) {
		set_error_code(httpStatusCodes.BadRequest.code);
		return ;
	}

	std::pair<std::map<std::string, std::string, CaseInsensitive>::iterator, bool>	ret;
	ret = this->headers.insert(std::make_pair(key, value));
	if (!ret.second) {
		set_error_code(httpStatusCodes.BadRequest.code);
		return ;
	}

	// If header is multipart/form-data, and boundary is not present, set Bad Request error
	if (key == "Content-Type" && value.find("multipart/form-data") != std::string::npos)
	{
		size_t pos = value.find("boundary=");
		if (pos == std::string::npos)
		{
			set_error_code(httpStatusCodes.BadRequest.code);
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

void	HttpRequest::parse_body( std::string partial_body ) {
	// We parse differently depending on whether the body is chunked or not
	if (this->headers.find("Transfer-Encoding") != this->headers.end() &&
		this->headers["Transfer-Encoding"] == "chunked")
	{
		// If the body is chunked, we need to parse the chunks
		this->body_parser.parseChunkedBody(partial_body);

		if (this->body_parser.getState() == COMPLETE) {
			this->is_body_parsed = true;
			this->body = this->body_parser.getFullChunkedBody();
			debug(INFO, "Full body after Chunked Parsing: " + this->body);
		}
	}
	// Check if the body is a file upload
	else if (this->headers.find("Content-Type") != this->headers.end() &&
			this->headers["Content-Type"].find("multipart/form-data") != std::string::npos)
	{
		if (this->full_upload_path.empty()) {
			debug(ERROR, "Full upload path is empty");
			set_error_code(httpStatusCodes.BadRequest.code);
		}
		// If the body is a file upload, we need to parse the multipart body
		this->body_parser.setUploadStore(this->full_upload_path);
		this->body_parser.parseMultipartBody(partial_body);
	}
	else {
		// If the body is not chunked, we just append the partial body to the body
		// We need to check if we finish reading the body, so we check the Content-Length header
		debug(INFO, "Default Body Parsing: " + partial_body);
		std::map<std::string, std::string>::iterator content_length_header = this->headers.find("Content-Length");
		if (content_length_header == this->headers.end())
		{
			set_error_code(httpStatusCodes.BadRequest.code);
			this->is_body_parsed = true;
			return ;
		}
		int content_length = atoi(content_length_header->second.c_str());
		if ((int)this->body.length() + (int)partial_body.length() > content_length)
		{
			set_error_code(httpStatusCodes.BadRequest.code);
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

	if (this->body_parser.getState() == COMPLETE) {
		this->is_body_parsed = true;
		
		if (this->body_parser.getIsError()) {
			set_error_code(httpStatusCodes.InternalServerError.code);
		}
	}
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

static std::string	remove_double_slash( std::string const &path )
{
	size_t		idx;
	std::string	tmp(path);

	idx = path.find("//");
	while (idx != std::string::npos)
	{
		tmp.replace(idx, 2, "/");
		idx = tmp.find("//");
	}
	return (tmp);
}

std::string	HttpRequest::_find_full_path( std::string method, std::string new_uri, Server *srv )
{
	int			upload(srv->upload);
	std::string	alias(""), upload_store(srv->upload_store), full_path;
	std::map<std::string, Location> locations = srv->location;
	std::map<std::string, Location>::iterator loc = locations.find(get_matched_location(new_uri, locations));

	full_path = std::getenv("PWD") + std::string("/") + srv->root + std::string("/");
	if (loc != locations.end())
	{
		if (new_uri == loc->first)
			new_uri += "/";
		alias = loc->second.alias;
		upload = loc->second.upload;
		upload_store = loc->second.upload_store;
		if (!alias.empty())
			new_uri = new_uri.substr(new_uri.find(loc->first) + loc->first.length());
	}
	std::string	remaining_folders = new_uri.substr(0, new_uri.find_last_of('/') + 1);
	std::string	resource = new_uri.substr(new_uri.find_last_of('/') + 1);

	full_path += alias;
	full_path += remaining_folders;
	if (method == "POST" && upload)
	{
		this->full_upload_path = remove_double_slash(full_path + upload_store);
		full_path += std::string("/") + upload_store + std::string("/");
	}

	struct stat buf;
	if (stat(full_path.c_str(), &buf) != 0 || !S_ISDIR(buf.st_mode))
		return ("");
	full_path += resource;
	return (remove_double_slash(full_path));
}

void	HttpRequest::validate_headers( Server *srv ) {
	std::cout << "Validating request:" << std::endl;

	if (this->method.empty() || this->uri.empty() || this->version.empty())
		set_error_code(httpStatusCodes.BadRequest.code);

	std::map<std::string, Location> locations = srv->location;
	std::map<std::string, Location>::iterator loc = locations.find(get_matched_location(this->uri, locations));
	if (loc != locations.end() && is_method_forbidden(this->method, loc->second.limit_except))
		set_error_code(httpStatusCodes.MethodNotAllowed.code);

	if (this->method == "HEAD" || this->method == "PUT" || this->method == "CONNECT" ||
			this->method == "OPTIONS" || this->method == "TRACE" || this->method == "PATCH")
		set_error_code(httpStatusCodes.MethodNotAllowed.code);

	if (this->method != "GET" && this->method != "POST" && this->method != "DELETE")
		set_error_code(httpStatusCodes.NotImplemented.code);

	if (this->uri.length() > 8000)
		set_error_code(httpStatusCodes.URITooLong.code);

	if (this->version != "HTTP/1.1")
		set_error_code(httpStatusCodes.HTTPVersionNotSupported.code);

	std::map<std::string, std::string>::iterator content_length_header = this->headers.find("Content-Length");
	if (content_length_header != this->headers.end() && ft_atoi(content_length_header->second.c_str()) == -1)
		set_error_code(httpStatusCodes.LengthRequired.code);
	else if (this->method == "POST" && !this->has_body)
		set_error_code(httpStatusCodes.LengthRequired.code);

	// Return directive on server block
	if (srv->redirect.first != 0 && !srv->redirect.second.empty())
	{
		set_error_code(srv->redirect.first);
		this->headers.insert(std::make_pair("Location", srv->redirect.second));
		return ;
	}

	size_t	query_idx = this->uri.find("?");
	size_t	resource_idx = this->uri.find_last_of("/", this->uri.find(".")) + 1;
	size_t	path_info_idx = this->uri.find("/", this->uri.find("."));
	std::string	resource;
	if (query_idx != std::string::npos) {
		this->query_string = this->uri.substr(query_idx);
		if (path_info_idx != std::string::npos)
		{
			this->path_info = this->uri.substr(path_info_idx, query_idx - path_info_idx);
			resource = this->uri.substr(resource_idx, path_info_idx - resource_idx);
		}
		else
		{
			resource = this->uri.substr(resource_idx, query_idx - resource_idx);
			path_info_idx = query_idx;
		}
	}
	else
	{
		if (path_info_idx != std::string::npos)
		{
			this->path_info = this->uri.substr(path_info_idx);
			resource = this->uri.substr(resource_idx, path_info_idx - resource_idx);
		}
		else
			resource = this->uri.substr(resource_idx);
	}
	std::string	final_root, new_uri, full_path;
	struct stat	buf;
	// If request is for file: set root from redirection, full_path and set 404 if not found
	if (resource.length() != 0)
	{
		new_uri = this->uri.substr(0, path_info_idx);
		// Return directive on matched location for file request
		if (loc != locations.end() && loc->second.redirect.first != 0 && !loc->second.redirect.second.empty())
		{
			set_error_code(loc->second.redirect.first);
			this->headers.insert(std::make_pair("Location", loc->second.redirect.second));
			return ;
		}
		full_path = this->_find_full_path(this->method, new_uri, srv);
		if (full_path.empty())
			set_error_code(httpStatusCodes.NotFound.code);
		if (this->method != "POST" && ((stat(full_path.c_str(), &buf) == -1) || !S_ISREG(buf.st_mode)))
			set_error_code(httpStatusCodes.NotFound.code);
	}
	// If request is for folder: search for index files with possible multiple redirections
	else
	{
		size_t	i = 0;
		std::vector<std::string> index_files = (loc != locations.end()) ? loc->second.index : srv->index;
		for (; i < index_files.size(); i++)
		{
			new_uri = this->uri.substr(0, path_info_idx);
			new_uri += (index_files[i][0] == '/') ? index_files[i].substr(1) : index_files[i];
			loc = locations.find(get_matched_location(new_uri, locations));
			// Return directive on matched location for folder request
			if (loc != locations.end() && loc->second.redirect.first != 0 && !loc->second.redirect.second.empty())
			{
				set_error_code(loc->second.redirect.first);
				this->headers.insert(std::make_pair("Location", loc->second.redirect.second));
				return ;
			}
			full_path = this->_find_full_path(this->method, new_uri, srv);

			if (full_path.empty())
				set_error_code(httpStatusCodes.NotFound.code);
			// Check if index file is found
			if (this->method == "POST" || ((stat(full_path.c_str(), &buf) != -1) && S_ISREG(buf.st_mode)))
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
					set_error_code(httpStatusCodes.NotFound.code);
				this->autoindex = true;
			}
			else
				set_error_code(httpStatusCodes.Forbidden.code);
		}
	}

	// ATTENTION: NEED TO CHANGE /ETC/HOSTS FILE TO INCLUDE OTHER SERVER_NAMES
	if (is_server_name_forbidden(this->host, srv->ip, srv->server_name))
		set_error_code(httpStatusCodes.NotFound.code);
	
	int	upload(srv->upload);
	if (loc != locations.end())
		upload = loc->second.upload;
	if (!upload && this->method == "POST" && this->headers.find("Content-Type") != this->headers.end() &&
			this->headers["Content-Type"].find("multipart/form-data") != std::string::npos) {
		debug(WARNING, "NO UPLOAD");
		set_error_code(httpStatusCodes.BadRequest.code);
	}

	int	code = this->get_error_code();
	std::map<int, std::string>::iterator it;
	for (it = srv->error_page.begin(); it != srv->error_page.end(); it++)
	{
		if (it->first == code)
		{
			full_path = std::getenv("PWD") + std::string("/") + srv->root + it->second;
			this->full_resource_path = full_path;
			return ;
		}
	}

	if (this->get_error_code() == 0)
		this->full_resource_path = full_path;
}

void	HttpRequest::validate_body( Server *srv ) {
	debug(INFO, "Validating body");
	int	max_body_size = srv->client_max_body_size;

	std::map<std::string, Location> locations = srv->location;
	std::map<std::string, Location>::iterator loc = locations.find(get_matched_location(this->uri, locations));
	if (loc != locations.end())
		max_body_size = loc->second.client_max_body_size;
	if (max_body_size != -1 && (int)this->body.length() > max_body_size)
		set_error_code(httpStatusCodes.PayloadTooLarge.code);
	debug(INFO, "Body validation: " + ft_itoa(this->get_error_code()));
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
