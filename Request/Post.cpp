#include "Post.hpp"
#include "Request.hpp"

Post::Post(Request	&_obj) :	obj(_obj),
								UnprocessedBuffer(_obj.GetUnprocessedBuffer()),
								Boundary(obj.GetBoundarySettings()),
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
	std::cout << "UnprocessedBuffer-Length:{" << UnprocessedBuffer.size() << "}" << std::endl;

	// if (obj.GetTotatlBytesRead() > this->MaxAllowedBodySize)
	// 	throw "413 Request Entity Too Large";
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
	|#			ParseBoundary	    	#|
	|#----------------------------------#|
*/

void	Post::WriteToFile(std::string	&Buffer)
{
	size_t	FilenamePos = 0, FilenameEndPos = 0, BodyPos = 0;
	std::ofstream		OutFile;
	std::string			Filename, FilenamePath, BodyContent;

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

void	Post::GetSubBodies(std::string &Buffer) // state machine
{
	std::string	FileName, SubBody;
	size_t	start = 0, end = 0;
	static	BoundaryFlager	Flager;

	while (true)
	{
		start = Buffer.find(Boundary.BoundaryStart, end);
		if (start != std::string::npos) // start Boundary kayn
		{
			Flager.BoolStart = true;

			end = Buffer.find(Boundary.BoundaryStart, start + Boundary.BoundaryStart.length());
			(end != std::string::npos)	?	Flager.BoolEnd = true : Flager.BoolEnd = false;
		}

		if (Flager.BoolStart && Flager.BoolEnd)
		{
			size_t size = end - start;
			
			SubBody = Buffer.substr(start, size);
			
			std::cout << "\n----->SubBody passed:{" << SubBody << "}" << std::endl;
			std::cout << CrlfCounter(SubBody) << std::endl;
			
			SubBodyStatus = WithBothBoundaries; // continue with the loop
		}
		if (Flager.BoolStart && !Flager.BoolEnd)
			SubBodyStatus = WithBoundaryStart;
		if (!Flager.BoolStart && !Flager.BoolEnd)
			SubBodyStatus = WithNoBoundary;

		std::cout << "here\n";
		
		switch (SubBodyStatus)
		{
			case WithBothBoundaries	: WriteToFile(SubBody); break ;
			case WithBoundaryStart	: PrintError("\nWithBoundaryStart Error!\n"), exit(1);
			case WithNoBoundary		: PrintError("\nWithNoBoundary Error!\n"), exit(1);
		}
	}
	//------------------	Check if Request Ended	------------------//
	if (Buffer.find(Boundary.BoundaryEnd) != std::string::npos)
		obj.SetClientStatus(EndReading);
}

void	Post::ParseBoundary(std::string	Body)
{
	static	std::string	FileName;
	std::ofstream		OutFileName;
	
	GetSubBodies(Body);
	
	if (obj.GetTotatlBytesRead() >= obj.GetContentLength())
	{
		obj.SetClientStatus(EndReading);
		throw "200 Success";
	}
}

void	Post::HandlePost()
{
	switch (obj.GetDataType()) // Chunked || FixedLength
	{
		case FixedLength	:	
		{
			if (obj.GetContentType() == ContentType::Boundary) // To not have a conflict with the (std::string Boundary)
				return ParseBoundary(UnprocessedBuffer);
			if (obj.GetContentType() == ContentType::Raw)
				;//	Function deyal Raw
			if (obj.GetContentType() == ContentType::Binary)
				;//	Function deyal Binary
			}
		case Chunked		:
		{
			if (obj.GetContentType() == ContentType::Boundary) // To not have a conflict with the (std::string Boundary)
				return ParseBoundary(UnprocessedBuffer);
			if (obj.GetContentType() == ContentType::Raw)
				;
			if (obj.GetContentType() == ContentType::Binary)
				;
		}
	}
}
















	// while (true)
	// {
	// 	start = Buffer.find(Boundary.BoundaryStart, end);
	// 	if (start != std::string::npos) // start Boundary kayn
	// 	{
	// 		Flager.BoolStart = true;

	// 		end = Buffer.find(Boundary.BoundaryStart, start + Boundary.BoundaryStart.length());
	// 		(end != std::string::npos)	?	Flager.BoolEnd = true : Flager.BoolEnd = false;
	// 	}

	// 	if (Flager.BoolStart && Flager.BoolEnd)
	// 	{
	// 		size_t size = end - start;
	// 		size += Boundary.BoundaryStart.length();
	// 		SubBody = Buffer.substr(start, size);
	// 		if (CrlfCounter(SubBody) != 5)
	// 			throw "400 Bad Request";

	// 		SubBodyStatus = WithBothBoundaries; // continue with the loop
	// 	}
	// 	else if (Flager.BoolStart && !Flager.BoolEnd) // SubBody = From Boundary start to end of buffer
	// 	{
	// 		SubBody	= Buffer.substr(start);
	// 		SubBodyStatus = WithBoundaryStart;
	// 		end		= start + Boundary.BoundaryStart.length(); // in case end == ::npos -> n7et end f lkhr d buffer
	// 	}
	// 	else if (!Flager.BoolStart && !Flager.BoolEnd)
	// 	{
	// 		SubBody = Buffer;
	// 		SubBodyStatus = WithNoBoundary;
	// 		end		= 0;
	// 	}
		
	// }










// void	Post::WriteToFile(std::string	&Buffer)
// {
// 	size_t	FilenamePos = 0, FilenameEndPos = 0, BodyPos = 0;
// 	std::ofstream		OutFile;
// 	std::string			Filename, BodyContent;
// 	std::istringstream	stream(Buffer);

// 	//------------------	Find SubBodyContent	------------------//
// 	BodyPos = Buffer.find("\r\n\r\n");
// 	if (BodyPos == std::string::npos)
// 		throw "400 Bad Request -WriteToFile()";
// 	BodyContent = Buffer.substr(BodyPos + 4);
// 	// std::cout << "BodyContent:{" << BodyContent << "}" << std::endl;


// }

// void	GetFileName(std::string	&FileName, std::string	&SubBody)
// {
// 	std::ofstream	OutFile;
// 	std::string		FileNamePath;
// 	size_t	FilenamePos = 0, FilenameEndPos = 0, BodyPos = 0;

// 	FilenamePos = SubBody.find("filename=\"");
// 	if (FilenamePos != std::string::npos) // ila kayn file
// 	{
// 		FilenameEndPos = SubBody.find("\"\r\n", FilenamePos + 10);
// 		if (FilenameEndPos == std::string::npos)
// 			throw "400 Bad Request -WriteToFile()";

// 		FileName = SubBody.substr(FilenamePos + 10, FilenameEndPos - (FilenamePos + 10)); // 10 = sizeof("filename=")

// 		FileName = "Uploads/" + FileName;

// 		// OutFile.open(FileNamePath.c_str(), std::ios::app); // std::ios::app => to append
// 		// if (!OutFile.is_open())
// 		// 	throw "500 Internal Server Error";
// 		// OutFile << SubBody;
	// }
// 	else // ila makaynch file
// 		throw "Filename Not Found.";
// }
