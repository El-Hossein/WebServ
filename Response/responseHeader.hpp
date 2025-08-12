#pragma once

#include "../cgi/cgiHeader.hpp"
class Response
{
    private:
        std::string uri;
        std::string finalResponse;
        std::string method;
        std::string pathRequested;
        std::string autoIndexOn;
        std::string index;
        std::string htmlFound;
        int         clientFd;
        std::string headers;
        size_t      headerSent;

        std::ifstream   file;
        size_t      filePos;
        size_t      fileSize;
        std::string staticFileBody;
        size_t staticFilePos;
        bool usingStaticFile;
        ssize_t  bytesSent;
        ssize_t  bytesWritten;
        std::string chunk;
        bool        hasMore;


    public :
        Cgi _cgi;
        Response();
        Response(Request &req, int _clientFd);
        ~Response();
        
        void                deleteResponse(std::vector<ConfigNode> ConfigPars, Request &req);
        void                servListingDiren(std::vector<ConfigNode> ConfigPars, Request	&req);
        bool                generateAutoIndexOn(Request	&req);
        void                getResponse(Request	&req, std::vector<ConfigNode> ConfigPars);
        std::string         generateListingDir();
        std::string         deleteResponseSuccess(const std::string& message);
        std::string         getMethod();
        std::string         getUri();
        std::string         getFinalResponse();
        void                setFinalResponse(std::string _finalResponse);
        std::string         checkContentType(int index);
        void                moveToResponse(int &client_fd, Request	&req, std::vector<ConfigNode> ConfigPars);
        int                 getClientFd();
        void                setClientFd(int _clientFd);
        bool                getNextChunk(size_t chunkSize);
        int                 prepareFileResponse(std::string filepath, std::string contentType, Request &req);
        size_t              getHeaderSent();
        void                setHeaderSent(size_t _aa);
        void                responseError(int statusCode, const std::string& message, std::vector<ConfigNode> ConfigPars, Request &req);
        ssize_t             getBytesSent();
        void                setBytesSent(ssize_t _bytessent);
        ssize_t             getBytesWritten();
        void                setBytesWritten(ssize_t _byteswritten);
        bool                getHasMore();
        void                setHasMore(bool _hasmore);
        std::string         getChunk();
        bool                checkPendingCgi(std::vector<ConfigNode> ConfigPars, Request &req);
        std::string         postResponseSuccess(const std::string& message);
        int                 checkLocation(Request &req, std::string meth, std::string directive, std::vector<ConfigNode> ConfigPars);
        int                 prepareRedirectResponse(std::vector<std::string> redirect, Request &req, std::vector<ConfigNode> ConfigPars);
        void                nonRedirect(std::string redirectUrl, Request &req, std::vector<ConfigNode> ConfigPars, int statusCode);
};


std::string getInfoConfig(std::vector<ConfigNode> ConfigPars, std::string what, std::string location, Request &req);
std::vector<std::string> getInfoConfigMultiple(std::vector<ConfigNode> ConfigPars, std::string what, std::string location, Request &req);