#ifndef HTTPREQUESTBODY_HPP
#define HTTPREQUESTBODY_HPP

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <ctime>

#include "../utils.hpp"

enum LogLevel {
    INFO,
    WARNING,
    ERROR
};

enum State {
	START, // Start boundary not found
	FILE_INFO, // Start boundary found, file info not complete
	PART, // File info complete, part not complete
	TAIL, // Part is complete, but tail is not complete
	COMPLETE // Part and tail are 
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
	State			state;
	std::string		fileName;
	std::string		fieldName;
	std::string		upload_store;
	int 			remaining_data;
	std::string		fullChunkedBody;

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

	void setUploadStore(const std::string& upload_store);

	static void debug(LogLevel level, const std::string& message);

	void parseContentDisposition(const std::string& content_disposition);

	void parseHeaders(const std::string& header);

	State getState();

	std::string getFullChunkedBody();
};

# endif
