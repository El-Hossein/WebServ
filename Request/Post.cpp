#include "Post.hpp"

Post::Post(Request	&_obj) :	obj(_obj),
								BodyUnprocessedBuffer(_obj.GetUnprocessedBuffer()),
								MaxAllowedBodySize(std::strtod(ConfigNode::getValuesForKey(_obj.GetRightServer(), "client_max_body_size", "NULL")[0].c_str(), NULL)) // [0] First element -> "10M"
{
}

Post::~Post() {
}

/**
 * @brief check the required headers for the Post method
 * the absence of Content-Type Header -> default Content type is "text/plain; charset=US-ASCII"
 * Each body part is preceded by a boundary delimiter
 * If the boundary existes in the body -> malformed body
 * The boundary consists of {1 to 70} characters -> Done
 * allowed : DIGIT / ALPHA / "'" / "(" / ")" / "+" / "_" / "," / "-" / "." / "/" / ":" / "=" / "?" -> almost Done

 * 
 */
void	Post::PostRequiredHeaders()
{
	std::map<std::string, std::string>	Headers = obj.GetHeaders();
	std::string							LowKey;
	
	if (Headers.find("transfer-encoding") != Headers.end() && Headers.find("transfer-encoding")->second != "chunked")
		throw "501 Not implemented - PostRequestHeaders()";

	if (Headers.find("content-length") != Headers.end())
	{
		if (!ValidContentLength(Headers.find("content-length")->second))	throw "400: Bad Request";
		obj.SetContentLength(strtod(Headers.find("content-length")->second.c_str(), NULL));
	}

	if (Headers.find("content-type") != Headers.end())
	{
		if (Headers.find("content-type")->second == "application/x-www-form-urlencoded")
				this->ContentType = UrlEncoded;
		if (Headers.find("content-type")->second.find("multipart/form-data") != std::string::npos)
			this->ContentType = MultipartFormData;
		if (Headers.find("content-type")->second == "application/json")
			this->ContentType = ApplicationJson;
	}
}

bool	Post::ReadFullBody()
{
	bool	flag = false;
	MaxAllowedBodySize = 1028 * 10; // tmp value = 10280 -> 10MegaBytes
	
	std::cout << "Content-Length:{" << obj.GetContentLength() << "}" << std::endl;
	std::cout << "UnprocessedBuffer-Length:{" << BodyUnprocessedBuffer.size() << "}" << std::endl;

	if (obj.GetContentLength() > this->MaxAllowedBodySize)
		throw "413 Request Entity Too Large";

	BodyUnprocessedBuffer.size() == obj.GetContentLength() ? flag = true : flag = false;
	return flag;
}

/*	|#----------------------------------#|
	|#			ParseUrlEncoded	    	#|
	|#----------------------------------#|
*/

void	Post::ParseUrlEncoded(std::istringstream	&stream)
{
	size_t				pos = 0;
	std::string			key, value, tmp;

	while (std::getline(stream, tmp, '&'))
	{
		pos = tmp.find("=");
		if (pos == std::string::npos)
		continue;
		key = tmp.substr(0, pos);
		value = tmp.substr(pos + 1);
		if (key.empty() || value.empty())
			continue;
		std::cout << "Before:" << value << std::endl;
		DecodeHexaToChar(value);
		std::cout << "After:" << value << std::endl;
		BodyParams[key] = value;
	}
}

/*	|#----------------------------------#|
	|#		ParseMultipartFormData    	#|
	|#----------------------------------#|
*/

void	Post::GetBoundaryFromHeader()
{
	std::string										tmp;
	std::map<std::string, std::string>				Headers = obj.GetHeaders();
	std::map<std::string, std::string>::iterator	it = Headers.find("content-type");
	
	size_t	pos = it->second.find(";");
	if (pos == std::string::npos)
		throw "400 Bad Request -ParseMultipartFormData()";

	tmp = it->second.substr(pos + 1);

	pos = it->second.find("=");
	if (pos == std::string::npos)
		throw "400 Bad Request -ParseMultipartFormData()";

	Boundary = it->second.substr(pos + 1); // setting Boundary
	if (Boundary.length() < 1 || Boundary.length() > 70 || !ValidBoundary(Boundary))
		throw "400 Bad Request -ParseMultipartFormData() -Boundary Parsing.";

	BoundaryStart = "--" + Boundary;
	BoundaryEnd = "--" + Boundary + "--";
}

void	Post::GetBodyEntity(std::string	&SmallBody)
{
	size_t				pos = 0;
	std::istringstream	stream(SmallBody);
	std::string			tmp, value;

	while (getline(stream, tmp))
	{
		TrimSpaces(tmp);
		// std::cout << "tmp value:{" << tmp << "}" <<  std::endl;
		pos = tmp.find(": ");
		if (pos == std::string::npos)
			continue;
		value = tmp.substr(pos + 2);
		std::cout << "value of the body header:{" << value<< "}" << std::endl;
	}
}

void	Post::ParseMultipartFormData(std::istringstream	&BodyStream, std::string &str)
{
	GetBoundaryFromHeader();

	size_t 		start = 0, end = 0;
	std::string	tmp;

	start = str.find(BoundaryStart, start) + BoundaryStart.length();
	if (start == std::string::npos)
		throw "404 Invalide request ParseMultipartFormData()";
	end = str.find(BoundaryEnd, start);
	if (end == std::string::npos)
		throw "404 Invalide request ParseMultipartFormData()";
	else
	{
		std::cout << start << std::endl;
		tmp = str.substr(start, end);
		GetBodyEntity(tmp);
		std::cout << "First Entity:\n" << "{" << tmp << "}" << std::endl;
	}
}

/*	|#----------------------------------#|
	|#		ParseApplicationJson    	#|
	|#----------------------------------#|
*/


void	Post::ParseApplicationJson(std::istringstream	&stream)
{

}

void	Post::ParseBody()
{
	std::string			Body = obj.GetUnprocessedBuffer();
	std::istringstream	stream(Body);

	switch (this->ContentType)
	{
		case UrlEncoded			:	return ParseUrlEncoded(stream);
		case MultipartFormData	:	return ParseMultipartFormData(stream, Body);
		case ApplicationJson	:	return ParseApplicationJson(stream);
		case Unknown			:	return throw "415 Unsupported Media Type";
	}
}

void	Post::HandleBody()
{
	std::cout << "\n\t\t<# POST #>\t\t\n\n";

	PostRequiredHeaders();
	bool	flag = ReadFullBody();
	if (flag) // the Body has been entirely read
		ParseBody();
}
