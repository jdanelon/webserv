#include "HttpStatusCodes.hpp"

HttpStatusCodes::HttpStatusCodes() : 
	OK(200, "OK"), 
	BadRequest(400, "Bad Request"), 
	NotFound(404, "Not Found"), 
	Created(201, "Created"),
	Accepted(202, "Accepted"),
	NoContent(204, "No Content"),
	MovedPermanently(301, "Moved Permanently"),
	Found(302, "Found"),
	Unauthorized(401, "Unauthorized"),
	Forbidden(403, "Forbidden"),
	MethodNotAllowed(405, "Method Not Allowed"),
	RequestTimeout(408, "Request Timeout"),
	Gone(410, "Gone"),
	LengthRequired(411, "Length Required"),
	PayloadTooLarge(413, "Payload Too Large"),
	URITooLong(414, "URI Too Long"),
	UnsupportedMediaType(415, "Unsupported Media Type"),
	TooManyRequests(429, "Too Many Requests"),
	InternalServerError(500, "Internal Server Error"),
	NotImplemented(501, "Not Implemented"),
	HTTPVersionNotSupported(505, "HTTP Version Not Supported"),
	Continue(100, "Continue")
{
	statusCodeMap[OK.code] = OK.description;
	statusCodeMap[BadRequest.code] = BadRequest.description;
	statusCodeMap[NotFound.code] = NotFound.description;
	statusCodeMap[Created.code] = Created.description;
	statusCodeMap[Accepted.code] = Accepted.description;
	statusCodeMap[NoContent.code] = NoContent.description;
	statusCodeMap[MovedPermanently.code] = MovedPermanently.description;
	statusCodeMap[Found.code] = Found.description;
	statusCodeMap[Unauthorized.code] = Unauthorized.description;
	statusCodeMap[Forbidden.code] = Forbidden.description;
	statusCodeMap[MethodNotAllowed.code] = MethodNotAllowed.description;
	statusCodeMap[RequestTimeout.code] = RequestTimeout.description;
	statusCodeMap[Gone.code] = Gone.description;
	statusCodeMap[LengthRequired.code] = LengthRequired.description;
	statusCodeMap[PayloadTooLarge.code] = PayloadTooLarge.description;
	statusCodeMap[URITooLong.code] = URITooLong.description;
	statusCodeMap[UnsupportedMediaType.code] = UnsupportedMediaType.description;
	statusCodeMap[TooManyRequests.code] = TooManyRequests.description;
	statusCodeMap[InternalServerError.code] = InternalServerError.description;
	statusCodeMap[NotImplemented.code] = NotImplemented.description;
	statusCodeMap[HTTPVersionNotSupported.code] = HTTPVersionNotSupported.description;
	statusCodeMap[Continue.code] = Continue.description;
}

std::string HttpStatusCodes::getDescription(int code) {
	std::map<int, std::string>::iterator it = statusCodeMap.find(code);
	if (it != statusCodeMap.end()) {
		return it->second;
	} else {
		return "Unknown";
	}
}

// Initializing the global object
HttpStatusCodes httpStatusCodes;
