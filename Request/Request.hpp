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

	PairedVectorSS	Headers;
	std::string		BodyUnprocessedBuffer;

	void	ReadRequestHeader();
	void	ReadFirstLine(std::string);
	void	ReadHeaders(std::string);
	
	void	CheckRequiredHeaders();
public:
	Request(const int	&, std::vector<ConfigNode>);
	~Request();
	// ---------		GETTERS 	 	--------- //
	PairedVectorSS	GetHeaders() const;
	std::string		GetHeaderValue(std::string key1) const;
	// ---------	MEMBER FUNCTIONS 	--------- //
	void	SetUpRequest();
};