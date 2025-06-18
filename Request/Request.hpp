#pragma once

#include "../allincludes.hpp"

#define BUFFER_SIZE 1024 // 1kb
#define MAX_HEADER_SIZE 8192 // 8kb

typedef std::vector<std::pair<std::string, std::string> > PairedVectorSS;

class Request 
{
private:
	int				ClientFd;
	std::string		FullRequest; // can't have this bec max size is 30mb
	PairedVectorSS	Headers;
public:
	Request(const int	&);
	~Request();
	// ---------		GETTERS 	 	--------- //
	PairedVectorSS		getHeaders() const;
	std::string			getFullRequest() const;
	// ---------	MEMBER FUNCTIONS 	--------- //
	
	void	ReadRequestHeader();
	void	ParseRequest();

	void	SetUpRequest();
};