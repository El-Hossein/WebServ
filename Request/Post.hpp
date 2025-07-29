#pragma once

#include "Request.hpp"
#include "../allincludes.hpp"

class Request;

class Post
{
private:
	Request	&obj;
	_BoundarySettings					Boundary;
	_SubBodyStatus						SubBodyStatus;
	BoundaryFlager						Flager;
	
	_BoundaryStatus						BoundaryStatus;

	bool								EndOfRequest;
	bool								BodyFullyRead;

	std::map<std::string, std::string>	BodyParams;
	std::string							UnprocessedBuffer;

	std::string							Filename;
	std::ofstream						OutFile;

	size_t			MaxAllowedBodySize;
public:
	Post(Request	&_obj);
	~Post();

	void	FindFileName(std::string	&Buffer, std::string	&Filename);
	void	WriteToFile(std::string	&Filename, std::string &Buffer);
	void	GetSubBodies(std::string &Buffer);
	void	ParseChunked(std::string);
	void	ParseBoundary();
	void	IsBodyFullyRead();
	void	HandlePost();
};
