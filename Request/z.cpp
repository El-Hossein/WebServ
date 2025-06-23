#include "../allincludes.hpp"

// void	DecodeQuery(std::string	&URI)
// {
// 	size_t start = 0;

//     while((start = URI.find("%20", start)) != std::string::npos)
// 	{
//         URI.replace(start, 3, " "); // 3 -> size d "%20"
//         start += 1; // 1 -> size d " "
//     }

// 	std::cout << "Decoded URI:" << URI << std::endl;
// }

// void	ParseURI(std::string	&URI)
// {

// 	for (size_t	i = 0, j = 0; i < URI.length(); i++)
// 	{
// 		if (!isalnum(URI[i]) && URI[i] != '/' && URI[i] != '.'
// 			&& URI[i] != '-' && URI[i] != '_' && URI[i] != '?')
// 				throw "400 Bad Request 1";
// 		/*
// 			check if there is two "?"
// 		*/
// 		if (URI[i] == '?')
// 			(++j >= 2) ? throw "400 Bad Request 2" : (void)0;
// 	}

// 	if (URI.length() > 2048)
// 		throw "414 Request-URI too long";
// }

// void	SplitURI(std::string &URI)
// {
// 	std::string	Path,Query;
// 	size_t pos = URI.find("?");

// 	DecodeQuery(URI);

// 	if (pos == std::string::npos) // Query has not been found
// 	{
// 		Path = URI;
// 		std::cout << "PATH ---->" << Path << std::endl;
// 		Query = "";
// 		std::cout << "QUERY ---->" << Query << std::endl;
// 		return ;
// 	}
// 	Path = URI.substr(0, pos);
// 	std::cout << "PATH ---->" << Path << std::endl;
// 	Query = URI.substr(pos + 1);
// 	std::cout << "QUERY ---->" << Query << std::endl;
// }

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

void	DecodeQuery(std::string	&URI)
{
	size_t pos = 0;

    while((pos = URI.find("%", pos)) != std::string::npos)
	{
		if (pos + 2 < URI.size() && IsHexa(URI[pos + 1]) && IsHexa(URI[pos + 2]))
		{
			URI.replace(pos, 3, HexaToChar(URI.substr(pos + 1, 2))); // 2 letters after '%
			pos += 1; // 1 -> size d charachter
		}
		else
			throw "Error: % in the Query";
    }

	std::cout << "Decoded URI:" << URI << std::endl;
}

int main()
{
	std::string	URI = "%2A%2A%3c/users/profile%20pics/../edit?name=zi%20yad&id=42&admin=true#section2";

	DecodeQuery(URI);
}