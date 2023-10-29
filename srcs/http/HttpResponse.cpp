#include "HttpResponse.hpp"

HttpResponse::HttpResponse( void ) : response(""),
									 response_line(""),
									 headers(std::map<std::string, std::string>()) {
	this->http_version = "HTTP/1.1";
	this->host = NULL;
	this->status_code = 0;
	this->fileOffset = 0;
	this->is_request_valid = true;
}

HttpResponse::HttpResponse( Server *server ) : response(""),
											   response_line(""),
											   headers(std::map<std::string, std::string>()) {
	this->http_version = "HTTP/1.1";
	this->host = server;
	this->status_code = 0;
	this->fileOffset = 0;
	this->is_request_valid = true;
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

HttpResponse &HttpResponse::operator = ( HttpResponse const &obj )
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
		this->is_request_valid = obj.is_request_valid;

		if (!obj.resourceFullPath.empty())
			this->fileHandle.open(obj.resourceFullPath.c_str(), std::ios::binary);
	}
	return (*this);
}

// Called after Parsing the request
void HttpResponse::configureResponse(HttpRequest &request)
{
	if (!request.is_valid) {
		std::cout << "Invalid Request" << std::endl;
		this->is_request_valid = false;
		this->setStatusCode(request.get_error_code());
		std::cout << "Error Code: " << this->status_code << std::endl;
		return ;
	}
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
	// Request.validate already brings 400
	// TODO, I think it was validated before
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
	this->resourceFullPath = request.resource;

	struct stat buf;
	stat(this->resourceFullPath.c_str(), &buf);
	if (S_ISDIR(buf.st_mode))
	{
		setStatusCode(httpStatusCodes.Forbidden.code);
		return ;
	}
	std::remove(this->resourceFullPath.c_str());

	this->setStatusCode(httpStatusCodes.OK.code);
	this->headers["Content-Type"] = "text/txt";
	this->body = "Deleted Successfully!\n";
}

std::string HttpResponse::getResponse( void )
{
	return (this->response);
}

void HttpResponse::setStatusCode( int const &code )
{
	this->status_code = code;
}

