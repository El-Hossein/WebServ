#pragma once

#include "../allincludes.hpp"

typedef std::vector<std::pair<std::string, std::string> > PairedVectorSS;

class Request 
{
private:
	std::string			full_request;
	PairedVectorSS		headers;

	void	ParseRequest();
public:
	Request(std::string);
	~Request();
	// ---------	getters 	---------
	PairedVectorSS		getHeaders() const;

	void	SetUpRequest();
};