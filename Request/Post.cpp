#include "Post.hpp"
#include "Request.hpp"

Post::Post(Request &_obj) : obj(_obj),
							UnprocessedBuffer(_obj.GetUnprocessedBuffer()),
							Boundary(obj.GetBoundarySettings()),
							// MaxAllowedBodySize(std::strtod(ConfigNode::getValuesForKey(_obj.GetRightServer(), "client_max_body_size", "NULL")[0].c_str(), NULL)), // [0] First element -> "10M"
							FirstTime(true),
							RmvFirstCrlf(false),
							BodyFullyRead(false)
{
	Flager.BoolStart = false;
	Flager.BoolEnd = false;
	Flager.BoolFile = false;
	Flager.CrlfCount = 0;

	BoundaryStatus = None;

	Chunk.ChunkStatus = ChunkVars::None;
	Chunk.BodySize = 0;

	BoundaryStatus = None;

	AccumulateBuffer = "";

	Dir = "/Users/zderfouf/goinfre/ServerUploads";
	srand(time(NULL));
}

Post::~Post()
{
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

void Post::IsBodyFullyRead()
{
	MaxAllowedBodySize = 1028 * 10; // tmp value = 10280 -> 10MegaBytes

	std::cout << "Content-Length:{" << obj.GetContentLength() << "}" << std::endl;
	std::cout << "UnprocessedBuffer-Length:{" << UnprocessedBuffer.size() << "}" << std::endl;

	// if (obj.GetTotatlBytesRead() > this->MaxAllowedBodySize)
	// 	throw "413 Request Entity Too Large";
}

void Post::FindFileName(std::string &Buffer, std::string &Filename)
{
	size_t FilenamePos = 0, FilenameEndPos = 0;

	FilenamePos = Buffer.find("filename=\"", 0);
	FilenameEndPos = Buffer.find("\"\r\n", FilenamePos + 10);

	if (FilenamePos == std::string::npos || FilenameEndPos == std::string::npos)
	{
		obj.PrintError("Could't find file", obj), throw 400;
	}
	Filename = Buffer.substr(FilenamePos + 10, FilenameEndPos - (FilenamePos + 10)); // 10 = sizeof("filename=")

	obj.CreateDirectory(Dir);
	Filename = Dir + "/" + Filename; // "/Users/zderfouf/goinfre/ServerUploads" --- Uploads

	OutFile.open(Filename.c_str(), std::ios::binary); // std::ios::app => to append
	if (!OutFile.is_open())
		obj.PrintError("Could't open file", obj), throw 500; // Internal Server Error
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

void Post::GetSubBodies(std::string &Buffer) // state machine
{
	std::string BodyContent;
	size_t start = 0, end = 0, BodyPos = 0;

	// std::cout << "\n\nBuffer{" << Buffer << "}" << std::endl;
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
			// std::cout << "\n****" << Buffer.substr(0, 10) << "**********\n";
			if (start == std::string::npos)
				obj.PrintError("Boudary Error", obj), throw 400;

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
				obj.PrintError("No Body Found", obj), throw 400;
			else // kayn Double CRLF
			{
				Buffer.erase(0, BodyPos + 4), BoundaryStatus = GotBody; // Go next
			}
		}
		if (BoundaryStatus == GotBody)
		{
			// AccumulateBuffer += Buffer;
			AccumulateBuffer.append(Buffer.data(), Buffer.size());

			std::string str = "\r\n--" + Boundary.Boundary;
			size_t end = AccumulateBuffer.find(str);
			if (end != std::string::npos)
			{
				BoundaryStatus = GotBodyEnd;
				BodyContent = AccumulateBuffer.substr(0, end);

				std::cout << "Size to write to file:"<< BodyContent.size() << std::endl;
				OutFile.write(BodyContent.c_str(), BodyContent.size());

				Buffer = AccumulateBuffer.substr(end);
				AccumulateBuffer.clear();
			}
			else
			{
				BodyContent = AccumulateBuffer.substr(0, Buffer.size());
				AccumulateBuffer = AccumulateBuffer.substr(Buffer.size());
				OutFile.write(BodyContent.c_str(), BodyContent.size());
				break ;
			}
		}
		if (BoundaryStatus == GotBodyEnd)
		{
			size_t pos;
			pos = Buffer.find(Boundary.BoundaryStart);
			if (pos != std::string::npos)
				BoundaryStatus = GotBoundaryEnd;
			pos = Buffer.find(Boundary.BoundaryEnd);
			if (pos != std::string::npos)
				BoundaryStatus = Finished;

			// if (BoundaryStatus == GotBodyEnd)
			// 	break;
		}
		if (BoundaryStatus == GotBoundaryEnd)
			OutFile.close(), BoundaryStatus = None;
		if (BoundaryStatus == Finished)
		{
			obj.SetClientStatus(EndReading);
			std::cout << "File Uploaded! In SubBodies()" << std::endl, throw 201; // "Created"
		}
	}
}

