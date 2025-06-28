#pragma once

#include "../allincludes.hpp"
#include "Request.hpp"

class Request;

class Get
{
private:
	const Request	&obj;
public:
	Get(const Request	&_obj);
	~Get();

	void			CheckPath(std::string &path);
	std::string		ReadFile();
};
