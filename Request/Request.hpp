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
	std::vector<ConfigNode>	ConfigPars;

	PairedVectorSS	Headers;
	std::string		BodyUnprocessedBuffer;

	void	ReadRequestHeader();
	
	void	ParseFirstLine(std::string);
	void	ParseRequestHeader(std::string);
	
	void	CheckRequiredHeaders();
public:
	Request(const int	&, std::vector<ConfigNode>);
	~Request();
	// ---------		GETTERS 	 	--------- //
	PairedVectorSS		getHeaders() const;
	// ---------	MEMBER FUNCTIONS 	--------- //
	void	SetUpRequest();
};