void Post::ParseBoundary()
{
	std::cout << obj.GetTotatlBytesRead() << "--" << obj.GetContentLength() << std::endl;
	GetSubBodies(UnprocessedBuffer);

	if (obj.GetTotatlBytesRead() >= obj.GetContentLength())
	{
		obj.SetClientStatus(EndReading);
		if (BoundaryStatus != Finished)
			obj.PrintError("Incomplete Request", obj), throw 400;
	}
}

void Post::ParseBirnaryOrRaw()
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

void Post::WriteChunkToFile(std::string &BodyContent)
{
	if (FirstTime)
	{
		obj.CreateDirectory(Dir);
		OutFile.open(Dir + "/" + RandomString() + obj.GetFileExtention(), std::ios::binary), FirstTime = false;
		if (!OutFile.is_open())
			obj.PrintError("Could't open file", obj), throw 500; // Internal Server Error
	}

	OutFile.write(BodyContent.c_str(), BodyContent.size());
}

void Post::GetChunks()
{
	size_t start = 0, end = 0;
	std::string BodyContent;

	switch (Chunk.ChunkStatus)
	{
	case ChunkVars::None:
		std::cout << "Status[None]" << std::endl;
		break;
	case ChunkVars::GotHexaSize:
		std::cout << "Status[GotHexaSize]" << std::endl;
		break;
	case ChunkVars::GotFullBody:
		std::cout << "Status[GotFullBody]" << std::endl;
		break;
	case ChunkVars::Finished:
		std::cout << "Status[Finished]" << std::endl;
		break;
	}
	// std::cout << "---->UnprocessedBuffer:{" << UnprocessedBuffer << "}\n" << std::endl;
	while (true)
	{
		switch (Chunk.ChunkStatus)
		{
		case ChunkVars::None:
		{
			start = UnprocessedBuffer.find("\r\n", 0);
			if (start == std::string::npos)
				return;
			// std::cout << "{" << UnprocessedBuffer.substr(0, 7) << "}" << std::endl;
			Chunk.BodySize = obj.HexaToInt(UnprocessedBuffer.substr(0, start));
			UnprocessedBuffer.erase(0, start + 2);
			Chunk.ChunkStatus = ChunkVars::GotHexaSize;
			break;
		}
		case ChunkVars::GotHexaSize:
		{
			BodyContent = UnprocessedBuffer.substr(0, Chunk.BodySize);
			WriteChunkToFile(BodyContent);
			UnprocessedBuffer.erase(0, BodyContent.size());

			Chunk.BodySize -= BodyContent.size();
			if (!Chunk.BodySize)
			{
				Chunk.ChunkStatus = ChunkVars::GotFullBody;
				break;
			}
			return;
		}
		case ChunkVars::GotFullBody:
		{
			end = UnprocessedBuffer.find("\r\n", 0);
			if (end != std::string::npos)
			{
				Chunk.ChunkStatus = ChunkVars::None;
				UnprocessedBuffer.erase(0, end + 2);

				if (UnprocessedBuffer.compare(0, 5, "0\r\n\r\n") == 0) // return 0 if strings are equal
					Chunk.ChunkStatus = ChunkVars::Finished;
				break;
			}
			return; // return to wait for the FullBody end
		}
		case ChunkVars::Finished:
		{
			this->OutFile.close();
			obj.SetClientStatus(EndReading);
			std::cout << "File Uploaded!" << std::endl, throw 201;
		}
		}
	}
}

void Post::ParseChunked()
{
	std::cout << obj.GetTotatlBytesRead() << "--" << obj.GetContentLength() << std::endl;
	GetChunks();
}

/*	|#----------------------------------#|
	|#		ParseChunkedBoundary	    #|
	|#----------------------------------#|
*/

