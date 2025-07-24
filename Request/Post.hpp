#pragma once

#include "Request.hpp"
#include "../allincludes.hpp"

#define	BODY_BUFFER_SIZE 1028

class Request;
class ConfigNode;

class Post
{
private:
	Request	&obj;

	bool								EndOfRequest;
	bool								BodyFullyRead;

	std::map<std::string, std::string>	BodyParams;
	std::string							BodyUnprocessedBuffer;

	size_t			MaxAllowedBodySize;
public:
	Post(Request	&_obj);
	~Post();

	void	WriteToFile(std::string	&str);
	void	GetSubBodys(std::string &Buffer);
	void	ParseChunked(std::string);
	void	ParseBoundary(std::string);
	void	IsBodyFullyRead();
	void	HandlePost();
};
