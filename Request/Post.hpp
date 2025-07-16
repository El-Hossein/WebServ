#pragma once

#include "Request.hpp"
#include "../allincludes.hpp"

#define	BODY_BUFFER_SIZE 1028

class Request;
class ConfigNode;

enum	ContentType
{
	UrlEncoded,
	MultipartFormData,
	ApplicationJson,
	Unknown
};

class Post
{
private:
	Request	&obj;

	ContentType							ContentType;
	std::string							Boundary;
	std::string							BoundaryStart;
	std::string							BoundaryEnd;

	bool								EndOfRequest;
	bool								BodyFullyRead;

	
	std::map<std::string, std::string>	BodyParams;
	std::string							BodyUnprocessedBuffer;
	char								BodyBuffer[BODY_BUFFER_SIZE];

	size_t			MaxAllowedBodySize;
public:
	Post(Request	&_obj);
	~Post();

	void	GetBoundaryFromHeader();
	
	void	WriteToFile(std::string	&str);
	void	GetSubBodys(std::string &Buffer);
	void	ParseUrlEncoded(std::istringstream	&);
	void	ParseMultipartFormData(std::istringstream	&, std::string &);
	void	ParseApplicationJson(std::istringstream	&);

	void	PostRequiredHeaders();
	void	ParseBody();
	void	IsBodyFullyRead();
	void	HandleBody();
};
