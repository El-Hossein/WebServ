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
#include <dirent.h>
#include "../Request/Request.hpp"

class Response;

enum CgiStatus
{
    CGI_ERROR,
    CGI_COMPLETED,
    CGI_RUNNING
};

enum Connection
{
    _close,
    keepAlive,
    _Empty
};

class Cgi
{
    private :
        std::string scriptPath;
        std::string scriptOutput;
        std::string inpFile;
        std::string outFile;
        int         status;
        std::string scriptFile;
        std::string pathInfo;
        char **envp;
        pid_t pid;
        int exCode;
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
        int checkConnection;
        std::string memoExt;
        bool hasPendingCgi;


    public :
        Cgi();
        ~Cgi();
        
        bool        formatHttpResponse(std::string cgiFilePath, Request &req);
        void        parseOutput();
        int   executeCgiScript(Request &req, std::vector<ConfigNode> ConfigPars);
        std::string getScriptPath();
        void        setScriptPath(std::string _scriptPath);
        std::string getScriptOutput();
        void        setScriptOutput(std::string _scriptOutput);
        int         getStatus();
        std::string getScriptFile();
        void        setScriptFile(std::string _scriptfile);
        std::string getPathInfo();
        void        setPathInfo(std::string _pathinfo);
        void        splitPathInfo(Request &req);
        void        handleCgiRequest(Request &req, std::vector<ConfigNode> ConfigPars);
        bool        getUsingCgi();
        std::ifstream& getFile();
        std::string getCgiChunk();
        ssize_t     getFileSize();
        size_t      getFilePos();
        void        setFilePos(size_t _filepos);
        std::string getCgiHeader();
        size_t      getCgiHeaderSent();
        void        setCgiHeaderSent(size_t aa);
        bool        responseErrorcgi(int statusCode, std::string message, std::vector<ConfigNode> ConfigPars, Request &req);
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
        bool                getCheckConnection();
        void                setCheckConnection(int conn);
        int                 checkLocationCgi(Request &req, std::string meth, std::string directive, std::vector<ConfigNode> ConfigPars);
        int                 IsCgiRequest(std::string uri, Request &req, std::vector<ConfigNode> ConfigPars);
        int                servListingDirenCgi(Request &req, std::vector<ConfigNode> ConfigPars, std::string uri);
        bool                generateAutoIndexOnCgi(Request &req);
        std::string         generateListingDirCgi(Request &req);
        bool                gethasPendingCgi();
        void                sethasPendingCgi(bool pendingcgi);
        std::vector<std::string> getInfoConfigMultipleCgi(std::vector<ConfigNode> ConfigPars, std::string what, std::string location, Request &req);
        std::string             getInfoConfigCgi(std::vector<ConfigNode> ConfigPars, std::string what, std::string location, Request &req);
        
        

};

std::string intToString(int n);
std::string readFileToStringCgi(const std::string& path);