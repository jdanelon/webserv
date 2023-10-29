#include "HttpRequestBody.hpp"

#include <chrono>
#include <sstream>

#include <ctime>
#include <sstream>

bool HttpRequestBody::debugEnabled = true;
const std::string HttpRequestBody::className = "HttpRequestBody";

std::string generateUniqueFilename()
{
    std::stringstream ss;
    ss << "tempfile_";
    time_t now = time(NULL);
    ss << now;
    ss << ".tmp";
    return ss.str();
}

void HttpRequestBody::parseContentDisposition(const std::string &content_disposition)
{
    size_t pos = 0;
    size_t end_pos;
    while ((pos = content_disposition.find("=", pos)) != std::string::npos)
    {
        std::string key = content_disposition.substr(0, pos);
        pos++; // Skip '='
        if (content_disposition[pos] == '\"')
        {
            pos++; // Skip '\"'
        }
        end_pos = content_disposition.find("\"", pos);
        if (end_pos == std::string::npos)
        {
            end_pos = content_disposition.length();
        }

        std::string value = content_disposition.substr(pos, end_pos - pos);

        if (key.find("name") != std::string::npos)
        {
            fieldName = value;
        }
        else if (key.find("filename") != std::string::npos)
        {
            fileName = value;
        }

        pos = end_pos + 1;
    }
}

void HttpRequestBody::parseHeaders(const std::string &headers)
{
    std::stringstream ss(headers);
    std::string line;
    while (std::getline(ss, line))
    {
        if (line.find("Content-Disposition") != std::string::npos)
        {
            parseContentDisposition(line);
        }
    }
}

HttpRequestBody::HttpRequestBody(const std::string &boundary) : boundary(boundary),
                                                                processingInProgress(false)
{
    debug(WARNING, "Default constructor called - with boundary ");
    tempFileName = "./tmp/" + generateUniqueFilename();
    state = START;
}

HttpRequestBody::HttpRequestBody() : processingInProgress(false)
{
    debug(WARNING, "Default constructor called - No boundary set");
    tempFileName = "./tmp/" + generateUniqueFilename();
    state = START;
}

HttpRequestBody::~HttpRequestBody()
{
    debug(WARNING, "Destructor called");
    tempFile.close();
}

HttpRequestBody &HttpRequestBody::operator=(const HttpRequestBody &other)
{
    if (this != &other)
    {
        this->boundary = other.boundary;
        this->tailBuffer = other.tailBuffer;
        this->tempFileName = other.tempFileName;
        this->processingInProgress = other.processingInProgress;
        this->partialBuffer = other.partialBuffer;
        this->state = other.state;
    }
    return *this;
}

void HttpRequestBody::appendPartialBody(const std::string &partial_body)
{
    std::cout << partial_body << std::endl;
}

void HttpRequestBody::setBoundary(const std::string &boundary)
{
    this->boundary = boundary;
}

void HttpRequestBody::parseMultipartBody(const std::string &partial_body)
{
    debug(INFO, "parseMultipartBody");
    debug(INFO, partial_body);

    partialBuffer += partial_body;

    // Check if we have received a boundary marker
    size_t pos = partialBuffer.find(boundary);
    if (state == START && pos != std::string::npos)
    {
        debug(INFO, "Found boundary marker");
        // A boundary has been received; switch to processing mode
        processingInProgress = true;
        state = FILE_INFO;

        std::string leftover = partialBuffer.substr(pos + boundary.length());
        partialBuffer = leftover;
    }
    if (state == FILE_INFO)
    {
        debug(INFO, "In FILE_INFO state");

        // Check if all headers are found by looking for an empty line
        size_t headers_end_pos = partialBuffer.find("\r\n\r\n");
        if (headers_end_pos != std::string::npos)
        {
            // Extract the headers string
            std::string headers_str = partialBuffer.substr(0, headers_end_pos);

            // Parse all headers
            parseHeaders(headers_str); 

            // Set the next state
            state = PART;

            // Remove parsed headers and empty line from partialBuffer
            std::string leftover = partialBuffer.substr(headers_end_pos + 4); // 4 = length of "\r\n\r\n"
            processCompletePart(leftover);
        }
    }
    else if (state == PART)
    {
        // We are in the midst of processing a part, go directly to processing function
        processCompletePart(partial_body);
    }
    else
    {
        // No boundary marker yet; keep accumulating data in partialBuffer
    }
}

void HttpRequestBody::processCompletePart(const std::string &partial_body)
{
    std::string end_boundary = "--" + boundary + "--";

    // Add partial_body to tailBuffer
    tailBuffer += partial_body;

    // Keep only the last N bytes in tailBuffer, where N is the boundary length plus 2
    size_t tail_length = end_boundary.length();
    if (tailBuffer.length() > tail_length)
    {
        // Write to the temp file immediately, but keep the last N bytes in memory
        size_t pos = tailBuffer.find(end_boundary);
        std::string write_data;
        if (pos != std::string::npos) {
            // if found, we set the write data to be everything before the end boundary
            write_data = tailBuffer.substr(0, pos);
        }
        else {
            // if not found, we set the write data to be everything except the last N bytes
            write_data = tailBuffer.substr(0, tailBuffer.length() - tail_length);
            tailBuffer = tailBuffer.substr(tailBuffer.length() - tail_length);
        }

        if (!tempFile.is_open())
        {
            debug(INFO, "Opening temp file");
            debug(INFO, "Upload Store: " + upload_store);
            tempFile.open(tempFileName.c_str(), std::ios::out | std::ios::app);
        }

        if (tempFile.is_open() && tempFile.good())
        {
            debug(INFO, "Writing to temp file");
            tempFile << write_data;
        }
        else if (!tempFile.good())
        {
            std::cerr << "An error occurred while writing to the file." << std::endl;
            std::cout << "Error state: " << tempFile.rdstate() << std::endl;
        }
    }

    // Check for end boundary in tailBuffer
    if (tailBuffer.find(end_boundary) != std::string::npos)
    {
        debug(INFO, "Found end boundary");

        // Finish up
        processingInProgress = false;
        isProcessingComplete = true;
        state = COMPLETE;

        // Close temp file, move it to its final location, and reset any flags
        if (tempFile.is_open())
        {
            tempFile.close();
        }
    }
}

void HttpRequestBody::parseChunkedBody(const std::string &partial_body)
{
    std::cout << partial_body << std::endl;
}

bool HttpRequestBody::getIsProcessingComplete()
{
    return isProcessingComplete;
}

void HttpRequestBody::setUploadStore(const std::string &upload_store)
{
    this->upload_store = upload_store;
}

void HttpRequestBody::debug(LogLevel level, const std::string &message)
{
    if (!debugEnabled)
    {
        return;
    }

    std::string prefix;
    std::string colorCode;
    switch (level)
    {
    case INFO:
        prefix = "[INFO] ";
        colorCode = "\033[1;34m"; // Blue
        break;
    case WARNING:
        prefix = "[WARNING] ";
        colorCode = "\033[1;33m"; // Yellow
        break;
    case ERROR:
        prefix = "[ERROR] ";
        colorCode = "\033[1;31m"; // Red
        break;
    }

    std::cout << colorCode << className << " " << prefix << message << "\033[0m" << std::endl; // \033[0m resets the color
}