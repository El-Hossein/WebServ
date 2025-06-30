#pragma once

#include "Get.hpp"
#include "Post.hpp"
#include "Delete.hpp"
#include "../allincludes.hpp"
#include "../pars_config/config.hpp"

#define BUFFER_SIZE 1024 // 1kb
#define MAX_HEADER_SIZE 8192 // 8kb

enum Method
{
	GET,
	POST,
	DELETE
};

class	Request
{
private:
	int						ClientFd;
	std::vector<ConfigNode>	Servers;
	ConfigNode				RightServer;

	Method								Method;
	std::map<std::string, std::string>	Headers;
	std::map<std::string, std::string>	QueryParams;
	std::string							FullSystemPath;
	std::vector<std::string>			PathParts;

	bool						KeepAlive;
	std::string					BodyUnprocessedBuffer;
	size_t						ContentLength;
	
public:
	Request(const int	&, std::vector<ConfigNode>);
	~Request();
	
	// ---------		GETTERS 	 	--------- //
	std::map<std::string, std::string>	GetHeaders() const;
	std::string		GetFullPath() const;
	std::string		GetHeaderValue(std::string) const;
	bool			GetConnection() const;
	
	// ---------		SETTERS 	 	--------- //
	void	SetHeaderValue(std::string, std::string);
	
	// ---------	MEMBER FUNCTIONS 	--------- //
	void	ReadRequestHeader();
	void	ReadFirstLine(std::string);
	void	ReadHeaders(std::string);

	void	CheckRequiredHeaders();

	void	HandleQuery();
	void	HandlePath();
	void	SplitURI();
	void	ParseURI(std::string	&URI);
	void	SetUpRequest();
};

// ---------	HELPER FUNCTIONS 	--------- //

bool			IsHexa(char c);
std::string		HexaToChar(std::string	Hexa);
void			PrintHeaders(std::map<std::string, std::string> Headers);
bool			ValidContentLength(const std::string& value);
bool			ValidFieldName(const std::string& name);
bool			ValidFieldValue(const std::string& value);