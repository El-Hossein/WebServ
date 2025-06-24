#pragma once

#include "../allincludes.hpp"
#include "../pars_config/config.hpp"

#define BUFFER_SIZE 1024 // 1kb
#define MAX_HEADER_SIZE 8192 // 8kb

typedef std::vector<std::pair<std::string, std::string> > PairedVectorSS;

class Request
{
private:
	int						ClientFd;
	std::vector<ConfigNode>	Servers;
	ConfigNodd				RightServer;

	PairedVectorSS	Headers;
	PairedVectorSS	QueryParams;

	std::string		FullSystemPath;
	std::string		BodyUnprocessedBuffer;

	void	ReadRequestHeader();
	void	ReadFirstLine(std::string);
	void	ReadHeaders(std::string);

	void	CheckRequiredHeaders();

	void	HandleQuery();
	void	SplitURI();
	void	ParseURI(std::string	&URI);
public:
	Request(const int	&, std::vector<ConfigNode>);
	~Request();

	// ---------		GETTERS 	 	--------- //
	PairedVectorSS	GetHeaders() const;
	std::string		GetHeaderValue(std::string) const;

	// ---------		SETTERS 	 	--------- //
	void	SetHeaderValue(std::string, std::string);

	// ---------	MEMBER FUNCTIONS 	--------- //
	void	SetUpRequest();
};

// ---------	HELPER FUNCTIONS 	--------- //

bool			IsHexa(char c);
std::string		HexaToChar(std::string	Hexa);
void			PrintHeaders(PairedVectorSS Headers);