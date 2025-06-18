#include "Request.hpp"

Request::Request(const int	&fd) :	ClientFd(fd),
									FullRequest("")
{
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

void	Request::ReadRequestHeader()
{
	char		buffer[MAX_HEADER_SIZE];
	int			BytesRead = 0;
	static	int	MaxHeaderSize = 0;
	
	while (true)
	{
		BytesRead = read(ClientFd, buffer, BUFFER_SIZE - 1);
		if (BytesRead < 0)
			throw std::runtime_error("Negative return read.");
		buffer[BytesRead] = '\0';

		MaxHeaderSize += BytesRead;
		if (MaxHeaderSize > MAX_HEADER_SIZE)
			throw std::runtime_error("Invalide request header lengh.");

		/*
			RetrieveRequestFromBuffer
			Parse the buffer here.
		*/
	}
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
	ReadRequestHeader();
	ParseRequest();

	// Print the Headers and body
	for (PairedVectorSS::const_iterator it = Headers.begin(); it != Headers.end(); ++it)
	{
		std::cout << it->first << ": -----> " << it->second << std::endl;
	}
	std::cout  << "-----------------------------------------------------" << std::endl;
}