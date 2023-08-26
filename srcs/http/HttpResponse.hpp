#ifndef HTTP_RESPONSE
# define HTTP_RESPONSE

# include <iostream>
# include <map>

# include "./HttpRequest.hpp"

class HttpResponse {
	public:

		HttpResponse( void );
		HttpResponse( HttpResponse const &obj );
		HttpResponse &operator = ( HttpResponse const &obj );
		virtual ~HttpResponse( void );

		void generateResponse(HttpRequest &request);
		void handlePost(HttpRequest &request);
		void handleGet(HttpRequest &request); // CGI
		void handleDelete(HttpRequest &request);

		std::string getResponse( void );
		void setStatusCode( int const &code );

		// Debug
		void print( int client_fd );
	private:
		void generateResponseLine( void );
		void generateHeaders( void );

		std::string response;
		std::string response_line;
		std::map<std::string, std::string> headers;
		
		int status_code;
		std::string http_version;
		std::string body;
};

#endif // !HTTP_RESPONSE
