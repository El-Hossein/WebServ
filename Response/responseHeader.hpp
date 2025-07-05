#pragma once

#include "../AllServer/HttpServer.hpp"
#include <dirent.h>

void    moveToResponse(int &client_fd, std::map<int, std::string>& response_map, Request	&req, std::vector<ConfigNode> ConfigPars);
std::string getInfoConfig(std::vector<ConfigNode> ConfigPars, std::string what);