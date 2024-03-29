#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <iostream>
# include <sstream>
# include <cstring>
# include <map>
# include <algorithm>
# include <sys/stat.h>
# include <unistd.h>
# include "../parser/Location.hpp"
# include "../utils.hpp"
# include "HttpRequestBody.hpp"
# include "HttpStatusCodes.hpp"

struct CaseInsensitive {
	bool	operator () ( std:: string const &s1, std::string const &s2 ) const {
		std::string	str1(s1);
		std::string	str2(s2);

		std::transform(str1.begin(), str1.end(), str1.begin(), ft_tolower);
		std::transform(str2.begin(), str2.end(), str2.begin(), ft_tolower);
		return (str1 < str2);
	}
};

class HttpRequest {

	public:
		static bool debugEnabled;
		static const std::string className;

		std::string											method;
		std::string											uri;
		std::string											version;
		std::string											host;
		std::map<std::string, std::string, CaseInsensitive>	headers;
		bool												chunked_body;
		std::string											body;
		std::string											raw;
		bool												autoindex;
		std::string											full_resource_path;
		std::string											full_upload_path;
		std::string											path_info;
		std::string											query_string;
		bool												is_valid;
		bool												has_body;
		bool												is_body_parsed;

		HttpRequestBody										body_parser;

		HttpRequest( void );
		HttpRequest( HttpRequest const &obj );
		HttpRequest &operator = ( HttpRequest const &obj );
		virtual ~HttpRequest( void );

		void	parse( std::string raw );
		void	parse_request_line( std::string line );
		void	parse_header_line( std::string line );
		void	parse_body( std::string partial_body);
		void	validate_headers( Server *srv );
		void	validate_body( Server *srv );
		int		get_error_code( void ) const;
		void	set_error_code( int const &code );

		//Debug
		void	print( int client_fd );
		static void debug(LogLevel level, const std::string& message);

	private:
		int	_error_code;

		std::string	_find_full_path( std::string method, std::string new_uri, Server *srv );
};

#endif
