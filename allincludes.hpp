#pragma once

#include <cstdio> 
#include <cstddef>
#include <exception>
#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <poll.h>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/event.h>
#include <sys/stat.h>
#include <csignal>

struct	_BoundarySettings
{
	std::string	Boundary;
	std::string	BoundaryStart;
	std::string	BoundaryEnd;
};

enum	_SubBodyStatus
{
	WithBoundaryStart,
	WithBothBoundaries,
	WithNoBoundary
};

struct	BoundaryFlager
{
	bool	BoolStart = false, BoolEnd = false, BoolFile = false;
	size_t	CrlfCount = 0;
};
