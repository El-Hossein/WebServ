#include "Request.hpp"

Request::Request(const std::string& buffer) : full_request(buffer) {
}

Request::~Request() {
}

const std::string	Request::getBody() const {
	return this->body;
}

PairedVectorSS	Request::getHeaders() const {
	return this->headers;
}

void	Request::ParseRequest()
{
    std::string line;
    std::string body;
	std::istringstream stream(full_request);
    bool isBody = false;

    // Parse the request lines
    if (std::getline(stream, line))
    {
		std::istringstream requestLine(line);
        std::string method, path, protocol;
        requestLine >> method >> path >> protocol;
        headers.push_back(std::make_pair("Method", method));
        headers.push_back(std::make_pair("Path", path));
        headers.push_back(std::make_pair("Protocol", protocol));
    }

    // Parse the headers and body
    while (std::getline(stream, line))
    {
		if (line == "\r")
        {
			isBody = true;
            continue;
        }
        if (isBody)
			body += line + "\n";
        else
        {
			size_t pos = line.find(": ");
            if (pos != std::string::npos)
            {
				std::string headerName = line.substr(0, pos);
                std::string headerValue = line.substr(pos + 2);
                headers.push_back(std::make_pair(headerName, headerValue));
            }
        }
    }
    headers.push_back(std::make_pair("body", body));
}

void	Request::SetUpRequest()
{
	ParseRequest();

	// Print the headers and body
	for (PairedVectorSS::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		std::cout << it->first << ": -----> " << it->second << std::endl;
	}
	std::cout  << "-----------------------------------------------------" << std::endl;
}