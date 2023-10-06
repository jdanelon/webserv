#include "HttpResponse.hpp"
#include "HttpStatusCodes.hpp"

HttpResponse::HttpResponse( void ) : response(""),
										response_line(""),
										headers(std::map<std::string, std::string>()) {
	this->http_version = "HTTP/1.1";
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
	}
	return (*this);
}

void HttpResponse::generateResponse(HttpRequest &request)
{
	(void)request;
	if (request.method == "GET")
		this->handleGet(request);
	else if (request.method == "POST")
		this->handlePost(request);
	else if (request.method == "DELETE")
		this->handleDelete(request);
	else
		this->handleGet(request);

	// Set response line and headers
	this->setStatusCode(request.get_error_code());
	this->generateResponseLine();
	this->generateHeaders();

	// Response line + headers
	this->response = this->response_line + "\r\n";
	std::map<std::string, std::string>::iterator it;
	for (it = this->headers.begin(); it != this->headers.end(); ++it)
		this->response += it->first + ": " + it->second + "\r\n";
	this->response += "\r\n";

	// Body (to be added later)
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
	(void)request;
	// 1. Read Request Object
	// 2. Generate Response line (ie: HTTP/1.1 200 OK)
	// 3. Generate Headers
	// 4. Generate Body
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

void HttpResponse::generateResponseLine( void )
{
	this->response_line = this->http_version + " " 
		+ ft_itoa(this->status_code) + " " 
		+ httpStatusCodes.getDescription(this->status_code);
}

void HttpResponse::generateHeaders( void )
{
	this->headers["Server"] = "webserv";
	// this->headers["Date"] = "Mon, 27 Jul 2009 12:28:53 GMT";
	this->headers["Date"] = get_time_string();
	this->headers["Content-Type"] = "text/html";
	this->headers["Content-Length"] = ft_itoa(this->body.size());
}

// Debug

void HttpResponse::print(int client_fd) {
	std::cout << "------------------" << std::endl;
	std::cout << "Client fd: " << client_fd << std::endl;
	std::cout << "Response to send: " << std::endl;
	std::cout << this->response << std::endl;
	std::cout << "------------------" << std::endl;
}