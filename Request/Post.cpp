#include "Post.hpp"
#include "Request.hpp"

Post::Post(Request	&_obj) :	obj(_obj),
								UnprocessedBuffer(_obj.GetUnprocessedBuffer()),
								Boundary(obj.GetBoundarySettings()),
								// MaxAllowedBodySize(std::strtod(ConfigNode::getValuesForKey(_obj.GetRightServer(), "client_max_body_size", "NULL")[0].c_str(), NULL)), // [0] First element -> "10M"
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



void	Post::WriteToFile(std::string &Filename, std::string &Buffer)
{
	std::ofstream OutFile;

	OutFile.open(Filename.c_str(), std::ios::binary); // std::ios::app => to append
	if (!OutFile.is_open())
		throw "500 Internal Server Error";
	OutFile.write(Buffer.c_str(), Buffer.size());
}

void	Post::GetSubBodies(std::string &Buffer) // state machine
{
	std::string	Filename, BodyContent;
	size_t	start = 0, end = 0, BodyPos = 0, finish = 0;

	while (true)
	{
		if (BoundaryStatus == None)
		{
			start = Buffer.find(Boundary.BoundaryStart, 0);
			if (start == std::string::npos)
				PrintError("Boudary Error"), throw "400 Bad Request";
			
			// std::string Previous(Buffer, start);
			// if (Previous.size() > 0 && !Filename.empty())
			// 	WriteToFile(Filename, Previous);

			Buffer = Buffer.substr(start + Boundary.BoundaryStart.size());
			BoundaryStatus = GotBoundaryStart;
		}
		if (BoundaryStatus == GotBoundaryStart)
		{
			FindFileName(Buffer, Filename);
			BoundaryStatus = GotFile;
		}
		if (BoundaryStatus == GotFile)
		{
			BodyPos = Buffer.find("\r\n\r\n");
			if (BodyPos == std::string::npos)
				PrintError("No Body Found - No Double CRLF"), throw "400 Bad Request";
			Buffer = Buffer.substr(BodyPos + 4);
			BoundaryStatus = GotBody;
		}
		if (BoundaryStatus == GotBody)
		{
			end = Buffer.find(Boundary.BoundaryStart);
			if (end != std::string::npos)
			{
				BodyContent = Buffer.substr(0, end);
				// Buffer = Buffer.substr(end);
				BoundaryStatus = GotBoundaryEnd;
			}
			
			WriteToFile(Filename, BodyContent);
		}
		if (BoundaryStatus == GotBoundaryEnd)
		{
			std::cout << "---->Filename:{" << Filename << "}\n" << std::endl;
			std::cout << "---->Its BodyContent:{" << BodyContent << "}\n" << std::endl;
			std::cout << "---->Its Buffer:{" << Buffer << "}\n" << std::endl;
			std::cout << "---->Its BoundaryEnd:{" << Boundary.BoundaryEnd<< "}\n" << std::endl;
			if (Buffer.find(Boundary.BoundaryEnd, end) == end) // Check if Request Ended && found the BoundaryEnd
			{
				obj.SetClientStatus(EndReading);
				BoundaryStatus = Finished;
				std::cout << "File Uploaded!" << std::endl, throw "201 Created";
			}
			BoundaryStatus = None;
		}
	}
}

void	Post::ParseBoundary(std::string	Body)
{	
	GetSubBodies(Body);

	if (obj.GetTotatlBytesRead() >= obj.GetContentLength())
	{
		obj.SetClientStatus(EndReading);
		if (BoundaryStatus != Finished)
			throw "400 Bad Request";
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








// void	Post::WriteToFile(std::string	&Buffer, _SubBodyStatus &Status)
// {
// 	size_t				BodyPos = 0;
// 	std::ofstream		OutFile;
// 	std::string			BodyContent, Filename;
// 	static	std::string	FilenamePath;
// 	static	bool		IsFileExist = false;

// 	if (Status == WithNoBoundary) // Pure Body
// 	{
// 		OutFile.open(FilenamePath.c_str(), std::ios::binary); // std::ios::app => to append
// 		if (!OutFile.is_open())
// 			throw "500 Internal Server Error";
// 		OutFile << BodyContent;
// 	}
// 	if (Status != WithNoBoundary)
// 	{
// 		FindFileName(Buffer, Filename);

// 		//------------------	Find SubBodyContent	------------------//
// 		BodyPos = Buffer.find("\r\n\r\n");
// 		if (BodyPos == std::string::npos)
// 			throw "400 Bad Request -WriteToFile() - No double CRLF -";
// 		BodyContent = Buffer.substr(BodyPos + 4);
// 	}
// 	else
// 		BodyContent = Buffer;

// 	if (Flager.BoolFile)
// 	{
// 		OutFile.open(FilenamePath.c_str(), std::ios::binary); // std::ios::app => to append
// 		if (!OutFile.is_open())
// 			throw "500 Internal Server Error";
// 		OutFile << BodyContent;
// 	}
// 	// std::cout << "BodyContent:{" << BodyContent << "}" << std::endl;
// }



	// 	start = Buffer.find(Boundary.BoundaryStart, end);
	// 	if (start != std::string::npos) // start Boundary kayn
	// 	{
	// 		Flager.BoolStart = true;

	// 		end = Buffer.find(Boundary.BoundaryStart, start + Boundary.BoundaryStart.length());
	// 		(end != std::string::npos)	?	Flager.BoolEnd = true : Flager.BoolEnd = false;
	// 	}
	// 	if (!(Flager.BoolStart && Flager.BoolEnd)) // ila makanoch bjouj
	// 		break ;

	// 	size_t size = end - start;

	// 	SubBody = Buffer.substr(start, size);
	// 	SubBodyStatus = WithBothBoundaries; // continue with the loop
	// 	WriteToFile(SubBody, SubBodyStatus);
	// }

	// if (Flager.BoolStart && !Flager.BoolEnd)	SubBodyStatus = WithBoundaryStart;
	// if (!Flager.BoolStart && !Flager.BoolEnd)	SubBodyStatus = WithNoBoundary;

	// if (SubBodyStatus != WithBothBoundaries)
	// {
	// 	std::cout << "\n----->SubBody passed:{" << SubBody << "}" << std::endl;

	// 	WriteToFile(SubBody, SubBodyStatus);
	// }







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