void Post::GetSubBodies2(std::string &Buffer) // state machine
{
	std::string BodyContent;
	size_t start = 0, end = 0, BodyPos = 0;

	// std::cout << "Buffer:{" << Buffer << "}\n" << std::endl;

	while (true)
	{
		if (BoundaryStatus == None)
		{
			start = Buffer.find(Boundary.BoundaryStart, 0);
			if (start == std::string::npos)
			{
				std::cout << "Buffer in [None]:" << Buffer << std::endl;
				obj.PrintError("Boudary Error", obj), throw 400;
			}

			Buffer.erase(0, start + Boundary.BoundaryStart.size());
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
			if (BodyPos == std::string::npos) // ila makantch
				obj.PrintError("No Body Found", obj), throw 400;
			else // kayn Double CRLF
			{
				Buffer.erase(0, BodyPos + 4), BoundaryStatus = GotBody; // Go next
			}
		}
		if (BoundaryStatus == GotBody)
		{
			size_t end;
			AccumulateBuffer += Buffer;

			end = AccumulateBuffer.find("\r\n" + Boundary.BoundaryStart); // look for boundary start
			if (end != std::string::npos)
				BoundaryStatus = GotBoundaryEnd;
			else // look for boundary end
			{
				end = AccumulateBuffer.find("\r\n" + Boundary.BoundaryEnd);
				if (end != std::string::npos)
					BoundaryStatus = Finished;
				else
					break;
			}

			if (BoundaryStatus != GotBody)
				AccumulateBuffer.clear();
			BodyContent = AccumulateBuffer.substr(0, end);
			Buffer = AccumulateBuffer.substr(BodyContent.size());

			// std::cout << "size of BodyContent:{" << BodyContent.size() << "}***BodyContent^^"<< BodyContent << "^^\n\n" << std::endl;

			// std::cout << "BodyContent:{" << BodyContent<< "}\n------------------\n" << std::endl;
			// static int i = 0;
			// i++;
			// if (i == 2)
			// 	exit(12);
			OutFile.write(BodyContent.c_str(), BodyContent.size());
			// std::cout << "---->Filename:{" << Filename << "}" << std::endl;
			if (BoundaryStatus == GotBody)
				break;
		}
		if (BoundaryStatus == GotBoundaryEnd)
			OutFile.close(), BoundaryStatus = None;
		if (BoundaryStatus == Finished)
		{
			std::cout << "File Uploaded! subBodies" << std::endl, obj.SetClientStatus(EndReading), throw 201;
		}
	}
}

void Post::ParseChunkedBoundary()
{
	size_t start = 0, end = 0;
	std::string BodyContent;

	while (true)
	{
		switch (Chunk.ChunkStatus)
		{
		case ChunkVars::None:
		{
			// std::cout << "In None\n";
			UnprocessedBuffer = PreviousBuffer + UnprocessedBuffer, PreviousBuffer.clear();
			start = UnprocessedBuffer.find("\r\n", 0);
			if (start == std::string::npos)
			{
				if (!UnprocessedBuffer.empty())
				{
					PreviousBuffer = UnprocessedBuffer;
					// std::cout << "Previous Buffer:|" << PreviousBuffer << "|\n";
				}
				// std::cout << "CRLF not found in None:{" <<  UnprocessedBuffer.substr(0, 10) << "}" << std::endl;
				return;
			}
			// std::cout << "Buffer to look for hexa:{" <<  UnprocessedBuffer.substr(0, 10) << "}" << std::endl;
			// std::cout << "Hexa found:{" << UnprocessedBuffer.substr(0, 20) << "}" << std::endl;
			Chunk.BodySize = obj.HexaToInt(UnprocessedBuffer.substr(0, start));
			UnprocessedBuffer.erase(0, start + 2);
			Chunk.ChunkStatus = ChunkVars::GotHexaSize;
			break;
		}
		case ChunkVars::GotHexaSize:
		{
			// std::cout << "In GotHexaSize\n";
			BodyContent = UnprocessedBuffer.substr(0, Chunk.BodySize);
			size_t tmp = BodyContent.size();

			GetSubBodies(BodyContent);

			UnprocessedBuffer = UnprocessedBuffer.substr(tmp);

			std::cout << Chunk.BodySize << "-------" << tmp << std::endl;
			Chunk.BodySize -= tmp;
			if (!Chunk.BodySize)
			{
				Chunk.ChunkStatus = ChunkVars::GotFullBody;
				break;
			}
			return;
		}
		case ChunkVars::GotFullBody:
		{
			end = UnprocessedBuffer.find("\r\n");
			if (end != std::string::npos)
			{
				Chunk.ChunkStatus = ChunkVars::None;
				UnprocessedBuffer = UnprocessedBuffer.substr(end + 2);

				if (UnprocessedBuffer.compare(0, 5, "0\r\n\r\n") == 0) // return 0 if strings are equal
				{
					Chunk.ChunkStatus = ChunkVars::Finished;
				}
				break;
			}
			return; // return to wait for the FullBody end
		}
		case ChunkVars::Finished:
		{
			std::cout << "File Uploaded!" << std::endl, obj.SetClientStatus(EndReading), throw 201;
		}
		}
	}
}

/*	|#----------------------------------#|
	|#			ParsingMenu		    	#|
	|#----------------------------------#|
*/

void Post::HandlePost()
{
	UnprocessedBuffer = obj.GetUnprocessedBuffer();
	switch (obj.GetDataType()) // Chunked || FixedLength
	{
	case FixedLength:
	{
		if (obj.GetContentType() == _Boundary) // To not have a conflict with the (std::string Boundary)
			return ParseBoundary();
		if (obj.GetContentType() == BinaryOrRaw)
			return ParseBirnaryOrRaw();
	}
	case Chunked:
	{
		if (obj.GetContentType() == _Boundary)
			return ParseChunkedBoundary();
		if (obj.GetContentType() == BinaryOrRaw)
			return ParseChunked();
	}
	}
}