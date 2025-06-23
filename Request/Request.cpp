#include "Request.hpp"

									;
Request::Request(const int	&fd, std::vector<ConfigNode> _ConfigPars) :	ClientFd(fd),
																		Servers(_ConfigPars)
{
}

Request::~Request() {
}

/*	|#----------------------------------#|
	|#			 	GETTERS    			#|
	|#----------------------------------#|
*/

PairedVectorSS	Request::GetHeaders() const
{
	return this->Headers;
}

std::string	Request::GetHeaderValue(std::string	key) const {
	for (PairedVectorSS::const_iterator it = Headers.begin(); it != Headers.end(); it++)
	{
		if (key == it->first)
			return it->second;
	}
	return NULL;
}

/*	|#----------------------------------#|
	|#			 	SETTERS    			#|
	|#----------------------------------#|
*/

void	Request::SetHeaderValue(std::string	key, std::string NewValue)
{
	for (PairedVectorSS::iterator it = Headers.begin(); it != Headers.end(); it++)
	{
		if (key == it->first)
			return it->second = NewValue, (void)0;
	}
}

/*	|#----------------------------------#|
	|#			MEMBER FUNCTIONS    	#|
	|#----------------------------------#|
*/

void	Request::HandleQuery()
{
	size_t pos = 0;
	std::string	Query = GetHeaderValue("Query");

	// ---------		Decode Query 	 	--------- //
    while((pos = Query.find("%", pos)) != std::string::npos)
	{
		if (pos + 2 < Query.size() && IsHexa(Query[pos + 1]) && IsHexa(Query[pos + 2]))
		{
			Query.replace(pos, 3, HexaToChar(Query.substr(pos + 1, 2))); // 2 letters after '%
			pos += 1; // 1 -> size d charachter
		}
		else
			throw "Error: % in the Query";
    }
	SetHeaderValue("Query", Query);

	// ---------	Split && Save Query Params	 	--------- //

	std::istringstream	stream(Query);
	std::string			tmp, key, value;
	
	while (std::getline(stream, tmp, '&'))
	{
		pos = tmp.find("=");
		if (pos == std::string::npos)
			continue;
		key = tmp.substr(0, pos);
		value = tmp.substr(pos + 1);
		if (key.empty() || value.empty())
			continue;
		QueryParams.push_back(make_pair(key, value));
	}
}

void	Request::SplitURI()
{
	std::string	Path,Query;
	std::string	URI = GetHeaderValue("URI");
	size_t		pos = URI.find("?");

	if (pos == std::string::npos) // Query has not been found
	{
		Path = URI, Query = "", (void)0;
	}
	else
	{
		Path = URI.substr(0, pos);
		Query = URI.substr(pos + 1);
	}

	this->Headers.push_back(std::make_pair("Path", Path));
	this->Headers.push_back(std::make_pair("Query", Query));

	// ---------# Parse Path #--------- //
	for (size_t	i = 0, j = 0; i < Path.length(); i++)
	{
		if (!isalnum(URI[i]) && URI[i] != '/' && URI[i] != '.'
			&& URI[i] != '-' && URI[i] != '_' && URI[i] != '?')
				throw "400 Bad Request 1";

		if (URI[i] == '?')
			(++j >= 2) ? throw "400 Bad Request 2" : (void)0;
	}
}

void	Request::ParseURI(std::string	&URI)
{
	if (URI.length() > 2048)
		throw "414 Request-URI too long";

	Headers.push_back(std::make_pair("URI", URI));
	SplitURI();
	HandleQuery();
}

void	Request::ReadFirstLine(std::string	FirstLine)
{
	std::string			line;
	std::istringstream	FirstLineStream(FirstLine); // to use getline

	std::getline(FirstLineStream, line);

	std::istringstream Attributes(line);
	std::string method, URI, protocol;
	Attributes >> method >> URI >> protocol;

	if (method != "GET" && method != "POST" && method != "DELETE")
		throw std::runtime_error("Invalide request method.");
	Headers.push_back(std::make_pair("Method", method));

	ParseURI(URI);

	if (protocol != "HTTP/1.1")
		throw std::runtime_error("Invalide request protocol.");
	Headers.push_back(std::make_pair("Protocol", protocol));
}

void	Request::ReadHeaders(std::string Header)
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

	ReadFirstLine(StdBuffer); // First line
	ReadHeaders(StdBuffer); // other lines
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
			if (it->first == "Transfer-Encoding" && it->second != "chunked")
				throw "501 Not implemented";
		}
		if (flag != 2)	throw std::runtime_error("400");
	}
}

void	Request::SetUpRequest()
{
	ReadRequestHeader();
	CheckRequiredHeaders();

	ConfigNode RightServer = ConfigNode::GetServer(Servers, "myserver.com");
    // RightServer.print();
    // std::vector<std::string> e = ConfigNode::getValuesForKey(RightServer, "allow_methods", "NULL");
    std::vector<std::string> e = ConfigNode::getValuesForKey(RightServer, "servernames", "NULL");
    if (!e.empty())
        for (std::vector<std::string>::iterator it = e.begin(); it != e.end(); it++)
            std::cout << *it << "\n";

	PrintHeaders(this->Headers);
}
