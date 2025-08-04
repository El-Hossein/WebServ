#pragma once

#include "Post.hpp"
#include "../allincludes.hpp"
#include "../pars_config/config.hpp"

#define MAX_HEADER_SIZE	8000
#define BUFFER_SIZE		80000

enum	Method
{
	GET, POST, DELETE
};

enum	ClientStatus
{
	ReadHeader, ReadBody, EndReading
};	

enum	DataType
{
	Chunked, FixedLength
};

enum	ContentType
{
	_Boundary, BinaryOrRaw
};

class	Request
{
private:
	_ServerDetails						ServerDetails;
	std::map<std::string, std::string>	ExtentionsMap;
	

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
	std::string							Location;
	std::vector<std::string>			AllowedMethods;
	std::vector<std::string>			PathParts;

	_BoundarySettings			BoundaryAttri;
	std::string					HeaderBuffer;
	std::string					BodyUnprocessedBuffer;
	std::string					FileExtention;

	size_t						TotalBytesRead;
	size_t						ContentLength;
	bool						KeepAlive;
	bool						RequestNotComplete;

public:
	Request(const int	&, ClientStatus, std::vector<ConfigNode>, int &);
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
	std::string							GetFileExtention();
	std::string							GetHeaderValue(std::string) const;
	std::string							GetUnprocessedBuffer() const;
	std::string							GetHeaderBuffer() const;
	std::vector<std::string>			GetPathParts() const;
	std::map<std::string, std::string>	GetHeaders() const;
	std::map<std::string, std::string>	GetQueryParams() const;
	ConfigNode							&GetRightServer();
	_BoundarySettings					GetBoundarySettings() const;
	_ServerDetails						GetServerDetails() const;
	
	// ---------		SETTERS 	 	--------- //
	void	SetFullSystemPath(std::string	&Path);
	void	SetExtentionsMap();
	void	SetHeaderValue(std::string, std::string);
	void	SetContentLength(const size_t	Length);
	void	SetClientStatus(ClientStatus	Status);
	void	SetServerDetails();
	
	// ---------	MEMBER FUNCTIONS 	--------- //

	void	ReadRequestHeader();
	void	ReadFirstLine(std::string);
	void	ReadHeaders(std::string);
	void	PostRequiredHeaders();

	void	ParseHeaders();

	void	CheckIfAllowedMethod();
	void	HandleQuery();
	void	HandlePath();
	void	SplitURI();
	void	ParseURI();
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
void			CreateDirectory(std::string &FilenameDir);
int				FindFileName(std::string	&Buffer, std::string	&Filename);
void			PrintCrlfString(std::string Buffer);
std::string		RandomString();
int				HexaToInt(std::string	x);
std::string		RemoveCrlf(std::string BodyContent);
