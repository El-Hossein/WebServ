#pragma once

#include "Request.hpp"
#include "../allincludes.hpp"

class Request;

class Post
{
private:
	Request	&obj;
	_BoundarySettings					Boundary;
	BoundaryFlager						Flager;
	_BoundaryStatus						BoundaryStatus;

	ChunkVars							Chunk;

	bool								FirstTime;
	bool								BodyFullyRead;
	bool								RmvFirstCrlf;

	std::map<std::string, std::string>	BodyParams;
	std::string							UnprocessedBuffer;
	std::string							PreviousBuffer;

	std::string							Dir;
	std::string							Filename;
	std::ofstream						OutFile;

	size_t			MaxAllowedBodySize;
public:
	Post(Request	&_obj);
	~Post();

	void	FindFileName(std::string	&Buffer, std::string	&Filename);

	void	GetSubBodies(std::string &Buffer);
	void	ParseBoundary();

	void	ParseBirnaryOrRaw();

	void	GetSubBodies2(std::string &Buffer);
	void	ParseChunkedBoundary();

	void	WriteChunkToFile(std::string &BodyContent);
	void	GetChunks();
	void	ParseChunked();

	void	IsBodyFullyRead();
	void	HandlePost();
};
