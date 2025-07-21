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

class Response;

enum CgiStatus {
            CGI_ERROR = 0,
            CGI_COMPLETED = 1,
            CGI_RUNNING = 2
};

class Cgi
{
    private :
        char *postRequestBody; //testing
        std::string scriptPath;
        std::string scriptOutput;
        std::string inpFile;
        std::string outFile;
        int         status;
        std::string scriptFile;
        std::string pathInfo;
        char **envp;
        pid_t pid;
        int exitCode;
        int cgiStatusCode;
        std::string cgiHeader;
        std::string cgiBody;
        size_t cgiHeaderSent;
        ssize_t cgiFileSize;
        size_t cgiFilePos;
        std::ifstream file;
        bool usingCgi;
        std::string cgiChunk;
        std::string statCgiFileBody;
        size_t statCgiFilePos;
        bool usingCgiStatFile;
        pid_t pid_1;
        int cgistatus;
        time_t startTime;
        std::string uniq;


    public :
        Cgi();
        ~Cgi();
        
        bool        formatHttpResponse(std::string cgiFilePath);
        void        parseOutput();
        int   executeCgiScript(const Request &req, std::vector<ConfigNode> ConfigPars);
        std::string getScriptPath();
        void        setScriptPath(std::string _scriptPath);
        std::string getScriptOutput();
        void        setScriptOutput(std::string _scriptOutput);
        int         getStatus();
        std::string getScriptFile();
        void        setScriptFile(std::string _scriptfile);
        std::string getPathInfo();
        void        setPathInfo(std::string _pathinfo);
        void        splitPathInfo(const Request &req);
        void        handleCgiRequest(const Request &req, std::vector<ConfigNode> ConfigPars);
        bool        getUsingCgi();
        std::ifstream& getFile();
        std::string getCgiChunk();
        ssize_t     getFileSize();
        size_t      getFilePos();
        void        setFilePos(size_t _filepos);
        std::string getCgiHeader();
        size_t      getCgiHeaderSent();
        void        setCgiHeaderSent(size_t aa);
        bool        responseErrorcgi(int statusCode, std::string message, std::vector<ConfigNode> ConfigPars);
        size_t      getStatCgiFilePos();
        void        setStatCgiFilePos(size_t _statCgifilepos);
        bool        getUsingStatCgiFile();
        void        setUsingStatCgiFile(bool _usingcgistatfile);
        std::string getStatCgiFileBody();
        void        setcgiHeader(std::string _cgiheader);
        int         getcgistatus();
        void         setcgistatus(int _cgistatus);
        pid_t         getpid_1();
        time_t         gettime();
        std::string    getoutfile();
        std::string    getinfile();
        
        

};

std::string intToString(int n);
int         IsCgiRequest(const char *uri);
