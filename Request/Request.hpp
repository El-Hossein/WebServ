#pragma once

#include "Post.hpp"
#include "../allincludes.hpp"
#include "../pars_config/config.hpp"

#define BUFFER_SIZE		1000
#define MAX_HEADER_SIZE	100

enum	Method
{
	GET,
	POST,
	DELETE
};

enum	ClientStatus
{
	ReadHeader,
	ReadBody,
	EndReading
};	

enum	DataType
{
	Chunked,
	FixedLength
};

enum	ContentType
{
	Boundary,
	Binary,
	Raw,
	Other
};

struct	BoundarySettings
{
	std::string	Boundary;
	std::string	BoundaryStart;
	std::string	BoundaryEnd;
};

class	Request
{
private:
	std::string zbi;


	ClientStatus			Client;
	int						ClientFd;
	std::vector<ConfigNode>	Servers;
	ConfigNode				RightServer;

	Method								Method;
	DataType							DataType;
	ContentType							ContentType;
	std::map<std::string, std::string>	Headers;
	std::map<std::string, std::string>	QueryParams;
	std::string							FullSystemPath;
	std::vector<std::string>			PathParts;

	BoundarySettings			BoundaryAttri;
	std::string					HeaderBuffer;
	std::string					BodyUnprocessedBuffer;
	size_t						TotalBytesRead;
	size_t						ContentLength;
	bool						KeepAlive;
	bool						RequestNotComplete;

public:
	Request(const int	&, ClientStatus, std::vector<ConfigNode>);
	~Request();
	// ---------		GETTERS 	 	--------- //
	bool								GetConnection() const;
	int									GetClientStatus() const;
	int									GetClientFd() const;
	int									GetDataType() const;
	int									GetContentType() const;
	size_t								GetContentLength() const;
	size_t								GetTotatlBytesRead() const;
	std::string							GetFullPath() const;
	std::string							GetHeaderValue(std::string) const;
	std::string							GetUnprocessedBuffer() const;
	std::string							GetHeaderBuffer() const;
	std::vector<std::string>			GetPathParts() const;
	std::map<std::string, std::string>	GetHeaders() const;
	std::map<std::string, std::string>	GetQueryParams() const;
	ConfigNode							&GetRightServer();
	BoundarySettings					GetBoundarySettings() const;

	
	// ---------		SETTERS 	 	--------- //
	void	SetHeaderValue(std::string, std::string);
	void	SetContentLength(const size_t	Length);
	void	SetClientStatus(ClientStatus	Status);
	
	// ---------	MEMBER FUNCTIONS 	--------- //
	void	ReadRequestHeader();
	void	ReadFirstLine(std::string);
	void	ReadHeaders(std::string);
	void	PostRequiredHeaders();

	void	CheckRequiredHeaders();

	void	HandleQuery();
	void	HandlePath();
	void	SplitURI();
	void	ParseURI(std::string	&URI);
	void	SetUpRequest();
	void	ReadBodyChunk();
	void	GetBoundaryFromHeader();
};

// ---------	HELPER FUNCTIONS 	--------- //

bool			IsHexa(char c);
void			TrimSpaces(std::string& str);
std::string		HexaToChar(std::string	Hexa);
void			DecodeHexaToChar(std::string	&str);
void			PrintHeaders(std::map<std::string, std::string> Headers);
void			PrintError(const std::string	&Err);
bool			ValidContentLength(const std::string& value);
bool			ValidFieldName(const std::string& name);
bool			ValidFieldValue(const std::string& value);
bool			ValidBoundary(const std::string	&value);
size_t			CrlfCounter(std::string	&str);
