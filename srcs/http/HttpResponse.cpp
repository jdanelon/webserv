#include <fstream>
#include <sstream>
#include "HttpResponse.hpp"
#include "HttpStatusCodes.hpp"

HttpResponse::HttpResponse( void ) : response(""),
										response_line(""),
										headers(std::map<std::string, std::string>()) {
	this->http_version = "HTTP/1.1";
	this->host = NULL;
	this->status_code = 0;
	this->fileOffset = 0;
}

HttpResponse::HttpResponse( Server *server ) : response(""),
										response_line(""),
										headers(std::map<std::string, std::string>()) {
	this->http_version = "HTTP/1.1";
	this->host = server;
	this->status_code = 0;
	this->fileOffset = 0;
}

HttpResponse::HttpResponse( HttpResponse const &obj )
{
	*this = obj;
	return ;
}

HttpResponse::~HttpResponse( void )
{
	return ;
}

HttpResponse &HttpResponse::operator = ( HttpResponse const &obj)
{
	if (this != &obj)
	{
		this->response = obj.response;
		this->response_line = obj.response_line;
		this->headers = obj.headers;
		this->body = obj.body;
		this->status_code = obj.status_code;
		this->http_version = obj.http_version;
		this->host = obj.host;
		this->fileOffset = obj.fileOffset;
		this->fileSize = obj.fileSize;
		this->resourceFullPath = obj.resourceFullPath;

		this->fileHandle.open(obj.resourceFullPath.c_str(), std::ios::binary);
	}
	return (*this);
}

// Called after Parsing the request
void HttpResponse::configureResponse(HttpRequest &request)
{
	// print host root
	if (request.method == "GET")
		this->handleGet(request);
	else if (request.method == "POST")
		this->handlePost(request);
	else if (request.method == "DELETE")
		this->handleDelete(request);
	else
		this->handleGet(request);
}

// Called for generating the response Line and Headers
void HttpResponse::prepareResponseHeaders() {
	this->generateResponseLine();
	this->generateBasicHeaders();

	// Response line + headers
	this->response = this->response_line + "\r\n";
	std::map<std::string, std::string>::iterator it;
	for (it = this->headers.begin(); it != this->headers.end(); ++it)
		this->response += it->first + ": " + it->second + "\r\n";
	this->response += "\r\n";
}

void HttpResponse::handlePost(HttpRequest &request)
{
	(void)request;
	// 1. Read Request Object
	// 2. Generate Response line (ie: HTTP/1.1 200 OK)
	// 3. Generate Headers
	// 4. Generate Body
}

void HttpResponse::handleGet(HttpRequest &request)
{
	std::string uri = request.uri;
	if (uri.empty()) {
		this->setStatusCode(httpStatusCodes.BadRequest.code);
		return;
	}

	// Read the corresponding file
	std::string content = this->configureContent(request);  

	// Store the content as the response body
	this->body = content;
}

void HttpResponse::handleDelete(HttpRequest &request)
{
	(void)request;
	// 1. Read Request Object
	// 2. Generate Response line (ie: HTTP/1.1 200 OK)
	// 3. Generate Headers
	// 4. Generate Body
}

std::string HttpResponse::getResponse( void )
{
	return (this->response);
}

void HttpResponse::setStatusCode( int const &code )
{
	this->status_code = code;
}

// Private

// To-Do: Check if there is a location block that matches the request URI
// If there is, update the root path and index files
void updatePathAndIndexBasedOnLocation(HttpRequest &request, Server *host, std::string &rootPath, std::vector<std::string> &indexFiles) {
	std::map<std::string, Location>::const_iterator it;
	for (it = host->location.begin(); it != host->location.end(); ++it) {
		if (request.uri.find(it->first) == 0) {
			Location loc = it->second;
			// rootPath = loc.root.empty() ? rootPath : loc.root;
			// rootPath = "";
			(void) rootPath;
			indexFiles = loc.index.empty() ? indexFiles : loc.index;
			break;
		}
	}
}

// To-Do: Check if any of the index files exist in the root path
bool handleIndexFiles(const std::string &fullPath, std::vector<std::string> &indexFiles, std::string &content, HttpResponse *response) {
	std::vector<std::string>::const_iterator it;
	for (it = indexFiles.begin(); it != indexFiles.end(); ++it) {
		std::string tmpPath = fullPath + *it;
		std::ifstream file(tmpPath.c_str());

		if (file.is_open()) {
			std::stringstream buffer;
			buffer << file.rdbuf();
			content = buffer.str();
			response->setStatusCode(httpStatusCodes.OK.code);
			return true;
		}
	}
	return false;
}

void HttpResponse::openFile(const std::string &fullPath) {
	if (fileHandle.is_open()) {
		fileHandle.close();
	}
	fileHandle.open(fullPath.c_str(), std::ios::binary);
}

