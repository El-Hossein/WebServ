#include "Post.hpp"
#include "Request.hpp"

Post::Post(Request	&_obj) :	obj(_obj),
								UnprocessedBuffer(_obj.GetUnprocessedBuffer()),
								Boundary(obj.GetBoundarySettings()),
								// MaxAllowedBodySize(std::strtod(ConfigNode::getValuesForKey(_obj.GetRightServer(), "client_max_body_size", "NULL")[0].c_str(), NULL)), // [0] First element -> "10M"
								FirstTime(true),
								BodyFullyRead(false)
{
	Flager.BoolStart = false;
	Flager.BoolEnd = false;
	Flager.BoolFile = false;
	Flager.CrlfCount = 0;

	Chunk.ChunkStatus = ChunkVars::None;
	Chunk.BodySize = 0;
	
	Dir = "/Users/zderfouf/goinfre/ServerUploads";
	srand(time(NULL));
}

Post::~Post() {
	OutFile.close();
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

void	Post::FindFileName(std::string	&Buffer, std::string	&Filename)
{
	size_t	FilenamePos = 0, FilenameEndPos = 0;

	FilenamePos = Buffer.find("filename=\"", 0);
	FilenameEndPos = Buffer.find("\"\r\n", FilenamePos + 10);

	if (FilenamePos == std::string::npos || FilenameEndPos == std::string::npos)
	{
		PrintError("Could't find file"), throw 400;
	}
	Filename = Buffer.substr(FilenamePos + 10, FilenameEndPos - (FilenamePos + 10)); // 10 = sizeof("filename=")

	CreateDirectory(Dir);
	Filename = Dir + "/" + Filename; // "/Users/zderfouf/goinfre/ServerUploads" --- Uploads

	OutFile.open(Filename.c_str(), std::ios::binary); // std::ios::app => to append
	if (!OutFile.is_open())
		PrintError("Could't open file"), throw 500; // Internal Server Error
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

void	Post::GetSubBodies(std::string &Buffer) // state machine
{
	std::string	BodyContent;
	size_t	start = 0, end = 0, BodyPos = 0;

	// std::cout << "Buffer{" << Buffer << "}" << std::endl;
	// switch (BoundaryStatus)
	// {
	// 	case None : std::cout << "its Boundary Status[None]" << std::endl; break ;
	// 	case GotBoundaryStart : std::cout << "its Boundary Status[GotBoundaryStart]" << std::endl; break ;
	// 	case GotFile : std::cout << "its Boundary Status[GotFile]" << std::endl; break ;
	// 	case GotBody : std::cout << "its Boundary Status[GotBody]" << std::endl; break ;
	// 	case GotBoundaryEnd : std::cout << "its Boundary Status[GotBoundaryEnd]" << std::endl; break ;
	// 	case Finished : std::cout << "its Boundary Status[Finished]" << std::endl; break ;
	// }
	
	while (true)
	{
		if (BoundaryStatus == None)
		{
			start = Buffer.find(Boundary.BoundaryStart, 0);
			if (start == std::string::npos)
				PrintError("Boudary Error"), throw 400;

			// std::string Previous(Buffer, start);
			// if (Previous.size() > 0 && !Filename.empty())
			// 	WriteToFile(Filename, Previous);

			Buffer.erase(0, start + Boundary.BoundaryStart.size());
			BoundaryStatus = GotBoundaryStart;
		}
		if (BoundaryStatus == GotBoundaryStart)
		{
			FindFileName(Buffer, Filename);
			// Buffer.erase(0, TrimBody + 3);
			BoundaryStatus = GotFile;
		}
		if (BoundaryStatus == GotFile)
		{
			BodyPos = Buffer.find("\r\n\r\n");
			if (BodyPos == std::string::npos) // ila makantch
			{
				PrintError("No Body Found"), throw 400;
			}
			else // kayn Double CRLF
			{
				Buffer.erase(0, BodyPos + 4);
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

			OutFile.write(BodyContent.c_str(), BodyContent.size()); // WriteToFile(Filename, BodyContent);
			// std::cout << "---->Filename:{" << Filename << "}" << std::endl;
			// std::cout << "---->Its BodyContent:{" << BodyContent << "}\n" << std::endl;

			if (BoundaryStatus != GotBoundaryEnd)
				break ;
		}
		if (BoundaryStatus == GotBoundaryEnd)
		{
			OutFile.close();
			// std::cout << "---->Its Buffer:{"; PrintCrlfString(Buffer) ; std::cout << "}\n" << std::endl;
			// std::cout << "---->Its BoundaryEnd:{"; PrintCrlfString(Boundary.BoundaryEnd) ; std::cout << "}\n" << std::endl;
			if (Buffer.find(Boundary.BoundaryStart) == Buffer.find(Boundary.BoundaryEnd)) // Check if Request Ended && found the BoundaryEnd
			{
				obj.SetClientStatus(EndReading);
				BoundaryStatus = Finished;
				std::cout << "File Uploaded!" << std::endl, throw 201; // "Created"
			}
			BoundaryStatus = None;
		}
	}
}

void	Post::ParseBoundary()
{	
	std::cout << obj.GetTotatlBytesRead() << "--" << obj.GetContentLength() << std::endl;
	GetSubBodies(UnprocessedBuffer);

	if (obj.GetTotatlBytesRead() >= obj.GetContentLength())
	{
		std::cout << "$$$$$$\n\n";
		obj.SetClientStatus(EndReading);
		if (BoundaryStatus != Finished)
			PrintError("Incomplete Request"), throw 400;
	}
}

void	Post::ParseBirnaryOrRaw()
{
	if (FirstTime)
		OutFile.open(Dir + "/" + RandomString() + obj.GetFileExtention(), std::ios::binary), FirstTime = false;

	OutFile.write(UnprocessedBuffer.c_str(), UnprocessedBuffer.size());

	if (obj.GetTotatlBytesRead() >= obj.GetContentLength())
	{
		OutFile.close();
		std::cout << "File Uploaded!" << std::endl, throw 201;
		obj.SetClientStatus(EndReading);
	}
}

/*	|#----------------------------------#|
	|#			ParseChunked	    	#|
	|#----------------------------------#|
*/

void	Post::WriteChunkToFile(std::string &BodyContent)
{
	if (FirstTime)
	{
		CreateDirectory(Dir);
		OutFile.open(Dir + "/" + RandomString() + obj.GetFileExtention(), std::ios::binary), FirstTime = false;
		if (!OutFile.is_open())
			PrintError("Could't open file"), throw 500; // Internal Server Error
	}

	OutFile.write(BodyContent.c_str(), BodyContent.size());
}

void	Post::GetChunks()
{
	size_t		start = 0, end = 0;
	std::string	BodyContent;

	switch (Chunk.ChunkStatus)
	{
		case ChunkVars::None : std::cout << "Status[None]" << std::endl; break ;
		case ChunkVars::GotHexaSize : std::cout << "Status[GotHexaSize]" << std::endl; break ;
		case ChunkVars::GotFullBody : std::cout << "Status[GotFullBody]" << std::endl; break ;
		case ChunkVars::Finished : std::cout << "Status[Finished]" << std::endl; break ;
	}
	// std::cout << "---->UnprocessedBuffer:{" << UnprocessedBuffer << "}\n" << std::endl;
	while (true)
	{
		switch (Chunk.ChunkStatus)
		{
			case	ChunkVars::None		:
			{
				start = UnprocessedBuffer.find("\r\n", 0);
				if (start == std::string::npos)
					return ;

				Chunk.BodySize = HexaToInt(UnprocessedBuffer.substr(0, start)); // Body size b hexa value
				UnprocessedBuffer.erase(0, start + 2);
				std::cout << "------>HexaSize To read:{" << Chunk.BodySize << "}" << std::endl;
				Chunk.ChunkStatus = ChunkVars::GotHexaSize; break;
			}
			case	ChunkVars::GotHexaSize	:
			{
				BodyContent = UnprocessedBuffer.substr(0, Chunk.BodySize);
				std::cout << BodyContent.size() << std::endl;
				std::cout << "------>Bodycontent:{" << BodyContent << "}" << std::endl;
				std::cout << "------>UnprocessedBuffer:{" << UnprocessedBuffer << "}" << std::endl;

				WriteChunkToFile(BodyContent);
				UnprocessedBuffer.erase(0, BodyContent.size());

				Chunk.BodySize -= BodyContent.size();
				std::cout << "LeftOverBytes Neeeded:[" << Chunk.BodySize << "]\n";

				if (!Chunk.BodySize)
				{
					Chunk.ChunkStatus = ChunkVars::GotFullBody;
					break ;
				}
				return ;
			}
			case	ChunkVars::GotFullBody	: // look for the CRLF (end of chunk) || look for the end of Request
			{
				end = UnprocessedBuffer.find("\r\n", 0);
				if (end != std::string::npos)
				{
					Chunk.ChunkStatus = ChunkVars::None;
					UnprocessedBuffer.erase(0, end + 2);

					if (UnprocessedBuffer.compare(0, 5, "0\r\n\r\n") == 0) // return 0 if strings are equal
						Chunk.ChunkStatus = ChunkVars::Finished;
					std::cout << "\nNone again\n";
					break ;
				}
				return ; // return to wait for the FullBody end 
			}
			case	ChunkVars::Finished	:
			{
				std::cout << "\n\n\n####	Finished	####\n\n\n";
				this->OutFile.close();
				obj.SetClientStatus(EndReading);
				std::cout << "File Uploaded!" << std::endl, throw 201;
			}
		}
	}
}

void	Post::ParseChunked()
{
	GetChunks();
}

void	Post::SetUnprocessedBuffer()
{
	UnprocessedBuffer = obj.GetUnprocessedBuffer();
	if (obj.GetContentType() == BinaryOrRaw)
		UnprocessedBuffer.erase(0, 2);
}

void	Post::HandlePost()
{
	SetUnprocessedBuffer();
	switch (obj.GetDataType()) // Chunked || FixedLength
	{
		case FixedLength	:	
		{
			if (obj.GetContentType() == _Boundary) // To not have a conflict with the (std::string Boundary)
				return ParseBoundary();
			if (obj.GetContentType() == BinaryOrRaw)
				return ParseBirnaryOrRaw();
		}
		case Chunked		:
		{
			if (obj.GetContentType() == _Boundary)
				; // return ParseChunkedBoundary()
			if (obj.GetContentType() == BinaryOrRaw)
				return ParseChunked();
		}
	}
}






		// start = UnprocessedBuffer.find("\r\n", 0);
		// if (start == std::string::npos)
		// 	throw "400 Bad Request";
		// HexaStr = UnprocessedBuffer.substr(0, start);
		// UnprocessedBuffer.erase(0, start + 2);
		// BodySize = HexaToInt(HexaStr); // Body size b hexa value
		// std::cout << "---->HexaStr:{" << HexaStr << "}" << std::endl;

		// BodyContent = UnprocessedBuffer.substr(0, BodySize);
		// UnprocessedBuffer.erase(0, BodySize);
		// if (BodyContent.size() == BodySize)
		// {
		// 	end = UnprocessedBuffer.find("\r\n", 0);
		// 	if (end == std::string::npos)
		// 		; // mal9itch CRLF donc Body mazal masala, wait for next chunk
		// 	else
		// 		; // salit chunk go to next
		// }



		// std::cout << "---->BodyContent:{" << BodyContent << "}" << std::endl;

		// size_t  finish = UnprocessedBuffer.find("0\r\n\r\n");
		// std::cout << end << "-" << finish << std::endl;
		// if (end + 2 == finish)
		// {
		// 	obj.SetClientStatus(EndReading);
		// 	std::cout << "File Uploaded!" << std::endl, throw "201 Created";
		// }
		// UnprocessedBuffer.erase(0, end + 2);