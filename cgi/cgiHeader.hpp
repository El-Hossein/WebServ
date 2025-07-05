#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sstream>
#include <fstream>
#include "../Request/Request.hpp"
#include "../Response/responseHeader.hpp"

struct CgiResponse
{
    std::string headers;
    std::string body;
    int status_code;
};

std::string formatHttpResponse(const CgiResponse& cgiResponse);
std::string responseError(int status_code, const std::string& message, std::vector<ConfigNode> ConfigPars);
CgiResponse parseOutput(const std::string& raw_output);
std::string executeCgiScript(const char* script_path, const Request &req);
std::string intToString(int n);
int IsCgiRequest(const char *uri);
std::string handleCgiRequest(const Request &req, std::vector<ConfigNode> ConfigPars);
