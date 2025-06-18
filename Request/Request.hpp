#pragma once

#include "../allincludes.hpp"

#define BUFFER_SIZE 1024 // 1kb
#define MAX_HEADER_SIZE 8192 // 8kb

typedef std::vector<std::pair<std::string, std::string> > PairedVectorSS;

class Request 
{
private:
	int				ClientFd;
	PairedVectorSS	Headers;
public:
	Request(const int	&);
	~Request();
	// ---------		GETTERS 	 	--------- //
	PairedVectorSS		getHeaders() const;
	// ---------	MEMBER FUNCTIONS 	--------- //
	
	void	ReadRequestHeader();
	
	void	ParseFirstLine(std::string);
	void	ParseRequestHeader(std::string);

	void	SetUpRequest();
};