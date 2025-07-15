#include "Request.hpp"

									;
Request::Request(const int	&fd, std::vector<ConfigNode> _ConfigPars) :	ClientFd(fd),
																		Servers(_ConfigPars),
																		ContentLength(0),
																		KeepAlive(false)
{
}

Request::~Request() {
}
//done:
int	Request::GetClientFd() const
{
	return this->ClientFd;
}

int	Request::GetNew() const
{
	return this->NewClient;
}
void	Request::SetNew(int is) 
{
	this->NewClient = is;
}
std::string	Request::GetHeaderBuffer() const
{
	return this->HeaderBuffer;
}

/*	|#----------------------------------#|
	|#			 	GETTERS    			#|
	|#----------------------------------#|
*/

std::map<std::string, std::string>	Request::GetHeaders() const
{
	return this->Headers;
}

std::string	Request::GetHeaderValue(std::string	key) const {
	for (std::map<std::string, std::string>::const_iterator it = Headers.begin(); it != Headers.end(); it++)
	{
		if (key == it->first)
			return it->second;
	}
	throw "";
}

std::map<std::string, std::string>	Request::GetQueryParams() const
{
	return this->QueryParams;
}

std::vector<std::string>	Request::GetPathParts() const
{
	return this->PathParts;
}

bool	Request::GetConnection() const
{
	return this->KeepAlive;
}

std::string	Request::GetFullPath() const
{
	return FullSystemPath;
}

std::string	Request::GetUnprocessedBuffer() const
{
	return this->BodyUnprocessedBuffer;
}

size_t		Request::GetContentLength() const
{
	return this->ContentLength;
}

ConfigNode	&Request::GetRightServer()
{
	return this->RightServer;
}

/*	|#----------------------------------#|
	|#			 	SETTERS    			#|
	|#----------------------------------#|
*/

void	Request::SetHeaderValue(std::string	key, std::string NewValue)
{
	for (std::map<std::string, std::string>::iterator it = Headers.begin(); it != Headers.end(); it++)
	{
		if (key == it->first)
			return it->second = NewValue, (void)0;
	}
}

void	Request::SetContentLength(const size_t	Length)
{
	this->ContentLength = Length;
}

/*	|#----------------------------------#|
	|#			MEMBER FUNCTIONS    	#|
	|#----------------------------------#|
*/

void	Request::HandleQuery()
{
	size_t pos = 0;
	std::string	Query = GetHeaderValue("query");

	// ---------		Decode Query 	 	--------- //
	DecodeHexaToChar(Query);
	for (size_t	i = 0; i < Query.length(); i++) // replacing each '+' with ' '
		if (Query[i] == '+')	Query[i] = ' ';
	SetHeaderValue("query", Query);

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
		QueryParams.insert(make_pair(key, value));
	}
	std::cout << "--->Query Params:" << std::endl; PrintHeaders(QueryParams);
}

void   Request::HandlePath()
{
	std::string					UriPath = GetHeaderValue("path");
	std::vector<std::string>	ConfigPath = ConfigNode::getValuesForKey(RightServer, "root", "NULL");
	if (ConfigPath.empty())
		return ;
	this->FullSystemPath = ConfigPath[0] + UriPath;

	std::istringstream	stream(FullSystemPath);
	std::string			part;

	FullSystemPath.clear();
	while (std::getline(stream, part, '/'))
	{
		if (part == "..")
			throw ("403 Forbidden");
		else if (!part.empty() && part != ".")
		{
			PathParts.push_back(part);
			this->FullSystemPath += "/";
			this->FullSystemPath += part;
		}
	}
}

void	Request::SplitURI()
{
	std::string	Path,Query;
	std::string	URI = GetHeaderValue("uri");
	
	// ---------# Parse URI #--------- //
	std::string URICharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%";
	for (size_t i = 0; i < URI.size() ; i++)
	{
		if (URICharacters.find(URI[i]) == std::string::npos)
			throw "400 Bad request";
	}
	size_t		pos = URI.find("?");
	if (pos == std::string::npos) // Query has not been found
		Path = URI, Query = "";
	else
	{
		Path = URI.substr(0, pos);
		Query = URI.substr(pos + 1);
	}

	Headers["path"] = Path;
	Headers["query"] = Query;

}

