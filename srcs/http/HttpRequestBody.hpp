
#ifndef HTTPREQUESTBODY_HPP
#define HTTPREQUESTBODY_HPP

#include <fstream>
#include <string>
#include <iostream>

class HttpRequestBody {

private:
	std::string 	boundary;
	std::string 	partialBuffer;
	std::string 	tailBuffer;
	std::ofstream 	tempFile;
	std::string 	tempFileName;
	bool 			processingInProgress;
	bool			isProcessingComplete;

public:
	HttpRequestBody(const std::string& boundary);

	HttpRequestBody();

    ~HttpRequestBody();

	HttpRequestBody& operator=(const HttpRequestBody& other);

	void parseMultipartBody(const std::string& partial_body);

	void parseChunkedBody(const std::string& partial_body);

    void appendPartialBody(const std::string& partial_body);

    void processCompletePart(const std::string& complete_part);

    bool getIsProcessingComplete();

	void setBoundary(const std::string& boundary);
};


# endif