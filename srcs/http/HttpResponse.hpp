#ifndef HTTP_RESPONSE
# define HTTP_RESPONSE

# include <iostream>
# include <map>
# include <fstream>

# include "./HttpRequest.hpp"
# include "../utils.hpp"

class HttpResponse {
	public:
		HttpResponse( void );
		HttpResponse( Server *server );
		HttpResponse( HttpResponse const &obj );
		HttpResponse &operator = ( HttpResponse const &obj );
		virtual ~HttpResponse( void );

		void configureResponse(HttpRequest &request);
		void handlePost(HttpRequest &request);
		void handleGet(HttpRequest &request); // CGI
		void handleDelete(HttpRequest &request);

		void prepareResponseHeaders();
		void openFile(const std::string &fullPath);
		std::string readChunkAndUpdateResponse(size_t chunkSize);

		std::string getResponse( void );
		void setStatusCode( int const &code );

		// Content configuration settings
		std::ifstream fileHandle;
		std::streamsize fileSize;
		unsigned int fileOffset;
		std::string resourceFullPath;

		// Debug
		void prepareDummyResponse( void );
		void print( int client_fd );
	private:
		void generateResponseLine( void );
		void generateBasicHeaders( void );
		std::string configureContent(HttpRequest &request);

		std::string response;
		std::string response_line;
		std::map<std::string, std::string> headers;
		
		int status_code;
		std::string http_version;
		std::string body;

		Server		*host;
};

#endif // !HTTP_RESPONSE
