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

struct CgiResponse
{
    std::string headers;
    std::string body;
    int status_code;
};

std::string formatHttpResponse(const CgiResponse& cgiResponse);
std::string responseError(int status_code, const std::string& message);
CgiResponse parseOutput(const std::string& raw_output);
std::string executeCgiScript(const char* script_path, const char* method, const char* uri, const char* query_string);
std::string intToString(int n);
