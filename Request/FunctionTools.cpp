#include "Request.hpp"

// --------------#	URI TOOLS #-------------- //

bool	IsHexa(char c)
{
	return std::isdigit(c) || (std::tolower(c) >= 'a' && std::tolower(c) <= 'f');
}

std::string	HexaToChar(std::string	Hexa)
{
	std::cout << Hexa << std::endl;
	std::string tmp = "0x" + Hexa; // Convert to Hexa form

	char	Helpervar = static_cast<char>(std::stod(tmp)); // Convert Hexa to Char
	return std::string(1, Helpervar); // calling constructor string with 1 character
}

// --------------#	PRINTER	 #-------------- //

void	PrintHeaders(PairedVectorSS Headers)
{
	for (PairedVectorSS::const_iterator it = Headers.begin(); it != Headers.end(); ++it)
	{
		std::cout << it->first << ": ----->" << it->second << std::endl;
	}
	std::cout  << "-----------------------------------------------------" << std::endl;
}