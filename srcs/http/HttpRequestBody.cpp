#include "HttpRequestBody.hpp"

#include <chrono>
#include <sstream>

#include <ctime>
#include <sstream>

bool HttpRequestBody::debugEnabled = true;
const std::string HttpRequestBody::className = "HttpRequestBody";

std::string generateUniqueFilename() {
    std::stringstream ss;
    ss << "tempfile_";
    time_t now = time(NULL);
    ss << now;
    ss << ".tmp";
    return ss.str();
}

HttpRequestBody::HttpRequestBody(const std::string& boundary) : 
	boundary(boundary), 
	processingInProgress(false)
{
    debug(WARNING, "Default constructor called - with boundary ");
    tempFileName = "./tmp/" + generateUniqueFilename();   
}

HttpRequestBody::HttpRequestBody() : processingInProgress(false) {
    debug(WARNING, "Default constructor called - No boundary set");
    tempFileName = "./tmp/" + generateUniqueFilename();
}

HttpRequestBody::~HttpRequestBody() {
	tempFile.close();
}

HttpRequestBody& HttpRequestBody::operator=(const HttpRequestBody& other) {
	if (this != &other) {
		this->boundary = other.boundary;
		this->tailBuffer = other.tailBuffer;
		this->tempFileName = other.tempFileName;
		this->processingInProgress = other.processingInProgress;
		this->partialBuffer = other.partialBuffer;
	}
	return *this;
}

void HttpRequestBody::appendPartialBody(const std::string& partial_body) {
	std::cout << partial_body << std::endl;
}

void	HttpRequestBody::setBoundary(const std::string& boundary) {
	this->boundary = boundary;
}

void	HttpRequestBody::parseMultipartBody(const std::string& partial_body) {
    debug(INFO, "parseMultipartBody");
    partialBuffer += partial_body;

    // Check if we have received a boundary marker
    size_t pos = partialBuffer.find(boundary);
    if (pos != std::string::npos) {
        // A boundary has been received; switch to processing mode
        processingInProgress = true;

        // Pass the left-over data to process_complete_part and clear partialBuffer
        std::string leftover = partialBuffer.substr(pos + boundary.length());
        partialBuffer.clear();
        processCompletePart(leftover);
    } else if (processingInProgress) {
        // We are in the midst of processing a part, go directly to processing function
        processCompletePart(partial_body);
    } else {
        // No boundary marker yet; keep accumulating data in partialBuffer
    }
}

void HttpRequestBody::processCompletePart(const std::string& partial_body) {
    // Open the file from the file name if it is not already open
    debug(INFO, "processCompletePart");
    debug(INFO, partial_body);
    if (!tempFile.is_open()) {
        debug(INFO, "Opening temp file");
        tempFile.open(tempFileName.c_str(), std::ios::out | std::ios::app);
    }
    if (tempFile.is_open()) {
        if (tempFile.good()) {
            debug(INFO, "Writing to temp file");
            tempFile << partial_body;
        }
        if (!tempFile.good()) {
            std::cerr << "An error occurred while writing to the file." << std::endl;
            std::cout << "Error state: " << tempFile.rdstate() << std::endl;
        }
    }

    // Append the new data to tailBuffer and check for the end boundary
    tailBuffer += partial_body;

    // Keep only the last N bytes in tailBuffer, where N is the boundary length plus 2
    if (tailBuffer.length() > boundary.length() + 2) {
        tailBuffer = tailBuffer.substr(tailBuffer.length() - boundary.length() - 4);
    }

    std::string end_boundary = boundary + "--";
    if (tailBuffer.find(end_boundary) != std::string::npos) {
        debug(INFO, "Found end boundary");
        // Found the end boundary; finish up
        processingInProgress = false;
		isProcessingComplete = true;
        // Close temp file, move it to its final location, and reset any flags
        tempFile.close();
    }
}

void HttpRequestBody::parseChunkedBody(const std::string& partial_body) {
	std::cout << partial_body << std::endl;
}

bool HttpRequestBody::getIsProcessingComplete() {
	return isProcessingComplete;
}

void HttpRequestBody::debug(LogLevel level, const std::string& message) {
    if (!debugEnabled) {
        return;
    }

    std::string prefix;
    std::string colorCode;
    switch (level) {
    case INFO:
        prefix = "[INFO] ";
        colorCode = "\033[1;34m";  // Blue
        break;
    case WARNING:
        prefix = "[WARNING] ";
        colorCode = "\033[1;33m";  // Yellow
        break;
    case ERROR:
        prefix = "[ERROR] ";
        colorCode = "\033[1;31m";  // Red
        break;
    }

    std::cout << colorCode << className << " " << prefix << message << "\033[0m" << std::endl;  // \033[0m resets the color
}