#pragma once

#include "../allincludes.hpp"

#define BUFFER_SIZE 200

typedef std::vector<std::pair<std::string, std::string> > PairedVectorSS;

class Request 
{
private:
	int				ClientFd;
	char			buffer[BUFFER_SIZE];
	std::string		FullRequest;
	PairedVectorSS	Headers;
public:
	Request(const int	&);
	~Request();
	// ---------		GETTERS 	 	--------- //
	PairedVectorSS		getHeaders() const;
	std::string			getFullRequest() const;
	// ---------	MEMBER FUNCTIONS 	--------- //
	
	void	ReadRequest();
	
	void	SetUpRequest();
	void	ParseRequest();
};