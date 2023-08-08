#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <map>
#include <iostream>

class HttpRequest {
	public:
		std::string method;
		std::string uri;
		std::string version;
		std::string host;
		std::map<std::string, std::string> headers;
		std::string body;
		std::string raw;

		HttpRequest();
		HttpRequest(HttpRequest const &obj);
		HttpRequest &operator = (HttpRequest const &obj);
		~HttpRequest();

		void parse(std::string raw);
		void parse_request_line(std::string line);
		void parse_header_line(std::string line);
		void validate();

		//Debug
		void print();
};

#endif
	