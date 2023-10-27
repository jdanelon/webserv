#ifndef HTTP_CONTENT_TYPES_HPP
# define HTTP_CONTENT_TYPES_HPP

# include <string>
# include <map>

class HttpContentTypes
{
	public:

		HttpContentTypes();

		std::string	getDescription( std::string extension );

	private:

		std::map<std::string, std::string> contentTypesMap;
};

// Global object, available for the entire project
extern HttpContentTypes httpContentTypes;

#endif // HTTP_STATUS_CODE_HPP
