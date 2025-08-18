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
        int                 kq;
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
        

        void                        setKq(int _kq);
        std::ifstream&              getFile();
        time_t                      gettime();
        long                        getCgiCL();
        pid_t                       getpid_1();
        std::string                 getinfile();
        std::string                 getoutfile();
        long                        getFilePos();
        long                        getFileSize();
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
        void                        setCgiCL(long _cgiCL);
        void                        setFilePos(size_t _filepos);
        void                        splitPathInfo(Request &req);
        void                        setcgistatus(int _cgistatus);
        void                        setCheckConnection(int conn);
        std::string                 frontPageCgi(std::string _uri);
        void                        executeCgiScript(Request &req);
        void                        handleCgiRequest(Request &req);
        void                        sethasPendingCgi(bool pendingcgi);
        std::string                 generateListingDirCgi(Request &req);
        bool                        generateAutoIndexOnCgi(Request &req);
        void                        setcgiHeader(std::string _cgiheader);
        bool                        prepareFileResponseCgi(Request &req);
        void                        setCgiHeaderSent(size_t _cgiHeaderSent);
        void                        setStatCgiFilePos(size_t _statCgifilepos);
        void                        setUsingStatCgiFile(bool _usingcgistatfile);
        int                         IsCgiRequest(std::string uri, Request &req);
        int                         servListingDirenCgi(Request &req, std::string uri);
        void                        responseErrorcgi(int statusCode, std::string message, Request &req);
        std::string                 getInfoConfigCgi(std::string what, std::string location, Request &req);
        int                         checkLocationCgi(Request &req, std::string meth, std::string directive);
        std::vector<std::string>    getInfoConfigMultipleCgi(std::string what, std::string location, Request &req);
        
};

std::string                         intToString(int n);
std::string                         readFileToStringCgi(std::string path);