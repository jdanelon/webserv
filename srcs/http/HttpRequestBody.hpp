
#ifndef HTTPREQUESTBODY_HPP
#define HTTPREQUESTBODY_HPP

#include <fstream>
#include <string>
#include <iostream>

enum LogLevel {
    INFO,
    WARNING,
    ERROR
};

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
	static bool debugEnabled;
	static const std::string className; 

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

	static void debug(LogLevel level, const std::string& message);
};

# endif