std::string HttpResponse::configureContent(HttpRequest &request)
{
	std::cout << "Generating Content...." << std::endl;
	std::string content;
	// print host root
	std::string rootPath = this->host->root;
	std::vector<std::string> indexFiles = this->host->index; // Assuming index is a property of host

	updatePathAndIndexBasedOnLocation(request, this->host, rootPath, indexFiles);

	// To-Do: Construct the full path
	std::string fullPath = constructPath(rootPath, request.uri);
	this->resourceFullPath = fullPath;

	// To-Do: Check if URI is a directory
	if (request.uri.empty() || request.uri[request.uri.size() - 1] == '/') {
		if (!handleIndexFiles(fullPath, indexFiles, content, this)) {
			content = httpStatusCodes.NotFound.description;
			this->setStatusCode(httpStatusCodes.NotFound.code);
		}
	}
	// Check if URI is a html file
	else if (request.uri.find(".html") != std::string::npos) {
		this->openFile(fullPath);
		this->fileHandle.seekg(0, std::ios::end);  
		this->fileSize = this->fileHandle.tellg();
		this->fileHandle.seekg(0, std::ios::beg);
		this->setStatusCode(httpStatusCodes.OK.code);

		this->headers["Content-Type"] = "text/html";
	}
	else if (request.uri.find(".cgi") != std::string::npos) {
	   // TO-DO: CGI 
	}
	else {
		content = httpStatusCodes.NotFound.description;
		this->setStatusCode(httpStatusCodes.NotFound.code);
	}

	return content;
}

void HttpResponse::generateResponseLine( void )
{
	this->response_line = this->http_version + " " 
		+ ft_itoa(this->status_code) + " " 
		+ httpStatusCodes.getDescription(this->status_code);
}

void HttpResponse::generateBasicHeaders( void )
{
	this->headers["Server"] = "webserv";
	this->headers["Date"] = "Mon, 27 Jul 2009 12:28:53 GMT";
}

std::string HttpResponse::readChunkAndUpdateResponse(size_t chunkSize) {
	char buffer[chunkSize];
	
	std::streamsize bytesRead = 0;

	if (this->fileHandle.is_open()) {
		this->fileHandle.seekg(this->fileOffset);
		this->fileHandle.read(buffer, chunkSize);
		bytesRead = this->fileHandle.gcount();  // Actual bytes read
	} else {
		// Handle the error case where fileHandle is not open
		std::cerr << "File handle is not open!" << std::endl;
		return ""; 
	}

	// Null-terminate the buffer, ensuring that you don't read past the buffer size.
	if (bytesRead < static_cast<std::streamsize>(chunkSize)) {
		buffer[bytesRead] = '\0';
	} else {
		// This is technically an error case; the buffer is not null-terminated
	}

	// Update the offset for the next read
	this->fileOffset += bytesRead;

	// Set header as chunked and prepare response only if its the first chunk
	if (this->fileOffset == bytesRead) {
		this->headers["Transfer-Encoding"] = "chunked";
		this->prepareResponseHeaders();
	}

	// Convert bytesRead to a hexadecimal string
	std::stringstream hexStream;
	hexStream << std::hex << bytesRead;

	// Prepare the chunked response
	std::string chunkedResponse;
	chunkedResponse += hexStream.str();  // chunk size in hex
	chunkedResponse += "\r\n";  // CRLF
	chunkedResponse += std::string(buffer, bytesRead);  // actual chunk data
	chunkedResponse += "\r\n";  // CRLF

	// If we're at the end of the file, append the final zero-length chunk
	if (bytesRead < static_cast<std::streamsize>(chunkSize)) {
		chunkedResponse += "0\r\n\r\n";
	}

	return chunkedResponse;
}

// Debug

void HttpResponse::prepareDummyResponse() {
	this->generateResponseLine();
	this->generateBasicHeaders();

	// Response line + headers
	this->response = this->response_line + "\r\n";
	std::map<std::string, std::string>::iterator it;
	// Set a content length of 12
	this->headers["Content-Length"] = "13";
	for (it = this->headers.begin(); it != this->headers.end(); ++it)
		this->response += it->first + ": " + it->second + "\r\n";
	this->response += "\r\n";

	// a simple html with content length 12
	this->response += "<html></html>";
}

void HttpResponse::print(int client_fd) {
	std::cout << "------------------" << std::endl;
	std::cout << "Client fd: " << client_fd << std::endl;
	std::cout << "Response Object: " << std::endl;
	std::cout << "------------------" << std::endl;
	std::cout << "Status Code: " << this->status_code << std::endl;
	std::cout << "HTTP Version: " << this->http_version << std::endl;
	std::cout << "Response Line: " << this->response_line << std::endl;
	std::cout << "Headers: " << std::endl;
	std::map<std::string, std::string>::iterator it;
	for (it = this->headers.begin(); it != this->headers.end(); ++it)
		std::cout << it->first << ": " << it->second << std::endl;
	std::cout << "Body: " << std::endl;
	std::cout << this->body << std::endl;
	std::cout << "File size: " << this->fileSize << std::endl;
	std::cout << "File offset: " << this->fileOffset << std::endl;
	std::cout << "------------------" << std::endl;
}
