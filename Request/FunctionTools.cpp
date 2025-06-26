#include "Request.hpp"

// --------------#	URI TOOLS #-------------- //

bool	IsHexa(char c)
{
	return std::isdigit(c) || (std::tolower(c) >= 'a' && std::tolower(c) <= 'f');
}

std::string	HexaToChar(std::string	Hexa)
{
	std::string tmp = "0x" + Hexa; // Convert to Hexa form

	char	Helpervar = static_cast<char>(std::stod(tmp)); // Convert Hexa to Char
	return std::string(1, Helpervar); // calling constructor string with 1 character
}

// --------------#	PRINTER	 #-------------- //

void	PrintHeaders(std::map<std::string, std::string> Headers)
{
	for (std::map<std::string, std::string>::const_iterator it = Headers.begin(); it != Headers.end(); ++it)
	std::cout << "key{" << it->first << "}		value:" << it->second << std::endl;
	std::cout  << "-----------------------------------------------------" << std::endl;
}

// --------------#	PARSERS	 #-------------- //

bool	ValidContentLength(const std::string& value)
{
	if (value.empty())					return false;
	for (size_t i = 0; i < value.length(); i++)
	    if (!std::isdigit(value[i]))	return false;
	size_t ret = strtod(value.c_str(), NULL);
	if (ret < 0)						return false;

	return true;
}
