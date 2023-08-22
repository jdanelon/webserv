#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <iostream>
# include <sstream>
# include <cstring>
# include <map>
# include <algorithm>
# include <sys/stat.h>
# include <unistd.h>
# include "../utils.hpp"

struct CaseInsensitive {
	bool	operator () ( std:: string const &s1, std::string const &s2 ) const {
		std::string	str1(s1.length(), ' ');
		std::string	str2(s2.length(), ' ');

		std::transform(str1.begin(), str1.end(), str1.begin(), ft_tolower);
		std::transform(str2.begin(), str2.end(), str2.begin(), ft_tolower);
		return (str1 < str2);
	}
};

class HttpRequest {

	public:

		std::string											method;
		std::string											uri;
		std::string											version;
		std::string											host;
		std::map<std::string, std::string, CaseInsensitive>	headers;
		std::string											body;
		std::string											raw;

		HttpRequest( void );
		HttpRequest( HttpRequest const &obj );
		HttpRequest &operator = ( HttpRequest const &obj );
		virtual ~HttpRequest( void );

		void	parse( std::string raw );
		void	parse_request_line( std::string line );
		void	parse_header_line( std::string line );
		void	validate( void );
		int		get_error_code( void ) const;
		void	set_error_code( int const &code );

		//Debug
		void	print( void );
	
	private:

		int	_error_code;

};

#endif
