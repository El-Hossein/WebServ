#include "Post.hpp"
#include "Request.hpp"

Post::Post(Request	&_obj) :	obj(_obj),
								UnprocessedBuffer(_obj.GetUnprocessedBuffer()),
								Boundary(obj.GetBoundarySettings()),
								// MaxAllowedBodySize(std::strtod(ConfigNode::getValuesForKey(_obj.GetRightServer(), "client_max_body_size", "NULL")[0].c_str(), NULL)), // [0] First element -> "10M"
								EndOfRequest(false),
								BodyFullyRead(false)
{
	Flager.BoolStart = false;
	Flager.BoolEnd = false;
	Flager.BoolFile = false;
	Flager.CrlfCount = 0;
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
	std::string	BodyContent;
	size_t	start = 0, end = 0, BodyPos = 0, finish = 0;

	std::cout << "Buffer{" << Buffer << "}" << std::endl;
	switch (BoundaryStatus)
	{
		case None : std::cout << "its Boundary Status[None]" << std::endl; break ;
		case GotBoundaryStart : std::cout << "its Boundary Status[GotBoundaryStart]" << std::endl; break ;
		case GotFile : std::cout << "its Boundary Status[GotFile]" << std::endl; break ;
		case GotBody : std::cout << "its Boundary Status[GotBody]" << std::endl; break ;
		case GotBoundaryEnd : std::cout << "its Boundary Status[GotBoundaryEnd]" << std::endl; break ;
		case Finished : std::cout << "its Boundary Status[Finished]" << std::endl; break ;
	}
	
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

			Buffer.erase(0, start + Boundary.BoundaryStart.size());
			BoundaryStatus = GotBoundaryStart;
		}
		if (BoundaryStatus == GotBoundaryStart)
		{
			size_t TrimBody = FindFileName(Buffer, Filename);
			// Buffer.erase(0, TrimBody + 3);
			std::cout << "1";PrintCrlfString(Buffer);
			BoundaryStatus = GotFile;
		}
		if (BoundaryStatus == GotFile)
		{
			std::cout << "2";PrintCrlfString(Buffer);
			BodyPos = Buffer.find("\r\n\r\n");
			if (BodyPos == std::string::npos) // ila makantch
			{
				std::cout << "3";PrintCrlfString(Buffer);
				std::cout << "#####Buffer where there is no 2 CRLF:{" << Buffer << "}\n\n";
				PrintError("No Body Found - No Double CRLF"), throw "400 Bad Request";
			}
			else // kayn Double CRLF
			{
				Buffer.erase(0, BodyPos + 4);
				std::cout << "66";PrintCrlfString(Buffer);
				BoundaryStatus = GotBody;
			}
		}
		if (BoundaryStatus == GotBody)
		{

			end = Buffer.find(Boundary.BoundaryStart);
			if (end != std::string::npos)
			{
				BodyContent = Buffer.substr(0, end);
				Buffer.erase(0, end);
				BoundaryStatus = GotBoundaryEnd;
				// std::cout << "FilenameBody{" <<  BodyContent << "}\n" << std::endl; // WriteToFile(Filename, BodyContent);
			}
			else
				BodyContent = Buffer;
			// std::cout << "Body to write to Filename{" <<  BodyContent << "}\n" << std::endl; 
			if (!BodyContent.empty())
			{
				WriteToFile(Filename, BodyContent);
				std::cout << "---->Filename:{" << Filename << "}" << std::endl;
				std::cout << "---->Its BodyContent:{" << BodyContent << "}\n" << std::endl;
			}

			if (BoundaryStatus != GotBoundaryEnd)
				break ;
		}
		if (BoundaryStatus == GotBoundaryEnd)
		{
			std::cout << "---->Its Buffer:{"; PrintCrlfString(Buffer) ; std::cout << "}\n" << std::endl;
			std::cout << "---->Its BoundaryEnd:{"; PrintCrlfString(Boundary.BoundaryEnd) ; std::cout << "}\n" << std::endl;

			size_t StartPos = Buffer.find(Boundary.BoundaryStart);
			size_t EndPos = Buffer.find(Boundary.BoundaryEnd);
			std::cout << StartPos << "----" << EndPos << std::endl;

			if (EndPos == StartPos) // Check if Request Ended && found the BoundaryEnd
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

	std::cout << obj.GetTotatlBytesRead() << "--" << obj.GetContentLength() << std::endl;
	if (obj.GetTotatlBytesRead() >= obj.GetContentLength())
	{
		std::cout << "$$$$$$\n\n";
		obj.SetClientStatus(EndReading);
		// if (BoundaryStatus != Finished)
		// 	throw "400 Bad Request";
	}
}

void	Post::HandlePost()
{
	UnprocessedBuffer = obj.GetUnprocessedBuffer();

	switch (obj.GetDataType()) // Chunked || FixedLength
	{
		case FixedLength	:	
		{
			if (obj.GetContentType() == _Boundary) // To not have a conflict with the (std::string Boundary)
				return ParseBoundary(UnprocessedBuffer);
			if (obj.GetContentType() == Raw)
				;//	Function deyal Raw
			if (obj.GetContentType() == Binary)
				;//	Function deyal Binary
		}
		case Chunked		:
		{
			if (obj.GetContentType() == _Boundary) // To not have a conflict with the (std::string Boundary)
				return ParseBoundary(UnprocessedBuffer);
			if (obj.GetContentType() == Raw)
				;
			if (obj.GetContentType() == Binary)
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