void	Request::ParseURI(std::string	&URI)
{
	if (URI.length() > 2048)
		throw "414 Request-URI too long";

	Headers["uri"] = URI;
	SplitURI();
	HandlePath();
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

	if		(method == "GET")		this->Method = GET;
	else if (method == "POST")		this->Method = POST;
	else if (method == "DELETE")	this->Method = DELETE;	
	else							throw ("400: Invalide request method.");
		
	Headers["method"] = method;

	ParseURI(URI);

	if (protocol != "HTTP/1.1")
		throw ("505: Invalide request protocol.");
	Headers["protocol"] =  protocol;
}

void	Request::ReadHeaders(std::string Header)
{
	std::string			line, headerName, headerValue, LowKey;
	std::istringstream	stream(Header);

	while (std::getline(stream, line))
	{
		size_t pos = line.find(": ");
		if (pos == std::string::npos)
			continue ;

		headerName = line.substr(0, pos);
		if (!ValidFieldName(headerName))
			throw "400 Bad Request ReadHeaders()-1";

		headerValue = line.substr(pos + 2);
		TrimSpaces(headerValue);
		if (!ValidFieldValue(headerValue))
			throw "400 Bad Request ReadHeaders()-2";

		std::transform(headerName.begin(), headerName.end(), headerName.begin(), ::tolower);

		Headers[headerName] = headerValue;
	}
}

void	Request::CheckRequiredHeaders()
{
	int	flag = 0;

	if (Headers.find("host") == Headers.end())
	{
		PrintError("No Host has been found!"), throw "400 Bad request";
	}
	if (Headers.find("connection") != Headers.end())
	{
		(Headers.find("connection")->second == "close") ? KeepAlive = false : KeepAlive = true;
	}
}


void Request::ReadBodyChunk() {
    int BytesRead = 0;
    char buffer[MAX_HEADER_SIZE];

    BytesRead = read(ClientFd, buffer, MAX_HEADER_SIZE - 1);
    if (BytesRead < 0) {
        throw ("Error: Read failed.");
    }

    if (BytesRead == 0) {
			SetNew(END_BODY);
	}
    buffer[BytesRead] = '\0';
    std::string StdBuffer(buffer, BytesRead);
    BodyUnprocessedBuffer += StdBuffer; // Append to BodyUnprocessedBuffer
	if (BodyUnprocessedBuffer.size() >= 800)
		SetNew(END_BODY);
}

void	Request::ReadRequestHeader()
{
	int			BytesRead = 0;
	char		buffer[MAX_HEADER_SIZE];

	BytesRead = read(ClientFd, buffer, MAX_HEADER_SIZE - 1);
	if (BytesRead < 0)
		throw ("Error: Read return.");
	if (BytesRead == 0)
		SetNew(END_BODY);


	buffer[BytesRead] = '\0';

	std::string	StdBuffer(buffer);
	HeaderBuffer	+= StdBuffer;
	size_t npos = HeaderBuffer.find("\r\n\r\n");
	if (npos == std::string::npos)
		throw ("400: Invalide request header.");

	BodyUnprocessedBuffer	= HeaderBuffer.substr(npos + 4);
	HeaderBuffer				= HeaderBuffer.substr(0, npos);
	ReadFirstLine(HeaderBuffer); // First line
	ReadHeaders(HeaderBuffer); // other lines
	if (BodyUnprocessedBuffer.empty() != false)
		SetNew(END_BODY);
	else
		SetNew(READ_BODY);
}

void	Request::SetUpRequest()
{
	RightServer = ConfigNode::GetServer(Servers, "myserver.com");
	if (GetNew() ==  READ_HEADER)
	{
		ReadRequestHeader();
		CheckRequiredHeaders();
	}
	else if (GetNew() == READ_BODY)
		ReadBodyChunk();
	// std::cout << Request::GetFullPath() << std::endl;

	switch (Method)
	{
		case GET	:	{	Get		GetObj(*this);		return GetObj.CheckPath(GetFullPath()); }
		case POST	:	{	Post	PostObj(*this);		return PostObj.HandlePost();	}
		case DELETE	:	{	Delete	DeleteObj(*this);	return DeleteObj.DoDelete(GetFullPath());	}
	}

    // std::vector<std::string> e = ConfigNode::getValuesForKey(RightServer, "servernames", "NULL");

	// PrintHeaders(this->Headers);
}
