#pragma once

#include "Request.hpp"
#include "../allincludes.hpp"

class Request;

class Post
{
private:
	const Request	&obj;
public:
	Post(const Request	&_obj);
	~Post();
};
