#include "Request.hpp"

Request::Request(const int	&fd) :	ClientFd(fd)
{
}

Request::~Request() {
}

/*	|#----------------------------------#|
	|#			 	GETTERS    			#|
	|#----------------------------------#|
*/

PairedVectorSS	Request::getHeaders() const {
	return this->Headers;
}

/*	|#----------------------------------#|
	|#			MEMBER FUNCTIONS    	#|
	|#----------------------------------#|
*/

void	Request::ParseFirstLine(std::string	FirstLine)
{
	std::string			line;
	std::istringstream	FirstLineStream(FirstLine); // to use getline

	std::getline(FirstLineStream, line);

	std::istringstream Attributes(line);
	std::string method, path, protocol;
	Attributes >> method >> path >> protocol;

	if (method != "GET" && method != "POST" && method != "DELETE")
		throw std::runtime_error("Invalide request method.");
	if (protocol != "HTTP/1.1")
		throw std::runtime_error("Invalide request protocol.");

	Headers.push_back(std::make_pair("Method", method));
	Headers.push_back(std::make_pair("Path", path));
	Headers.push_back(std::make_pair("Protocol", protocol));
}

void	Request::ParseRequestHeader(std::string Header)
{
	std::string			line;
	std::istringstream	stream(Header); // to use getline

	// Parse the Headers
	while (std::getline(stream, line))
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

void	Request::ReadRequestHeader()
{
	int			BytesRead = 0;
	static	int	MaxHeaderSize = 0;
	static bool	FirstLine = false;
	char		buffer[BUFFER_SIZE];

	while (true)
	{
		BytesRead = read(ClientFd, buffer, BUFFER_SIZE - 1);
		if (BytesRead < 0)
			throw std::runtime_error("Negative return read.");
		buffer[BytesRead] = '\0';

		MaxHeaderSize += BytesRead;
		if (MaxHeaderSize > MAX_HEADER_SIZE)
			throw std::runtime_error("Invalide request header lengh.");

		if (!FirstLine)
		{
			ParseFirstLine(buffer); // First line
			FirstLine = true;
		}
		ParseRequestHeader(buffer); // other lines

		std::string	tmp(buffer);
		if (tmp.find("\r\n\r\n")) // ila salit lHeader nkhroj -> Possible error incase [...\r\n\r] [\n...]
			break;
	}
	/*
		check if all required headers exist
	*/
}

inline void	PrintHeaders(PairedVectorSS Headers)
{
	for (PairedVectorSS::const_iterator it = Headers.begin(); it != Headers.end(); ++it)
	{
		std::cout << it->first << ": -----> " << it->second << std::endl;
	}
	std::cout  << "-----------------------------------------------------" << std::endl;
}

void	Request::SetUpRequest()
{
	ReadRequestHeader();


	PrintHeaders(this->Headers);
}
