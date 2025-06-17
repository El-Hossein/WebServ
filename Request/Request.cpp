#include "Request.hpp"

Request::Request(const int	&fd) : ClientFd(fd) {
}

Request::~Request() {
}

/*	|#----------------------------------#|
	|#			 	GETTERS    			#|
	|#----------------------------------#|
*/

std::string		Request::getFullRequest() const
{
	return this->FullRequest;
}

PairedVectorSS	Request::getHeaders() const {
	return this->Headers;
}

/*	|#----------------------------------#|
	|#			MEMBER FUNCTIONS    	#|
	|#----------------------------------#|
*/

void	Request::ReadRequest()
{
	int bytes_read = read(ClientFd, buffer, BUFFER_SIZE - 1);

	if (bytes_read <= 0)
		return ;

	buffer[bytes_read] = '\0';

	std::cout << "#------>" <<  buffer << std::endl;
}

void	Request::ParseRequest()
{
    std::string line;
    std::string body;
	std::istringstream stream(FullRequest);
    bool isBody = false;

    // Parse the request lines
    if (std::getline(stream, line))
    {
		std::istringstream requestLine(line);
        std::string method, path, protocol;
        requestLine >> method >> path >> protocol;
        Headers.push_back(std::make_pair("Method", method));
        Headers.push_back(std::make_pair("Path", path));
        Headers.push_back(std::make_pair("Protocol", protocol));
    }

    // Parse the Headers and body
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
                Headers.push_back(std::make_pair(headerName, headerValue));
            }
        }
    }
    Headers.push_back(std::make_pair("body", body));
}

void	Request::SetUpRequest()
{
	ParseRequest();

	// Print the Headers and body
	for (PairedVectorSS::const_iterator it = Headers.begin(); it != Headers.end(); ++it)
	{
		std::cout << it->first << ": -----> " << it->second << std::endl;
	}
	std::cout  << "-----------------------------------------------------" << std::endl;
}