#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>
# include <cstring>
# include <vector>

# include "Server.hpp"

class Server;

class Location
{

	public:

		std::string							root;
		std::vector<std::string>			index;
		std::vector<std::string>			limit_except;
		int									client_max_body_size;
		int									autoindex;
		std::map<std::string, std::string>	cgi;
		std::pair<size_t, std::string>		redirect;
		int									upload;
		std::string							upload_store;

		Location( void );
		Location( Location const &obj );
		Location &operator = ( Location const &obj );
		virtual ~Location( void );

		void	fill_with_defaults( Server const &srv );
};

#endif
