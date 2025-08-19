#include "Post.hpp"
#include "Request.hpp"

Post::Post(Request &_obj) : obj(_obj),
							UnprocessedBuffer(_obj.GetBodyBuffer()),
							Boundary(obj.GetBoundarySettings()),
							FirstTime(true),
							BodyFullyRead(false)
{
	Boundary.CrlfCount = 0;
	BoundaryStatus = None;

	Chunk.ChunkStatus = ChunkVars::None;
	Chunk.BodySize = 0;
	Chunk.Temp = "";
	PrevBuffer = "";

	Dir = obj.GetUploadPath();
	srand(time(NULL));
}

Post::~Post()
{
	OutFile.close();
}

void Post::FindFileName(std::string &Buffer, std::string &Filename)
{
	struct	stat	Tmp;
	size_t	FilenamePos = 0, FilenameEndPos = 0;

	FilenamePos = Buffer.find("filename=\"", 0);
	FilenameEndPos = Buffer.find("\"\r\n", FilenamePos + 10);

	if (FilenamePos == std::string::npos || FilenameEndPos == std::string::npos)
	{
		obj.PrintError("Could't find file", obj), throw 400;
	}
	Filename = Buffer.substr(FilenamePos + 10, FilenameEndPos - (FilenamePos + 10)); // 10 = sizeof("filename=")
	if (Filename.empty())
		obj.PrintError("Bad Request", obj), throw 404;

	if (stat(Dir.c_str(), &Tmp) != 0) // Check for directory existance 
		obj.PrintError("Not Found", obj), throw 404;
	Filename = Dir + "/" + Filename; // "/Users/zderfouf/goinfre/ServerUploads" + "/" + "file.txt"

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

	// std::cout << "\n\nBuffer{" << Buffer << "}\n\n" << std::endl;
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

			Boundary.CrlfCount += CrlfCounter(Buffer, start + Boundary.BoundaryStart.size());
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
				Boundary.CrlfCount += CrlfCounter(Buffer, BodyPos + 4);
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

				if (++Boundary.CrlfCount != 5)
					obj.PrintError("Bad Request", obj), throw 400;
				BodyContent = Buffer.substr(0, end);
				OutFile.write(BodyContent.data(), BodyContent.size());
				PrevBuffer.clear();
				Buffer = Buffer.substr(end);
			}
		}
		if (BoundaryStatus == GotBodyEnd)
		{
			Boundary.CrlfCount = -1; // -1 for the {Boundary*CRLF*}

			size_t pos;
			pos = Buffer.find(Boundary.BoundaryStart);
			if (pos != std::string::npos)
				BoundaryStatus = GotBoundaryEnd;
			pos = Buffer.find(Boundary.BoundaryEnd);
			if (pos != std::string::npos)
				BoundaryStatus = Finished;
			if (BoundaryStatus == GotBodyEnd) break ;
		}
		if (BoundaryStatus == GotBoundaryEnd)
			OutFile.close(), FirstTime = true, BoundaryStatus = None;
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
	{
		FirstTime = false;
		if (obj.GetIsCGI())
		{
			cgiFileName = "/tmp/" + RandomString() + obj.GetFileExtention();
			obj.setCgiFileName(cgiFileName);
			OutFile.open(cgiFileName, std::ios::binary);
		}
		else
			OutFile.open(Dir + "/" + RandomString() + obj.GetFileExtention(), std::ios::binary);
	}

	OutFile.write(UnprocessedBuffer.c_str(), UnprocessedBuffer.size());

	if (obj.GetTotatlBytesRead() >= obj.GetContentLength())
	{
		OutFile.close(), FirstTime = true;
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
		FirstTime = false;
		if (obj.GetIsCGI())
		{
			cgiFileName = "/tmp/" + RandomString() + obj.GetFileExtention();
			obj.setCgiFileName(cgiFileName);
			OutFile.open(cgiFileName, std::ios::binary);
		}
		else
			OutFile.open(Dir + "/" + RandomString() + obj.GetFileExtention(), std::ios::binary);
		if (!OutFile.is_open())
			obj.PrintError("Could't open file", obj), throw 500; // Internal Server Error
	}

	OutFile.write(BodyContent.c_str(), BodyContent.size());
}


void Post::ParseChunked()
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
				this->OutFile.close(), FirstTime = true;
				obj.SetClientStatus(EndReading);
				std::cout << "File Uploaded!" << std::endl, throw 201;
			}
		}
	}
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
	|#				HandleCGI		    #|
	|#----------------------------------#|
*/

void	Post::HandleCGI()
{
	if (FirstTime)
	{
		cgiFileName = "/tmp/" + RandomString();
		obj.setCgiFileName(cgiFileName);
		OutFile.open(cgiFileName, std::ios::binary), FirstTime = false;
		if (!OutFile.is_open())
			obj.PrintError("Could't open file", obj), throw 500; // Internal Server Error
	}
	// if (obj.GetTotatlBytesRead() >= obj.GetContentLength())
	// {
	// 	obj.SetClientStatus(EndReading);
	// 	OutFile.close();
	// 	std::cout << "File Uploaded!" << std::endl, throw 201;
	// }
}

/*	|#----------------------------------#|
	|#			ParsingMenu		    	#|
	|#----------------------------------#|
*/

void	Post::HandlePost()
{
	UnprocessedBuffer = obj.GetBodyBuffer();

	std::cout << "Total bytes read: " << obj.GetTotatlBytesRead() << "/" << obj.GetContentLength() << std::endl;

	// if (obj.GetIsCGI())
	// 	HandleCGI();


	// std::cout << "DataType----->";
	// switch (obj.GetDataType())
	// {
	// 	case FixedLength:  { std::cout << "FixedLength!\n"; break ; }
	// 	case Chunked:  { std::cout << "Chunked!\n"; break ; }
	// }
	// std::cout << "ContentType----->";
	// switch (obj.GetContentType())
	// {
	// 	case _Boundary:  { std::cout << "_Boundary!\n"; break ; }
	// 	case BinaryOrRaw:  { std::cout << "BinaryOrRaw!\n"; break ; }
	// }

	switch (obj.GetDataType()) // Chunked || FixedLength
	{
		case FixedLength:
		{
			if (obj.GetTotatlBytesRead() > obj.GetContentLength())
				obj.PrintError("Malformed Request", obj), throw 400;
			switch (obj.GetContentType())
			{
				case	_Boundary	:	return ParseBoundary();
				case	BinaryOrRaw	:	return ParseBirnaryOrRaw();
			}
		}
		case Chunked:
		{
			if (obj.GetLimitedBodySize() && obj.GetTotatlBytesRead() > obj.GetMaxAllowedBodySize())
				obj.PrintError("Request Entity Too Large", obj), throw 413;
			switch (obj.GetContentType())
			{
				case	_Boundary	:	return ParseChunkedBoundary();
				case	BinaryOrRaw	:	return ParseChunked();
			}
		}
	}
}