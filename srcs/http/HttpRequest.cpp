#include <iostream>
#include <sstream>

#include "HttpRequest.hpp"

HttpRequest::HttpRequest() {}

HttpRequest::HttpRequest(HttpRequest const &obj) {
	*this = obj;
}

HttpRequest &HttpRequest::operator = (HttpRequest const &obj) {
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

HttpRequest::~HttpRequest() {}

void HttpRequest::parse(std::string raw) {
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

void HttpRequest::parse_request_line(std::string line) {
	std::istringstream iss(line);
	std::string token;

	// Store the values read from the request line and validate they exist
	if (!(iss >> this->method >> this->uri >> this->version) || !iss.eof()) {
		std::cout << "Error: Invalid request line" << std::endl;
    }
}

void HttpRequest::parse_header_line(std::string line) {
    std::istringstream iss(line);
    std::string key;
    std::string value;

    // Read the key (everything before the colon)
    std::getline(iss, key, ':');

    // Use the stream extractor to read the value, skipping leading whitespace
    iss >> std::ws; // Skip any leading whitespace
    std::getline(iss, value);

    if (key.empty() || value.empty()) {
        std::cout << "Error: Invalid header line" << std::endl;
        return;
    }

    this->headers[key] = value;
}

void HttpRequest::validate() {
	std::cout << "Validating request:" << std::endl;

	// Check if the request line is valid
	if (this->method.empty() || this->uri.empty() || this->version.empty()) {
		std::cout << "Error: Invalid request line" << std::endl;
	}

	// Other checks
}

// Debug
void HttpRequest::print() {
	// Print a header to see where the request starts
	std::cout << "------------------" << std::endl;
	std::cout << "Printing parsed request:" << std::endl;
	std::cout << "method: " << this->method << std::endl;
	std::cout << "uri: " << this->uri << std::endl;
	std::cout << "version: " << this->version << std::endl;
	std::cout << "host: " << this->host << std::endl;
	std::cout << "headers: " << std::endl;
	for (std::unordered_map<std::string, std::string>::iterator it = this->headers.begin(); it != this->headers.end(); ++it) {
		std::cout << it->first << ": " << it->second << std::endl;
	}
	std::cout << "body: " << this->body << std::endl;
	std::cout << "------------------" << std::endl;
}