#include "HttpContentTypes.hpp"

HttpContentTypes::HttpContentTypes( void )
{
	contentTypesMap[".txt"] = "text/plain";
	contentTypesMap[".html"] = "text/html";
	contentTypesMap[".css"] = "text/css";
	contentTypesMap[".js"] = "text/javascript";
	contentTypesMap[".json"] = "application/json";
	contentTypesMap[".xml"] = "application/xml";
	contentTypesMap[".pdf"] = "application/pdf";
	contentTypesMap[".zip"] = "application/zip";
	contentTypesMap[".gzip"] = "application/gzip";
	contentTypesMap[".tar"] = "application/x-tar";
	contentTypesMap[".png"] = "image/png";
	contentTypesMap[".jpg"] = "image/jpeg";
	contentTypesMap[".jpeg"] = "image/jpeg";
	contentTypesMap[".gif"] = "image/gif";
	contentTypesMap[".svg"] = "image/svg+xml";
	contentTypesMap[".ico"] = "image/x-icon";
}

std::string HttpContentTypes::getDescription( std::string extension ) {
	std::map<std::string, std::string>::iterator it = contentTypesMap.find(extension);
	if (it != contentTypesMap.end()) {
		return (it->second);
	} else {
		return ("");
	}
}

// Initializing the global object
HttpContentTypes httpContentTypes;
