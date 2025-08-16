#pragma once

#include "Request.hpp"
#include "../allincludes.hpp"

/**
 *
 *	Handle the Location in request for every path...
 * 
 */


class Request;

class Post
{
private:
	Request	&obj;
	_BoundarySettings					Boundary;
	_BoundaryStatus						BoundaryStatus;

	ChunkVars							Chunk;

	bool								FirstTime;
	bool								BodyFullyRead;
	bool								RmvFirstCrlf;

	std::map<std::string, std::string>	BodyParams;
	std::string							UnprocessedBuffer;
	std::string							PreviousBuffer;
	std::string							PrevBuffer;

	std::string							Dir;
	std::string							Filename;
	std::string							cgiFileName;
	std::ofstream						OutFile;
public:
	Post(Request	&_obj);
	~Post();

	void	FindFileName(std::string	&Buffer, std::string	&Filename);

	void	GetSubBodies(std::string &Buffer);
	void	ParseBoundary();

	void	ParseBirnaryOrRaw();

	void	ParseChunkedBoundary();

	void	WriteChunkToFile(std::string &BodyContent);
	void	GetChunks();
	void	ParseChunked();

	void	IsBodyFullyRead();
	void	HandleCGI();
	void	HandlePost();
};
