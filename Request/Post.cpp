#include "Post.hpp"

Post::Post(Request	&_obj) :	obj(_obj),
								BodyUnprocessedBuffer(_obj.GetUnprocessedBuffer()),
								MaxAllowedBodySize(std::strtod(ConfigNode::getValuesForKey(_obj.GetRightServer(), "client_max_body_size", "NULL")[0].c_str(), NULL)), // [0] First element -> "10M"
								EndOfRequest(false),
								BodyFullyRead(false)
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

	if (Headers.find("content-length") == Headers.end() && Headers.find("transfer-encoding") == Headers.end())
		PrintError("Missing POST required headers"), throw "400 Bad request";

	if (Headers.find("content-length") != Headers.end())
	{
		if (!ValidContentLength(Headers["content-length"]))
			PrintError("Invalide Content-Length"), throw "400: Bad Request";
		obj.SetContentLength(strtod(Headers["content-length"].c_str(), NULL));
		DataType = FixedLength;
	}

	(Headers.find("transfer-encoding") != Headers.end() && Headers["transfer-encoding"] == "chunked") ?
		DataType = Chunked : throw "501 Not implemented - PostRequestHeaders()";

	if (Headers.find("content-type") != Headers.end())
	{
		if (Headers["content-type"] == "application/x-www-form-urlencoded")
				this->ContentType = UrlEncoded;
		if (Headers["content-type"].find("multipart/form-data") != std::string::npos)
			this->ContentType = MultipartFormData;
		if (Headers["content-type"] == "application/json")
			this->ContentType = ApplicationJson;
	}
}

void	Post::IsBodyFullyRead()
{
	MaxAllowedBodySize = 1028 * 10; // tmp value = 10280 -> 10MegaBytes
	
	std::cout << "Content-Length:{" << obj.GetContentLength() << "}" << std::endl;
	std::cout << "UnprocessedBuffer-Length:{" << BodyUnprocessedBuffer.size() << "}" << std::endl;

	// if (obj.GetContentLength() > this->MaxAllowedBodySize)
	// 	throw "413 Request Entity Too Large";

	BodyUnprocessedBuffer.size() == obj.GetContentLength() ? BodyFullyRead = true : BodyFullyRead = false;
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

	BoundaryStart = "\r\n--" + Boundary;
	BoundaryEnd = "\r\n--" + Boundary + "--";
}

void	Post::WriteToFile(std::string	&Buffer)
{
	size_t	FilenamePos = 0, FilenameEndPos = 0, BodyPos = 0;
	std::ofstream		OutFile;
	std::string			Filename, FilenamePath, BodyContent;
	std::istringstream	stream(Buffer);

	//------------------	Find SubBodyContent	------------------//
	BodyPos = Buffer.find("\r\n\r\n");
	if (BodyPos == std::string::npos)
		throw "400 Bad Request -WriteToFile()";
	BodyContent = Buffer.substr(BodyPos + 4);
	// std::cout << "BodyContent:{" << BodyContent << "}" << std::endl;

	//------------------	Find FileName	------------------//
	FilenamePos = Buffer.find("filename=\"");
	if (FilenamePos != std::string::npos)
	{
		FilenameEndPos = Buffer.find("\"\r\n", FilenamePos + 10);
		if (FilenameEndPos == std::string::npos)
		throw "400 Bad Request -WriteToFile()";
		
		Filename = Buffer.substr(FilenamePos + 10, FilenameEndPos - (FilenamePos + 10)); // 10 = sizeof("filename=")
		std::cout << "Filename:{" << Filename << "}" << std::endl;
		
		FilenamePath = "Uploads/" + Filename;

		OutFile.open(FilenamePath.c_str(), std::ios::app); // std::ios::app => to append
		if (!OutFile.is_open())
			throw "500 Internal Server Error";
		OutFile << BodyContent;
	}
}

void	Post::GetSubBodys(std::string &Buffer)
{
	Buffer = "\r\n" + Buffer;
	std::string	tmp;
	size_t start = 0, end = 0, RequestEnd = 0;
	
//------------------	Check if Request Ended	------------------//
	if (Buffer.find(BoundaryEnd) != std::string::npos)
		this->EndOfRequest = true;
	while (true)
	{
		start = Buffer.find(BoundaryStart, end);
		if (start == std::string::npos)		break;

		end = Buffer.find(BoundaryStart, start + BoundaryStart.length());
		if (end == std::string::npos)		break;

		size_t	size = end - start;
		tmp = Buffer.substr(start, size);

		WriteToFile(tmp);
	}
}

void	Post::ParseMultipartFormData(std::istringstream	&BodyStream, std::string &str)
{

	GetBoundaryFromHeader();
/*	 Loop on to read the full request from socket	*/
	GetSubBodys(str);

/*	 Check if the Request has been fully read	*/
	if (EndOfRequest)	throw "200 Success";
	else				PrintError("Invalide Boundary End"), throw "400 Bad request";
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
		default					:	return throw "415 Unsupported Media Type";
	}
}

void	Post::HandlePost()
{
	std::cout << "\n\t\t<# POST #>\t\t\n\n";

	PostRequiredHeaders();

	IsBodyFullyRead();
	if (BodyFullyRead) // the Body has been entirely read
		ParseBody();

}
