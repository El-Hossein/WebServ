#include "Request.hpp"

/*
	check if URI contains characters not allowed
		throw "400 Bad Request";

	check if URI length > 2048 chars
		throw "414 Request-URI too long"
*/
void	ParseURI(std::string	&URI)
{
	for (size_t	i = 0; i < URI.length(); i++)
		if (!isalnum(URI[i]) && URI[i] != '/' && URI[i] != '.'
			&& URI[i] != '-' && URI[i] != '_')
				throw "400 Bad Request";

	if (URI.length() > 2048)
		throw "414 Request-URI too long";
}

void	SplitURI(std::string	&URI)
{
	
}