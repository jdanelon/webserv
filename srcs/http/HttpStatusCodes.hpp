#ifndef HTTP_STATUS_CODE_HPP
#define HTTP_STATUS_CODE_HPP

#include <string>
#include <map>

class HttpStatusCodes {
public:
	struct Code {
		int code;
		std::string description;
		Code(int v, const std::string& d) : code(v), description(d) {}
	};

	Code OK;
	Code BadRequest;
	Code NotFound;
	Code Created;
	Code Accepted;
	Code NoContent;
	Code MovedPermanently;
	Code Found;
	Code Unauthorized;
	Code Forbidden;
	Code MethodNotAllowed;
	Code RequestTimeout;
	Code Gone;
	Code LengthRequired;
	Code PayloadTooLarge;
	Code URITooLong;
	Code UnsupportedMediaType;
	Code TooManyRequests;
	Code InternalServerError;
	Code NotImplemented;
	Code HTTPVersionNotSupported;
	Code Continue;

	std::string getDescription(int code);

	HttpStatusCodes();
private:
	std::map<int, std::string> statusCodeMap;
};

// Global object, available for the entire project
extern HttpStatusCodes httpStatusCodes;

#endif // HTTP_STATUS_CODE_HPP
