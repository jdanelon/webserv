#include "HttpRequestBody.hpp"

HttpRequestBody::HttpRequestBody(const std::string& boundary) : 
	boundary(boundary), 
	tempFileName("./temp_file1.txt"),
	processingInProgress(false)
{}

HttpRequestBody::HttpRequestBody() : tempFileName("./temp_file1.txt"), processingInProgress(false) {
    std::cout << "Default constructor called" << std::endl;
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
    // std::cout << "parseMultipartBody" << std::endl;
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
    std::cout << "processCompletePart" << std::endl;
    if (!tempFile.is_open()) {
        std:: cout << "Opening temp file" << std::endl;
        tempFile.open(tempFileName.c_str(), std::ios::out | std::ios::app);
    }
    if (tempFile.is_open()) {
        if (tempFile.good()) {
            std::cout << "Writing to temp file" << std::endl;
            std::cout << partial_body << std::endl;
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
        std::cout << "Found end boundary" << std::endl;
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

