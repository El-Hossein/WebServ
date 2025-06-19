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
	char		buffer[MAX_HEADER_SIZE];

	BytesRead = read(ClientFd, buffer, MAX_HEADER_SIZE - 1);
	if (BytesRead <= 0)
		throw std::runtime_error("Error: Read return.");
	if (BytesRead >= MAX_HEADER_SIZE - 1)
		throw std::runtime_error("Invalide request header lengh.");

	buffer[BytesRead] = '\0';

	std::string	StdBuffer(buffer);
	size_t npos = StdBuffer.find("\r\n\r\n");
	if (npos == std::string::npos)
		throw std::runtime_error("Invalide request header.");

	BodyUnprocessedBuffer	= StdBuffer.substr(StdBuffer.find("\r\n\r\n"));
	StdBuffer				= StdBuffer.substr(0, StdBuffer.find("\r\n\r\n"));

	ParseFirstLine(StdBuffer); // First line
	ParseRequestHeader(StdBuffer); // other lines
}

inline void	PrintHeaders(PairedVectorSS Headers)
{
	for (PairedVectorSS::const_iterator it = Headers.begin(); it != Headers.end(); ++it)
	{
		std::cout << it->first << ": -----> " << it->second << std::endl;
	}
	std::cout  << "-----------------------------------------------------" << std::endl;
}

void	Request::CheckRequiredHeaders()
{
	int	flag = 0;
	PairedVectorSS::const_iterator it = Headers.begin();

	if (it->second == "Post")
	{
		for (PairedVectorSS::const_iterator _it = Headers.begin(); _it != Headers.end(); _it++)
		{
			if (it->first == "Content-Length" || it->first == "Transfer-Encoding")
				flag++;
		}
		if (flag != 2)	throw std::runtime_error("400");
	}
}

void	Request::SetUpRequest()
{
	ReadRequestHeader();
	CheckRequiredHeaders();



	PrintHeaders(this->Headers);
}
