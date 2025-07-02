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
	return "NULL";
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
	for (size_t	i = 0; i < Query.length(); i++) // replacing each '+' with ' '
		if (Query[i] == '+')	Query[i] = ' ';
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
		QueryParams.insert(make_pair(key, value));
	}
	std::cout << "--->Query Params:" << std::endl; PrintHeaders(QueryParams);
}

void   Request::HandlePath()
{
	std::string					UriPath = GetHeaderValue("Path");
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

	Headers["Path"] = Path;
	Headers["Query"] = Query;

	// ---------# Parse Path #--------- //
	for (size_t	i = 0, j = 0; i < Path.length(); i++)
	{
		if (!isalnum(URI[i]) && URI[i] != '/' && URI[i] != '.'
			&& URI[i] != '-' && URI[i] != '_')
				throw "400 Bad Request 1";

		if (URI[i] == '?')
			(++j >= 2) ? throw "400 Bad Request 2" : (void)0;
	}
}

void	Request::ParseURI(std::string	&URI)
{
	if (URI.length() > 2048)
		throw "414 Request-URI too long";

	Headers["URI"] = URI;
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
	else						throw ("400: Invalide request method.");
		
	Headers["Method"] = method;

	ParseURI(URI);

	if (protocol != "HTTP/1.1")
		throw ("505: Invalide request protocol.");
	Headers["Protocol"] =  protocol;
}

void	Request::ReadHeaders(std::string Header)
{
	std::string			line, headerName, headerValue;
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
		if (!headerValue.empty() && headerValue.back() == '\r')
			headerValue.pop_back();
		if (!ValidFieldValue(headerValue))
			throw "400 Bad Request ReadHeaders()-2";

		Headers[headerName] = headerValue;
	}
}

void	Request::ReadRequestHeader()
{
	int			BytesRead = 0;
	char		buffer[MAX_HEADER_SIZE];

	BytesRead = read(ClientFd, buffer, MAX_HEADER_SIZE - 1);
	if (BytesRead <= 0)
		throw ("Error: Read return.");

	buffer[BytesRead] = '\0';

	std::string	StdBuffer(buffer);
	size_t npos = StdBuffer.find("\r\n\r\n");
	if (npos == std::string::npos)
		throw ("400: Invalide request header.");

	BodyUnprocessedBuffer	= StdBuffer.substr(npos + 4);
	StdBuffer				= StdBuffer.substr(0, npos);

	ReadFirstLine(StdBuffer); // First line
	ReadHeaders(StdBuffer); // other lines
}

void	Request::CheckRequiredHeaders()
{
	int	flag = 0;

	for (std::map<std::string, std::string>::const_iterator it = Headers.begin(); it != Headers.end(); it++)
	{
		std::string LowKey = it->first;
		std::transform(LowKey.begin(), LowKey.end(), LowKey.begin(), ::tolower);

		if (LowKey == "connection")
		{
			if (it->second != "keep-alive" && it->second != "close")
				SetHeaderValue("connection", "keep-alive"); // setting up default behaviour
			it-> second == "keep-alive" ? KeepAlive = true : KeepAlive = false;
		}
		// if (Headers.begin()->second == "POST")
		// {
			if (LowKey == "content-length")
			{
				if (!ValidContentLength(it->second))	throw "400: Bad Request";
				ContentLength = strtod(it->second.c_str(), NULL);
				flag++;
			}
			// else if (LowKey == "transfer-encoding" && it->second != "chunked")
			// 	throw "501 Not implemented";
		// }
	}
	if (Headers.begin()->second == "POST")
		if (flag == 2)	throw ("400");
}

void	Request::SetUpRequest()
{
	RightServer = ConfigNode::GetServer(Servers, "myserver.com");

	ReadRequestHeader();
	CheckRequiredHeaders();

	switch (Method)
	{
		case GET:		{	Get		GetObj(*this);		GetObj.CheckPath(GetFullPath()); }
		case POST:		{	Post	PostObj(*this);		PostObj.HandleBody();	}
		case DELETE:	{	Delete	DeleteObj(*this);	DeleteObj.DoDelete(GetFullPath());	}
	}

    std::vector<std::string> e = ConfigNode::getValuesForKey(RightServer, "servernames", "NULL");

	// PrintHeaders(this->Headers);
}
