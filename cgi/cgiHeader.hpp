#pragma once

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
        pid_t               pid;
        std::ifstream       file;
        std::string         uniq;
        pid_t               pid_1;
        int                 status;
        char                **envp;
        std::string         inpFile;
        std::string         outFile;
        std::string         cgiBody;
        std::string         memoExt;
        std::string         pathInfo;
        bool                usingCgi;
        std::string         cgiChunk;
        std::string         cgiHeader;
        int                 cgistatus;
        time_t              startTime;
        std::string         scriptPath;
        size_t              cgiFilePos;
        std::string         scriptFile;
        ssize_t             cgiFileSize;
        int                 cgiStatusCode;
        size_t              cgiHeaderSent;
        bool                hasPendingCgi;
        size_t              statCgiFilePos;
        std::string         statCgiFileBody;
        int                 checkConnection;
        bool                usingCgiStatFile;
        long                cgiContentLength;


    public :
        Cgi();
        ~Cgi();
        

        std::ifstream&              getFile();
        time_t                      gettime();
        long                        getCgiCL();
        void                        setCgiCL(long _cgiCL);
        pid_t                       getpid_1();
        std::string                 getinfile();
        std::string                 getoutfile();
        size_t                      getFilePos();
        ssize_t                     getFileSize();
        void                        parseOutput();
        bool                        getUsingCgi();
        std::string                 getCgiHeader();
        int                         getcgistatus();
        size_t                      getCgiHeaderSent();
        bool                        gethasPendingCgi();
        size_t                      getStatCgiFilePos();
        std::string                 getStatCgiFileBody();
        bool                        getCheckConnection();
        bool                        getUsingStatCgiFile();
        void                        splitPathInfo(Request &req);
        void                        setFilePos(size_t _filepos);
        void                        setcgistatus(int _cgistatus);
        void                        setCheckConnection(int conn);
        std::string                 frontPageCgi(std::string _uri);
        void                        sethasPendingCgi(bool pendingcgi);
        std::string                 generateListingDirCgi(Request &req);
        void                        setcgiHeader(std::string _cgiheader);
        bool                        prepareFileResponseCgi(Request &req);
        void                        setCgiHeaderSent(size_t _cgiHeaderSent);
        void                        setStatCgiFilePos(size_t _statCgifilepos);
        void                        setUsingStatCgiFile(bool _usingcgistatfile);
        int                         executeCgiScript(Request &req, std::vector<ConfigNode> ConfigPars);
        void                        handleCgiRequest(Request &req, std::vector<ConfigNode> ConfigPars);
        bool                        generateAutoIndexOnCgi(std::vector<ConfigNode> ConfigPars, Request &req);
        int                         IsCgiRequest(std::string uri, Request &req, std::vector<ConfigNode> ConfigPars);
        int                         servListingDirenCgi(Request &req, std::vector<ConfigNode> ConfigPars, std::string uri);
        bool                        responseErrorcgi(int statusCode, std::string message, std::vector<ConfigNode> ConfigPars, Request &req);
        std::string                 getInfoConfigCgi(std::vector<ConfigNode> ConfigPars, std::string what, std::string location, Request &req);
        int                         checkLocationCgi(Request &req, std::string meth, std::string directive, std::vector<ConfigNode> ConfigPars);
        std::vector<std::string>    getInfoConfigMultipleCgi(std::vector<ConfigNode> ConfigPars, std::string what, std::string location, Request &req);
        
};

std::string                         intToString(int n);
std::string                         readFileToStringCgi(std::string path);