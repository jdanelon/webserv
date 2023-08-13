#include "HttpRequest.hpp"

HttpRequest::HttpRequest( void ) {}

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
}

void	HttpRequest::parse_request_line( std::string line ) {
	std::istringstream	iss(line);
	std::string			token;

	// Store the values read from the request line and validate they exist
	// if (!(iss >> this->method >> this->uri >> this->version) || iss.eof()) {
	// 	std::cout << "Error: Invalid request line" << std::endl;
    // }

	size_t	first_space, second_space;

	first_space = line.find_first_of(" ");
	second_space = line.find_first_of(" ", first_space + 1);
	if (first_space == std::string::npos || second_space == std::string::npos ||
		second_space != line.find_last_of(" "))
	{
		std::cout << "Error: Invalid request line" << std::endl;
		// If invalid - ERROR CODE 400 (Bad Request)
		return ;
	}
	this->method = line.substr(0, first_space);
	this->uri = line.substr(first_space + 1, second_space - first_space - 1);
	this->version = line.substr(second_space + 1, line.length() - second_space - 2);

	// Version is not (HTTP + / + DIGIT + . + DIGIT)? - ERROR CODE 400 (Bad Request)
	if (this->version.length() != 8 || this->version.compare(0, 5, "HTTP/") ||
		!isdigit(this->version[5]) || this->version[6] != '.' || !isdigit(this->version[7]))
	{
		std::cout << "Error: Invalid request line" << std::endl;
		// If invalid - ERROR CODE 400 (Bad Request)
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

    if (key.empty() || value.empty()) {
        std::cout << "Error: Invalid header line" << std::endl;
		// If invalid - ERROR CODE 400 (Bad Request)
        return ;
    }

	// this->headers[key] = value;
	std::pair<std::map<std::string, std::string, CaseInsensitive>::iterator, bool>	ret;
	ret = this->headers.insert(std::make_pair(key, value));
	if (!ret.second) {
        std::cout << "Error: Invalid header line" << std::endl;
		// If invalid - ERROR CODE 400 (Bad Request)
        return ;
    }
}

void	HttpRequest::validate( void ) {
	std::cout << "Validating request:" << std::endl;

	// Check if the request line is valid
	if (this->method.empty() || this->uri.empty() || this->version.empty()) {
		std::cout << "Error: Invalid request line" << std::endl;
		// If invalid - ERROR CODE 400 (Bad Request)
	}

	if (this->method == "HEAD" || this->method == "PUT" || this->method == "CONNECT" ||
		this->method == "OPTIONS" || this->method == "TRACE" || this->method == "PATCH")
	{
		std::cout << "Error: Method not allowed" << std::endl;
		// Method is HEAD/PUT/CONNECT/OPTIONS/TRACE/PATCH? - ERROR CODE 405 (Method Not Allowed)
	}

	if (this->method != "GET" && this->method != "POST" && this->method != "DELETE")
	{
		std::cout << "Error: Method not implemented" << std::endl;
		// Method is other? - ERROR CODE 501 (Not Implemented)
	}

	if (this->uri.length() > 8000)
	{
		std::cout << "Error: URI too long" << std::endl;
		// Check for URI too long(?) - ERROR CODE 414 (URI Too Long)
	}

	char		dir[256];
	struct stat	buf;
	getcwd(dir, sizeof(dir));
	std::string	path(dir + std::string("/") + this->uri);
	if ((stat(path.c_str(), &buf) == -1) || (!S_ISDIR(buf.st_mode | S_IRUSR) && !(buf.st_mode & S_IXUSR)))
	{
		std::cout << "Error: Not found" << std::endl;
		// URI does not exist - ERROR CODE 404 (Not Found)
	}

	if (this->version != "HTTP/1.1")
	{
		std::cout << "Error: HTTP version not supported" << std::endl;
		// Version not HTTP/1.1 - ERROR CODE 505 (HTTP Version Not Supported)
	}

	// Other checks:
}

// Debug
void	HttpRequest::print( void ) {
	// Print a header to see where the request starts
	std::cout << "------------------" << std::endl;
	std::cout << "Printing parsed request:" << std::endl;
	std::cout << "method: '" << this->method << "'" << std::endl;
	std::cout << "uri: '" << this->uri << "'" << std::endl;
	std::cout << "version: '" << this->version << "'" << std::endl;
	std::cout << "host: " << this->host << std::endl;
	std::cout << "headers: " << std::endl;
	for (std::map<std::string, std::string>::iterator it = this->headers.begin(); it != this->headers.end(); ++it) {
		std::cout << "\t" << it->first << ": " << it->second << std::endl;
	}
	std::cout << "body: " << this->body << std::endl;
	std::cout << "------------------" << std::endl;
}
