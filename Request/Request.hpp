#pragma once

#include "../allincludes.hpp"

typedef std::vector<std::pair<std::string, std::string> > PairedVectorSS;

class Request 
{
private:
	const	std::string	&full_request;
	PairedVectorSS		headers;
	const std::string	body;

	void	ParseRequest();
public:
	Request(const std::string &);
	~Request();
	// ---------	getters 	---------
	PairedVectorSS		getHeaders() const;
	const std::string	getBody() const;

	void	SetUpRequest();
};