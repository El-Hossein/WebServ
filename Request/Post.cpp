#include "Post.hpp"
#include "Request.hpp"

Post::Post(Request &_obj) : obj(_obj),
							UnprocessedBuffer(_obj.GetBodyBuffer()),
							Boundary(obj.GetBoundarySettings()),
							FirstTime(true),
							RmvFirstCrlf(false),
							BodyFullyRead(false)
{
	BoundaryStatus = None;

	Chunk.ChunkStatus = ChunkVars::None;
	Chunk.BodySize = 0;
	Chunk.Temp = "";

	BoundaryStatus = None;

	PrevBuffer = "";

	Dir = obj.GetFullPath();
	srand(time(NULL));
}

Post::~Post()
{
	OutFile.close();
}

/**
 * @brief check the required headers for the Post method
 * the absence of Content-Type Header -> default Content type is "text/plain; charset=US-ASCII"
 *
 */

void Post::IsBodyFullyRead()
{

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
				if (Buffer.empty())
					return ;
			}
		}
		if (BoundaryStatus == GotBody)
		{
			std::string tmp(Buffer);

			Appender(Buffer, PrevBuffer, tmp);

			size_t end = Buffer.find("\r\n--" + Boundary.Boundary);
			if (end == std::string::npos)
			{
				OutFile.write(PrevBuffer.data(), PrevBuffer.size());
				PrevBuffer = tmp;
				// std::cout << "--->Filename{" << Filename << "}" << std::endl;
				break ;
			}
			else
			{
				BoundaryStatus = GotBodyEnd;

				BodyContent = Buffer.substr(0, end);
				OutFile.write(BodyContent.data(), BodyContent.size());
				PrevBuffer.clear();
				Buffer = Buffer.substr(end);
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
		}
		if (BoundaryStatus == GotBoundaryEnd)
			OutFile.close(), BoundaryStatus = None;
		if (BoundaryStatus == Finished)
		{
			obj.SetClientStatus(EndReading);
			std::cout << "File Uploaded!" << std::endl, throw 201; // "Created"
		}
	}
}

void Post::ParseBoundary()
{
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
		obj.SetClientStatus(EndReading);
		std::cout << "File Uploaded!" << std::endl, throw 201;
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

	// switch (Chunk.ChunkStatus)
	// {
	// 	case ChunkVars::None:
	// 		std::cout << "Status[None]" << std::endl;
	// 		break;
	// 	case ChunkVars::GotHexaSize:
	// 		std::cout << "Status[GotHexaSize]" << std::endl;
	// 		break;
	// 	case ChunkVars::GotFullBody:
	// 		std::cout << "Status[GotFullBody]" << std::endl;
	// 		break;
	// 	case ChunkVars::Finished:
	// 		std::cout << "Status[Finished]" << std::endl;
	// 		break;
	// }
	// std::cout << "---->UnprocessedBuffer:{" << UnprocessedBuffer << "}\n" << std::endl;
	while (true)
	{
		switch (Chunk.ChunkStatus)
		{
			case ChunkVars::None:
			{
				Chunk.Temp.assign(UnprocessedBuffer.data(), UnprocessedBuffer.size());
				Appender(UnprocessedBuffer, PreviousBuffer, Chunk.Temp), PreviousBuffer.clear();

				start = UnprocessedBuffer.find("\r\n", 0);
				if (start == std::string::npos)
				{
					if (!UnprocessedBuffer.empty())
						PreviousBuffer = UnprocessedBuffer;
					return;
				}
				Chunk.BodySize = obj.HexaToInt(UnprocessedBuffer.substr(0, start));
				UnprocessedBuffer = UnprocessedBuffer.substr(start + 2);

				Chunk.ChunkStatus = ChunkVars::GotHexaSize;
				break;
			}
			case ChunkVars::GotHexaSize: // Fix the overflow here
			{
				BodyContent = UnprocessedBuffer.substr(0, Chunk.BodySize);
				WriteChunkToFile(BodyContent);
				UnprocessedBuffer = UnprocessedBuffer.substr(BodyContent.size());

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
				Chunk.Temp.assign(UnprocessedBuffer.data(), UnprocessedBuffer.size());
				Appender(UnprocessedBuffer, PreviousBuffer, Chunk.Temp), PreviousBuffer.clear();
				end = UnprocessedBuffer.find("\r\n");
				if (end != std::string::npos)
				{
					Chunk.ChunkStatus = ChunkVars::None;
					UnprocessedBuffer = UnprocessedBuffer.substr(end + 2);

					if (UnprocessedBuffer.compare(0, 5, "0\r\n\r\n") == 0) // return 0 if strings are equal
						Chunk.ChunkStatus = ChunkVars::Finished;
					break;
				}
				else
					PreviousBuffer = UnprocessedBuffer;
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
	GetChunks();
}

/*	|#----------------------------------#|
	|#		ParseChunkedBoundary	    #|
	|#----------------------------------#|
*/

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
				Chunk.Temp.assign(UnprocessedBuffer.data(), UnprocessedBuffer.size());
				Appender(UnprocessedBuffer, PreviousBuffer, Chunk.Temp), PreviousBuffer.clear();

				start = UnprocessedBuffer.find("\r\n", 0);
				if (start == std::string::npos)
				{
					if (!UnprocessedBuffer.empty())
						PreviousBuffer = UnprocessedBuffer;
					return;
				}
				Chunk.BodySize = obj.HexaToInt(UnprocessedBuffer.substr(0, start));
				UnprocessedBuffer = UnprocessedBuffer.substr(start + 2);

				Chunk.ChunkStatus = ChunkVars::GotHexaSize;
				break;
			}
			case ChunkVars::GotHexaSize:
			{
				BodyContent = UnprocessedBuffer.substr(0, Chunk.BodySize);
				size_t tmp = BodyContent.size();

				GetSubBodies(BodyContent);

				UnprocessedBuffer = UnprocessedBuffer.substr(tmp);
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
				Chunk.Temp.assign(UnprocessedBuffer.data(), UnprocessedBuffer.size());
				Appender(UnprocessedBuffer, PreviousBuffer, Chunk.Temp), PreviousBuffer.clear();
				end = UnprocessedBuffer.find("\r\n");
				if (end != std::string::npos)
				{
					Chunk.ChunkStatus = ChunkVars::None;
					UnprocessedBuffer = UnprocessedBuffer.substr(end + 2);

					if (UnprocessedBuffer.compare(0, 5, "0\r\n\r\n") == 0) // return 0 if strings are equal
						Chunk.ChunkStatus = ChunkVars::Finished;
					break;
				}
				else
					PreviousBuffer = UnprocessedBuffer;
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
	UnprocessedBuffer = obj.GetBodyBuffer();

	std::cout << "Total bytes read: " << obj.GetTotatlBytesRead() << std::endl;
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