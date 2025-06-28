#pragma once

#include "Request.hpp"
#include "../allincludes.hpp"

class Request;

class Delete
{
private:
	const Request	&obj;
public:
	Delete(const Request	&_obj);
	~Delete();
};
