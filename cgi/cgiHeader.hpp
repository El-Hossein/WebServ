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

struct pathInfo
{
    std::string scriptFile;
    std::string _pathInfo;
};

class Cgi
{
    private :
        char *postRequestBody; //testing
        std::string scriptPath;
        std::string scriptOutput;
        std::string inpFile;
        std::string outFile;
        std::string output;
        int         status;


    public :
        Cgi();
        ~Cgi();
        std::string formatHttpResponse(const CgiResponse& cgiResponse);
        CgiResponse parseOutput(const std::string& raw_output);
        std::string executeCgiScript(const Request &req, std::vector<ConfigNode> ConfigPars, std::string _pathInfo);
        std::string getScriptPath();
        void        setScriptPath(std::string _scriptPath);
        std::string getScriptOutput();
        void        setScriptOutput(std::string _scriptOutput);
        int         getStatus();


};

std::string responseErrorcgi(int status_code, const std::string& message, std::vector<ConfigNode> ConfigPars);
std::string intToString(int n);
std::string handleCgiRequest(const Request &req, std::vector<ConfigNode> ConfigPars);
int         IsCgiRequest(const char *uri);
