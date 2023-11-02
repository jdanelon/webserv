#include "HttpResponse.hpp"

bool HttpResponse::debugEnabled = true;
const std::string HttpResponse::className = "HttpResponse";

HttpResponse::HttpResponse( void ) : response(""),
									 response_line(""),
									 headers(std::map<std::string, std::string>()) {
	this->http_version = "HTTP/1.1";
	this->host = NULL;
	this->status_code = 0;
	this->fileSize = 0;
	this->fileOffset = 0;
	this->resourceFullPath = "";
	this->is_request_valid = true;
}

HttpResponse::HttpResponse( Server *server ) : response(""),
											   response_line(""),
											   headers(std::map<std::string, std::string>()) {
	this->http_version = "HTTP/1.1";
	this->host = server;
	this->status_code = 0;
	this->fileSize = 0;
	this->fileOffset = 0;
	this->resourceFullPath = "";
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
			this->openFile(obj.resourceFullPath);
	}
	return (*this);
}

// Called after Parsing the request
void HttpResponse::configureResponse(HttpRequest &request)
{
	this->resourceFullPath = request.full_resource_path;
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
		this->handleDelete();
	else
		std::cout << "\tMETHOD_ERROR" << std::endl;
		// this->handleGet(request);
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

void HttpResponse::handleGet(HttpRequest &request)
{
	std::string content;

	size_t extension_idx = this->resourceFullPath.find_last_of(".");
	if (extension_idx == std::string::npos)
		extension_idx = 0;
	std::string extension = this->resourceFullPath.substr(extension_idx);

	// AUTOINDEX
	if (request.autoindex) {
		content = autoindex_content(this->resourceFullPath, this->host->root, this->host->port);
		if (content.empty())
			this->setStatusCode(httpStatusCodes.InternalServerError.code);
		else
		{
			this->resourceFullPath += generateUniqueFilename();
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
	// CGI
	else if (this->host->cgi.find(extension) != this->host->cgi.end()) {
		std::string	exe = this->host->cgi[extension];
		try {
			content = handle_cgi(exe, this->resourceFullPath, request, this->host->root);
			this->headers["Content-Type"] = "text/html";
			if (content.empty())
			{
				content = "<!DOCTYPE html>\n";
				content += "<html>\n<head>\n";
				content += "<title>Internal Server Error</title>\n";
				content += "</head>\n<body>\n";
				content += "CGI has timed out!\n";
				content += "</body>\n</html>\n";
				this->setStatusCode(httpStatusCodes.InternalServerError.code);
			}
			else
				this->setStatusCode(httpStatusCodes.OK.code);
		}
		catch (const std::exception& e)
		{
			this->setStatusCode(httpStatusCodes.InternalServerError.code);
		}
		std::string tmp_file_path = this->resourceFullPath.substr(0, this->resourceFullPath.find_last_of("/") + 1);
		this->resourceFullPath = tmp_file_path + generateUniqueFilename();
		std::ofstream output_file(this->resourceFullPath.c_str());
		output_file << content;
		output_file.close();
		this->openFile(this->resourceFullPath);
		this->fileHandle.seekg(0, std::ios::end);
		this->fileSize = this->fileHandle.tellg();
		this->fileHandle.seekg(0, std::ios::beg);
	}
	// File exists, but is not handled by cgi
	else {
		std::string contentType = httpContentTypes.getDescription(extension);
		if (contentType.empty())
		{
			this->is_request_valid = false;
			this->resourceFullPath = generateUniqueFilename();
			this->setStatusCode(httpStatusCodes.UnsupportedMediaType.code);
		}
		else
		{
			this->headers["Content-Type"] = contentType;

			this->openFile(this->resourceFullPath);
			this->fileHandle.seekg(0, std::ios::end);
			this->fileSize = this->fileHandle.tellg();
			this->fileHandle.seekg(0, std::ios::beg);
			if (this->fileSize == 0)
				this->setStatusCode(httpStatusCodes.NoContent.code);
			else
				this->setStatusCode(httpStatusCodes.OK.code);
		}
	}
}

void HttpResponse::handlePost(HttpRequest &request)
{
	// WORK WITH HTML FILES?
	std::string content;

	size_t extension_idx = this->resourceFullPath.find_last_of(".");
	if (extension_idx == std::string::npos)
		extension_idx = 0;
	std::string extension = this->resourceFullPath.substr(extension_idx);

	if (this->host->cgi.find(extension) != this->host->cgi.end()) {
		std::string	exe = this->host->cgi[extension];
		try {
			content = handle_cgi(exe, this->resourceFullPath, request, this->host->root);
			debug(INFO, "CGI CONTENT: '" + content + "'");
			this->headers["Content-Type"] = "text/html";
			if (content.empty())
			{
				content = "<!DOCTYPE html>\n";
				content += "<html>\n<head>\n";
				content += "<title>Internal Server Error</title>\n";
				content += "</head>\n<body>\n";
				content += "CGI has timed out!\n";
				content += "</body>\n</html>\n";
				this->setStatusCode(httpStatusCodes.InternalServerError.code);
			}
			else
				this->setStatusCode(httpStatusCodes.OK.code);
		}
		catch (const std::exception& e)
		{
			this->setStatusCode(httpStatusCodes.InternalServerError.code);
		}
		std::string tmp_file_path = this->resourceFullPath.substr(0, this->resourceFullPath.find_last_of("/") + 1);
		this->resourceFullPath = tmp_file_path + generateUniqueFilename();
		std::ofstream output_file(this->resourceFullPath.c_str());
		output_file << content;
		output_file.close();
		this->openFile(this->resourceFullPath);
		this->fileHandle.seekg(0, std::ios::end);
		this->fileSize = this->fileHandle.tellg();
		this->fileHandle.seekg(0, std::ios::beg);
		return ;
	}

	std::ofstream	new_file;
	std::string		request_body = request.body;

	new_file.open(this->resourceFullPath.c_str(), std::ios::binary);
	if (new_file.fail())
	{
		this->is_request_valid = false;
		this->setStatusCode(httpStatusCodes.InternalServerError.code);
		return ;
	}

	new_file.write(request_body.c_str(), request_body.length());
	new_file.close();

	size_t		location_idx = this->resourceFullPath.find(this->host->root) + this->host->root.length();
	std::string	location = this->resourceFullPath.substr(location_idx);
	this->headers["Location"] = location;

	std::string tmp_file_path = this->resourceFullPath.substr(0, this->resourceFullPath.find_last_of("/") + 1);
	this->resourceFullPath = tmp_file_path + generateUniqueFilename();
	std::ofstream output_file(this->resourceFullPath.c_str());
	output_file << "Created Successfully!\n";
	output_file.close();
	this->openFile(this->resourceFullPath);
	this->fileHandle.seekg(0, std::ios::end);
	this->fileSize = this->fileHandle.tellg();
	this->fileHandle.seekg(0, std::ios::beg);

	this->headers["Content-Type"] = "text/plain";
	this->setStatusCode(httpStatusCodes.Created.code);
}

void HttpResponse::handleDelete( void )
{
	struct stat buf;
	stat(this->resourceFullPath.c_str(), &buf);
	if (S_ISDIR(buf.st_mode))
	{
		this->is_request_valid = false;
		setStatusCode(httpStatusCodes.Forbidden.code);
		return ;
	}
	std::remove(this->resourceFullPath.c_str());

	std::string tmp_file_path = this->resourceFullPath.substr(0, this->resourceFullPath.find_last_of("/") + 1);
	this->resourceFullPath = tmp_file_path + generateUniqueFilename();
	std::ofstream output_file(this->resourceFullPath.c_str());
	output_file << "Deleted Successfully!\n";
	output_file.close();
	this->openFile(this->resourceFullPath);
	this->fileHandle.seekg(0, std::ios::end);
	this->fileSize = this->fileHandle.tellg();
	this->fileHandle.seekg(0, std::ios::beg);

	this->setStatusCode(httpStatusCodes.OK.code);
	this->headers["Content-Type"] = "text/plain";
}

std::string HttpResponse::getResponse( void )
{
	return (this->response);
}

void HttpResponse::setStatusCode( int const &code )
{
	this->status_code = code;
}

void HttpResponse::openFile(const std::string &fullPath) {
	if (fileHandle.is_open()) {
		fileHandle.close();
	}
	fileHandle.open(fullPath.c_str(), std::ios::binary);
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

void HttpResponse::prepareFullResponse( void ) {

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

void HttpResponse::prepareErrorResponse( HttpRequest &request )
{
	std::string		errorRoute = this->resourceFullPath;
	std::ifstream	errorFile(errorRoute.c_str(), std::ios::in);
	std::string		fileContent;

	errorFile.seekg(0, std::ios::end);
	this->fileSize = errorFile.tellg();
	errorFile.seekg(0, std::ios::beg);

	if (this->fileSize && errorFile.is_open()) {
		fileContent.assign((std::istreambuf_iterator<char>(errorFile)),
						   (std::istreambuf_iterator<char>()));
		errorFile.close();
	} else {
		fileContent = "<html>\n<body>\n<h1>" 
			+ httpStatusCodes.getDescription(this->status_code) 
			+ "</h1>\n<p>This is a default error page.</p>"
			+ "\n</body>\n<a href=\"http://localhost:3490\">Home</a>\n</html>\n";
	}
	// Generate Response String
	this->response = "";
	this->generateResponseLine();
	this->generateBasicHeaders();

	if (request.headers.find("Location") != request.headers.end())
		this->headers["Location"] = request.headers["Location"];

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

void HttpResponse::debug(LogLevel level, const std::string &message)
{
	if (!debugEnabled)
	{
		return;
	}

	std::string prefix;
	std::string colorCode;
	switch (level)
	{
	case INFO:
		prefix = "[INFO] ";
		colorCode = "\033[1;34m"; // Blue
		break;
	case WARNING:
		prefix = "[WARNING] ";
		colorCode = "\033[1;33m"; // Yellow
		break;
	case ERROR:
		prefix = "[ERROR] ";
		colorCode = "\033[1;31m"; // Red
		break;
	}

	std::cout << colorCode << className << " " << prefix << message << "\033[0m" << std::endl; // \033[0m resets the color
}