// Check if there is a location block that matches the request URI
// If there is, update the root path and index files
void updatePathAndIndexBasedOnLocation(HttpRequest &request, Server *host, std::string &rootPath, std::vector<std::string> &indexFiles) {
	std::map<std::string, Location>::const_iterator it;
	for (it = host->location.begin(); it != host->location.end(); ++it) {
		if (request.uri.find(it->first) == 0) {
			Location loc = it->second;
			rootPath = loc.alias.empty() ? rootPath : loc.alias;
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

static std::string	autoindex_content( std::string path, std::string root, std::string port )
{
	DIR	*dir = opendir(path.c_str());
	if (!dir)
		return ("");
	std::string	autoindex_path = path.substr(std::string(std::getenv("PWD")).length());
	size_t	root_index = autoindex_path.find(root);
	if (root_index != std::string::npos)
		autoindex_path = autoindex_path.substr(root_index + root.length());
	std::string	index_page = "<!DOCTYPE html>\n<html>\n<head>\n<title>" + autoindex_path +
							 "</title>\n</head>\n<body>\n<h1>Index of " + autoindex_path + "</h1>\n<p>\n";
	struct dirent *list;
	while ((list = readdir(dir)) != NULL)
	{
		std::string	item(list->d_name);
		struct stat	buf;
		stat((path + "/" + item).c_str(), &buf);

		char		time_str[30];
		std::string	last_modified("");
		std::string	file_size("");
		if (item != "." && item != "..")
		{
			std::strftime(time_str, sizeof(time_str), "%d-%b-%Y %H:%M", std::localtime(&(buf.st_ctime)));
			last_modified = std::string(time_str);
			if (!S_ISDIR(buf.st_mode))
				file_size = ft_itoa(buf.st_size);
			else
				file_size = "-";
		}
		if (S_ISDIR(buf.st_mode))
			item += "/";

		std::stringstream	ss;
		ss	<< "\t\t<p><a href=\"http://localhost" << ":" << port << autoindex_path
		<< item + "\">" + item + "</a>&emsp;&emsp;" + last_modified + "&emsp;&emsp;" + file_size + "</p>\n";
		index_page += ss.str();
	}
	index_page += "</p>\n</body>\n</html>\n";
	closedir(dir);
	return (index_page);
}

std::string HttpResponse::configureContent(HttpRequest &request)
{
	std::cout << "Generating Content...." << std::endl;
	std::string content;

	this->resourceFullPath = request.resource;
	size_t	extension_idx = this->resourceFullPath.find_last_of(".");
	if (extension_idx == std::string::npos)
		extension_idx = 0;
	std::string	extension = this->resourceFullPath.substr(extension_idx);

	if (request.autoindex) {
		// AUTOINDEX
		content = autoindex_content(this->resourceFullPath, this->host->root, this->host->port);
		if (content.empty())
			this->setStatusCode(httpStatusCodes.InternalServerError.code);
		else
		{
			this->resourceFullPath += DFL_TMP_FILE;
			std::ofstream output_file(this->resourceFullPath.c_str());
			output_file << content;
			output_file.close();
			this->openFile(this->resourceFullPath);
			this->fileHandle.seekg(0, std::ios::end);  
			this->fileSize = this->fileHandle.tellg();
			this->fileHandle.seekg(0, std::ios::beg);
			this->setStatusCode(httpStatusCodes.OK.code);
			this->headers["Content-Type"] = "text/html";
		}
	}
	// Check if URI is a html file
	else if (extension == ".htm" || extension == ".html") {
		std::cout << "HTML file" << std::endl;
		this->openFile(this->resourceFullPath);
		this->fileHandle.seekg(0, std::ios::end);
		this->fileSize = this->fileHandle.tellg();
		this->fileHandle.seekg(0, std::ios::beg);
		this->setStatusCode(httpStatusCodes.OK.code);

		this->headers["Content-Type"] = "text/html";
	}
	else if (this->host->cgi.find(extension) != this->host->cgi.end()) {
		// CGI
		std::string	exe = this->host->cgi[extension];
		try {
			content = handle_cgi(exe, this->resourceFullPath, request);
			if (content.empty())
			{
				content = "<!DOCTYPE html>\n";
				content += "<html>\n<head>\n";
				content += "<title>Internal Server Error</title>\n";
				content += "</head>\n<body>\n";
				content += "CGI has timed out!\n";
				content += "</body>\n</html>\n";
				this->setStatusCode(httpStatusCodes.InternalServerError.code);
				this->headers["Content-Type"] = "text/html";
			}
			else
			{
				this->setStatusCode(httpStatusCodes.OK.code);
				this->headers["Content-Type"] = "text/html";
			}
		}
		catch (const std::exception& e)
		{
			this->setStatusCode(httpStatusCodes.InternalServerError.code);
		}
		this->resourceFullPath = this->resourceFullPath.substr(0, this->resourceFullPath.find_last_of("/")) + DFL_TMP_FILE;
		std::ofstream output_file(this->resourceFullPath.c_str());
		output_file << content;
		output_file.close();
		this->openFile(this->resourceFullPath);
		this->fileHandle.seekg(0, std::ios::end);
		this->fileSize = this->fileHandle.tellg();
		this->fileHandle.seekg(0, std::ios::beg);
	}
	// Does this happen?
	// On request validation, we already check if there is a file (folder for autoindex) to respond
	// If not, error is set and response catches it before this function
	else {
		content = httpStatusCodes.NotFound.description;
		this->setStatusCode(httpStatusCodes.NotFound.code);
	}
	return (content);
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
	this->headers["Date"] = get_time_string();
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
		setStatusCode(httpStatusCodes.InternalServerError.code);
		throw std::runtime_error("Error opening file");
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
	return (chunkedResponse);
}

void HttpResponse::prepareFullResponse(HttpRequest &request) {

	std::string rootPath = this->host->root;
	std::vector<std::string> indexFiles = this->host->index;
	updatePathAndIndexBasedOnLocation(request, this->host, rootPath, indexFiles);

	std::string fileContent;
	if (fileHandle.is_open()) {
		fileContent.assign((std::istreambuf_iterator<char>(fileHandle)),
						(std::istreambuf_iterator<char>()));
		fileHandle.close();	
	}
	else {
		setStatusCode(httpStatusCodes.InternalServerError.code);
		throw std::runtime_error("Error opening file");
	}

	this->generateResponseLine();
	this->generateBasicHeaders();

	// Set content length
	this->headers["Content-Length"] = ft_itoa(fileContent.length());

	// Response line + headers
	this->response = this->response_line + "\r\n";
	std::map<std::string, std::string>::iterator it;
	for (it = this->headers.begin(); it != this->headers.end(); ++it)
		this->response += it->first + ": " + it->second + "\r\n";
	this->response += "\r\n";

	this->response += fileContent;
}

void HttpResponse::prepareErrorResponse(HttpRequest &request) {
	std::string rootPath = this->host->root;
	std::vector<std::string> indexFiles = this->host->index;
	updatePathAndIndexBasedOnLocation(request, this->host, rootPath, indexFiles);
	std::string errorPage = host->error_page[this->status_code];
	std::string errorRoute = constructPath(rootPath, errorPage);

	std::ifstream errorFile(errorRoute.c_str(), std::ios::in);
	std::string fileContent;

	if (errorFile.is_open()) {
		fileContent.assign((std::istreambuf_iterator<char>(errorFile)),
						   (std::istreambuf_iterator<char>()));
		errorFile.close();
	} else {
		fileContent = "<html><body><h1>\n" 
			+ httpStatusCodes.getDescription(this->status_code) 
			+ "\n<p>This is a default error page.</p>"
			+ "\n</h1></body></html>";
	}
	// Generate Response String
	this->response = "";
	this->generateResponseLine();
	this->generateBasicHeaders();

	// Set content length
	this->headers["Content-Length"] = ft_itoa(fileContent.length());

	// Response line + headers
	this->response = this->response_line + "\r\n";
	std::map<std::string, std::string>::iterator it;
	for (it = this->headers.begin(); it != this->headers.end(); ++it)
		this->response += it->first + ": " + it->second + "\r\n";
	this->response += "\r\n";

	this->response += fileContent;
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
