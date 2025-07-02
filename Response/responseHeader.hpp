#pragma once

#include "../AllServer/HttpServer.hpp"


void    moveToResponse(int &client_fd, std::map<int, std::string>& response_map, Request	&req);