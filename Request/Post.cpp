#include "Post.hpp"
#include "Request.hpp"

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

// void	Post::ParseUrlEncoded(std::istringstream	&stream)
// {
// 	size_t				pos = 0;
// 	std::string			key, value, tmp;

// 	while (std::getline(stream, tmp, '&'))
// 	{
// 		pos = tmp.find("=");
// 		if (pos == std::string::npos)
// 		continue;
// 		key = tmp.substr(0, pos);
// 		value = tmp.substr(pos + 1);
// 		if (key.empty() || value.empty())
// 			continue;
// 		std::cout << "Before:" << value << std::endl;
// 		DecodeHexaToChar(value);
// 		std::cout << "After:" << value << std::endl;
// 		BodyParams[key] = value;
// 	}
// }

/*	|#----------------------------------#|
	|#			Parse	Chunked    		#|
	|#----------------------------------#|
*/

void	Post::ParseChunked(std::string	stream)
{
}

/*	|#----------------------------------#|
	|#			ParseBoundary	    	#|
	|#----------------------------------#|
*/

void	Post::WriteToFile(std::string	&Buffer)
{
	size_t	FilenamePos = 0, FilenameEndPos = 0, BodyPos = 0;
	std::ofstream		OutFile;
	std::string			Filename, BodyContent;
	std::istringstream	stream(Buffer);

	//------------------	Find SubBodyContent	------------------//
	BodyPos = Buffer.find("\r\n\r\n");
	if (BodyPos == std::string::npos)
		throw "400 Bad Request -WriteToFile()";
	BodyContent = Buffer.substr(BodyPos + 4);
	// std::cout << "BodyContent:{" << BodyContent << "}" << std::endl;


}

void	GetFileName(std::string	&FileName, std::string	&SubBody)
{
	std::ofstream	OutFile;
	std::string		FileNamePath;
	size_t	FilenamePos = 0, FilenameEndPos = 0, BodyPos = 0;

	FilenamePos = SubBody.find("filename=\"");
	if (FilenamePos != std::string::npos) // ila kayn file
	{
		FilenameEndPos = SubBody.find("\"\r\n", FilenamePos + 10);
		if (FilenameEndPos == std::string::npos)
			throw "400 Bad Request -WriteToFile()";

		FileName = SubBody.substr(FilenamePos + 10, FilenameEndPos - (FilenamePos + 10)); // 10 = sizeof("filename=")

		FileName = "Uploads/" + FileName;

		// OutFile.open(FileNamePath.c_str(), std::ios::app); // std::ios::app => to append
		// if (!OutFile.is_open())
		// 	throw "500 Internal Server Error";
		// OutFile << SubBody;
	}
	else // ila makaynch file
		throw "Filename Not Found.";
}

void	Post::GetSubBodys(std::string &Buffer)
{
	std::string	FileName, SubBody;
	size_t	start = 0, end = 0;
	BoundarySettings		Helper = obj.GetBoundarySettings();
	static	BoundaryFlager	Flager;

	//------------------	Check if Request Ended	------------------//
	if (Buffer.find(Helper.BoundaryEnd) != std::string::npos)
		obj.SetClientStatus(EndReading);

	while (true)
	{
		if (!Flager.BoolStart)
		{
			start = Buffer.find(Helper.BoundaryStart, end); // Get Start
			if (start != std::string::npos)
				Flager.BoolStart = true;
		}

		if (!Flager.BoolFile)
		{
			try { GetFileName(FileName, SubBody), Flager.BoolFile = true; } // Get File to write to
				catch (const char *e) { continue; }
		}

		if (!Flager.BoolEnd)
		{
			end = Buffer.find(Helper.BoundaryStart, start + Helper.BoundaryStart.length()); // Get End
			if (end != std::string::npos)
				Flager.BoolEnd = true;
		}

		Flager.CrlfCount = CrlfCounter(Buffer);
		if (Flager.CrlfCount != 5)

		if (Flager.BoolStart && Flager.BoolEnd) // Gathered the SubBody
		{
			size_t	size = end - start;
			SubBody = Buffer.substr(start, size);
		}

		std::cout << "FileName:{" << FileName << "}\n\t->Body:{"<< SubBody << "}\n";
	}
}

void	Post::ParseBoundary(std::string	Body)
{
	static	std::string	FileName;
	std::ofstream		OutFileName;
	BoundarySettings	Helper = obj.GetBoundarySettings();

	// GetSubBodys(Body);

	if (obj.GetTotatlBytesRead() >= obj.GetContentLength())
	{
		obj.SetClientStatus(EndReading);
		throw "200 Success";
	}
}

void	Post::HandlePost()
{
	std::string			Body = obj.GetUnprocessedBuffer();

	switch (obj.GetDataType()) // Chunked || FixedLength
	{
		case FixedLength	:	
		{
			if (obj.GetContentType() == ContentType::Boundary) // To not have a conflict with the (std::string Boundary)
				return ParseBoundary(Body);
			if (obj.GetContentType() == ContentType::Raw)
				;//	Function deyal Raw
			if (obj.GetContentType() == ContentType::Binary)
				;//	Function deyal Binary
			}
		case Chunked		:
		{
			if (obj.GetContentType() == ContentType::Boundary) // To not have a conflict with the (std::string Boundary)
				return ParseBoundary(Body);
			if (obj.GetContentType() == ContentType::Raw)
				;
			if (obj.GetContentType() == ContentType::Binary)
				;
		}
	}
}
