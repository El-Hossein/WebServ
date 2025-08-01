#include "Request.hpp"

std::string	RandomString()
{
	std::string			Ret;
	const std::string	Chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	while(Ret.size() != 15) // generante random 15 chars string
		Ret += Chars[rand() % (Chars.size() - 1)];
	return Ret;
}

void	PrintCrlfString(std::string Buffer)
{
	std::cout << "|";

	for (size_t i = 0; i < Buffer.size(); i++)
	{
		if (Buffer[i] == '\r')
			std::cout << "R";
		else if (Buffer[i] == '\n')
			std::cout << "N";
		else
			std::cout << Buffer[i];
	}

	std::cout << "|\n";
}

// --------------#	URI TOOLS #-------------- //


bool	IsHexa(char c)
{
	return std::isdigit(c) || (std::tolower(c) >= 'a' && std::tolower(c) <= 'f');
}

bool	IsValide(char c)
{
	return std::isalnum(c) || c == '!' || c == '#' || c == '$' || c == '%' ||
		   c == '&' || c == '\'' || c == '*' || c == '+' || c == '-' || c == '.' ||
		   c == '^' || c == '_' || c == '`' || c == '|' || c == '~';
}

std::string	HexaToChar(std::string	Hexa)
{
	std::string tmp = "0x" + Hexa; // Convert to Hexa form

	char	Helpervar = static_cast<char>(std::stod(tmp)); // Convert Hexa to Char
	return std::string(1, Helpervar); // calling constructor string with 1 character
}

int			HexaToInt(std::string	&x)
{
	int y;
    std::stringstream stream(x);

	std::string HexaChars = "ABCDEFabcdef0123456789";
	for (size_t i = 0; i < x.size() ; i++)
	{
		if (HexaChars.find(x[i]) == std::string::npos)
			PrintError("Invalide Hexa value"), throw "400 Bad request";
	}
    stream << x;
    stream >> std::hex >> y;
    return y;
}

void		DecodeHexaToChar(std::string	&str)
{
	size_t	pos = 0;
	
	while((pos = str.find("%", pos)) != std::string::npos)
	{
		if (pos + 2 < str.size() && IsHexa(str[pos + 1]) && IsHexa(str[pos + 2]))
		{
			str.replace(pos, 3, HexaToChar(str.substr(pos + 1, 2))); // 2 letters after '%
			pos += 1; // 1 -> size d charachter
		}
		else
			throw "Error: % in the str";
    }
}

// --------------#	PRINTER	 #-------------- //

void	PrintHeaders(std::map<std::string, std::string> Headers)
{
	for (std::map<std::string, std::string>::const_iterator it = Headers.begin(); it != Headers.end(); ++it)
	std::cout << "key{" << it->first << "}		value:" << it->second << std::endl;
	std::cout  << "-----------------------------------------------------" << std::endl;
}

void	PrintError(const std::string	&Err)
{
	std::cerr << Err << std::endl;
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

bool	ValidFieldName(const std::string& name)
{
    if (name.empty())
		return false;
    for (size_t i = 0; i < name.size(); ++i)
        if (!IsValide(name[i]))
			return false;
    return true;
}

bool	ValidFieldValue(const std::string& value)
{
    for (size_t i = 0; i < value.size(); i++)
        if (!std::isprint(value[i]))
			return false;
    return true;
}

bool	ValidBoundary(const std::string	&value)
{
	for (size_t	i = 0; i < value.length(); i++)
	{
		if (!std::isalnum(value[i]) || !std::isprint(value[i]))
		{
			if (value[i] != '(' && value[i] != ')' && value[i] != '+' && value[i] != '-' &&
				value[i] != '_'	&& value[i] != ',' && value[i] != '.' && value[i] != ':' &&
				value[i] != '=' && value[i] != '?')
					return false;
		}
	}
	return true;
}

void	TrimSpaces(std::string& str)
{
    size_t start = 0;

    while (start < str.length() && std::isspace(static_cast<unsigned char>(str[start])))
        ++start;

    size_t end = str.length();

    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1])))
        --end;

	str = str.substr(start, end - start);
}

void	CreateDirectory(std::string &FilenameDir)
{
	struct stat	Tmp;

	if (stat(FilenameDir.c_str(), &Tmp)) // return 0 if exists || if not create it
	{
		if (mkdir(FilenameDir.c_str(), 0777)) // return 0 means success
			PrintError("Could't open Directory"), throw "400 Bad Request";
	}
}

// --------------#	COUNTERS	 #-------------- //

size_t	CrlfCounter(std::string	&str)
{
	std::string	CRLF("\n\r");
	size_t Pos = 0, Count = 0;
	
	while ((Pos = str.find(CRLF, Pos))!= std::string::npos)
	{
		Pos += CRLF.length();
		Count++;
	}
	return Count;
}
