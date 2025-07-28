#include "Request.hpp"

Request::Request(const int	&fd, ClientStatus Status, std::vector<ConfigNode> _ConfigPars, int &_RealPort)
						:	ClientFd(fd),
							Client(ReadHeader),
							DataType(FixedLength),
							ContentType(Other),
							Servers(_ConfigPars),
							HeaderBuffer(""),
							ContentLength(0),
							TotalBytesRead(0),
							KeepAlive(false),
							RequestNotComplete(true),
							zbi("")
{
	ServerDetails.IsPortExist = false;
	ServerDetails.RealPort = _RealPort;
}

Request::~Request() {
}
//done:
int	Request::GetClientFd() const
{
	return this->ClientFd;
}

int		Request::GetClientStatus() const
{
	return this->Client;
}
void	Request::SetClientStatus(ClientStatus	Status) 
{
	this->Client = Status;
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
	return "";
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

int			Request::GetDataType() const
{
	return this->DataType;
}

_BoundarySettings	Request::GetBoundarySettings() const
{
	return this->BoundaryAttri;
}

int			Request::GetContentType() const
{
	return this->ContentType;
}

size_t				Request::GetTotatlBytesRead() const
{
	return this->TotalBytesRead;
}

_ServerDetails	Request::GetServerDetails() const
{
	return this->ServerDetails;
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

void	Request::SetServerDetails()
{
	std::string	&Host = Headers["host"];

	size_t		Pos = Host.find(":");
	if (Pos == std::string::npos)
		return ;
	
	ServerDetails.IsPortExist = true;
	ServerDetails.ServerHost = Host.substr(0, Pos);
	ServerDetails.ServerPort = Host.substr(Pos + 1);

	// std::cout << "{" << ServerDetails.ServerHost << "}" << std::endl;
	// std::cout << "{" << ServerDetails.ServerPort << "}\n\n" << std::endl;
}

/*	|#----------------------------------#|
	|#			MEMBER FUNCTIONS    	#|
	|#----------------------------------#|
*/

void	Request::GetBoundaryFromHeader()
{
	std::string										Boundary;
	std::map<std::string, std::string>::iterator	it = Headers.find("content-type");

	size_t	pos = it->second.find("=");
	if (pos == std::string::npos)
		throw "400 Bad Request -ParseBoundary()";

	Boundary = it->second.substr(pos + 1); // setting Boundary
	if (Boundary.length() < 1 || Boundary.length() > 70 || !ValidBoundary(Boundary))
		throw "400 Bad Request -ParseBoundary() -Boundary Parsing.";

	BoundaryAttri.Boundary = Boundary;
	BoundaryAttri.BoundaryStart = "\r\n--" + Boundary;
	BoundaryAttri.BoundaryEnd = "\r\n--" + Boundary + "--";
}

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
	// std::cout << "--->Query Params:" << std::endl; PrintHeaders(QueryParams);
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

void	Request::ParseURI()
{
	std::string URI = Headers["uri"];
	if (URI.length() > 2048)
		throw "414 Request-URI too long";

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

	Headers["uri"] = URI;

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

void	Request::PostRequiredHeaders()
{
	if (Headers.find("content-length") == Headers.end() && Headers.find("transfer-encoding") == Headers.end())
		PrintError("Missing POST required headers"), throw "400 Bad request";

	if (Headers.find("content-length") != Headers.end())
	{
		if (!ValidContentLength(Headers["content-length"]))
			PrintError("Invalide Content-Length"), throw "400: Bad Request";
		SetContentLength(strtod(Headers["content-length"].c_str(), NULL));
		this->DataType = FixedLength;
	}
	
	if (Headers.find("transfer-encoding") != Headers.end())
		(Headers["transfer-encoding"] == "chunked") ? DataType = Chunked : throw "501 Not implemented - PostRequestHeaders()";
	
	if (Headers.find("content-type") != Headers.end())
	{
		if (Headers["content-type"].find("multipart/form-data") != std::string::npos)
		{
			this->ContentType = Boundary;
			GetBoundaryFromHeader();
		}
	}
}

void	Request::ParseHeaders()
{

	if (Headers.find("host") == Headers.end())
		PrintError("No Host has been found!"), throw "400 Bad request";
	if (Headers.find("connection") != Headers.end())
		(Headers.find("connection")->second == "close") ? KeepAlive = false : KeepAlive = true;
	if (Method == POST)
	{
		PostRequiredHeaders();
	}
	SetServerDetails(); // Init localhost + port
	RightServer = ConfigNode::GetServer(Servers, ServerDetails); // send {IsPortExist, ServerHost, ServerPort, RealPort}
	ParseURI();
}

void Request::ReadBodyChunk()
{
    int		BytesRead = 0;
    char	buffer[BUFFER_SIZE];

	std::memset(buffer, 0, BUFFER_SIZE);
    BytesRead = read(ClientFd, buffer, BUFFER_SIZE - 1);
    if (BytesRead < 0)
        throw ("Error: Read failed.");
    if (BytesRead == 0)
	{
		Client = EndReading;
		return ;
	}
	TotalBytesRead += BytesRead;
	BodyUnprocessedBuffer.assign(buffer, BytesRead);

	if (TotalBytesRead == ContentLength)
		Client = EndReading;

	zbi += buffer;
}

void	Request::ReadRequestHeader()
{
	int			BytesRead = 0;
	char		buffer[MAX_HEADER_SIZE];

	std::memset(buffer, 0, MAX_HEADER_SIZE);
	BytesRead = read(ClientFd, buffer, MAX_HEADER_SIZE - 1);
	if (BytesRead < 0)
		throw ("Error: Read return.");
	if (BytesRead == 0)
		Client = EndReading;

	HeaderBuffer.append(buffer, BytesRead);

	zbi += HeaderBuffer;

	size_t npos = HeaderBuffer.find("\r\n\r\n");
	if (npos == std::string::npos)
		return ;
	else
		RequestNotComplete = false, Client = ReadBody;

	BodyUnprocessedBuffer.assign(HeaderBuffer.substr(npos + 2));
	HeaderBuffer				= HeaderBuffer.substr(0, npos);
	if (HeaderBuffer.size() >= 8192) // 8kb
		PrintError("Header too long."), throw "400 Bad Request";

	if (BodyUnprocessedBuffer.size() == 2)
		Client = EndReading;
	TotalBytesRead += BodyUnprocessedBuffer.size() - 2; // (- 2) For CRLF

	ReadFirstLine(HeaderBuffer); // First line
	ReadHeaders(HeaderBuffer); // other lines
	ParseHeaders();
}

void	Request::SetUpRequest()
{
	switch (Client)
	{
		case	ReadHeader	:	ReadRequestHeader();	break ;
		case	ReadBody	:	ReadBodyChunk();		break ;
		case	EndReading	:	break ;
	}

	if (Client == EndReading)
	{
		if (RequestNotComplete == true)
			PrintError("Something is missing."), throw "404 Bad Request";

		// std::ofstream File; File.open("ZRawRequest.txt"), File << zbi ;
	}
	if (Method == POST)
	{
		static	Post	PostObj(*this);

		PostObj.HandlePost();
	}
}

// std::vector<std::string> e = ConfigNode::getValuesForKey(RightServer, "allow_methods", "/");

