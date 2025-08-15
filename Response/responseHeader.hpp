#pragma once

#include "../cgi/cgiHeader.hpp"

class Response
{
    private:
        std::string                    uri;
        std::string                    index;
        std::string                    chunk;
        ssize_t                        bytesSent;
        std::string                    method;
        std::string                    headers;
        size_t                         filePos;
        bool                           hasMore;
        int                            clientFd;
        std::ifstream                  file;
        size_t                         fileSize;
        size_t                         staticFilePos;
        bool                           usingStaticFile;
        std::string                    htmlFound;
        ssize_t                        bytesWritten;
        size_t                         headerSent;
        std::string                    autoIndexOn;
        std::string                    finalResponse;
        std::string                    pathRequested;
        std::string                    staticFileBody;

    public :
        Cgi                            _cgi;
        Response();
        Response(Request &req, int _clientFd);
        ~Response();
        
        std::string                    getChunk();
        bool                           getHasMore();
        int                            getClientFd();
        ssize_t                        getBytesSent();
        ssize_t                        getBytesWritten();
        void                           setHasMore(bool _hasmore);
        std::string                    checkContentType(int index);
        bool                           getNextChunk(size_t chunkSize);
        void                           setBytesSent(ssize_t _bytessent);
        void                           setHeaderSent(size_t _headerSent);
        std::string                    generateListingDir(Request	&req);
        void                           postResponse(Request &req, int e);
        void                           setBytesWritten(ssize_t _byteswritten);
        std::string                    postResponseSuccess(std::string message);
        std::string                    deleteResponseSuccess(std::string message);
        void                           getResponse(Request	&req, std::vector<ConfigNode> ConfigPars);
        void                           deleteMethod(Request &req, std::vector<ConfigNode> ConfigPars);
        void                           deleteResponse(std::vector<ConfigNode> ConfigPars, Request &req);
        bool                           checkPendingCgi(std::vector<ConfigNode> ConfigPars, Request &req);
        void                           postMethod(Request &req, std::vector<ConfigNode> ConfigPars, int e);
        void                           servListingDiren(std::vector<ConfigNode> ConfigPars, Request	&req);
        bool                           generateAutoIndexOn(std::vector<ConfigNode> ConfigPars, Request	&req);
        int                            prepareFileResponse(std::string filepath, std::string contentType, Request &req);
        void                           moveToResponse(int &client_fd, Request	&req, std::vector<ConfigNode> ConfigPars, int e);
        void                           responseError(int statusCode, std::string message, std::vector<ConfigNode> ConfigPars, Request &req);
        void                           nonRedirect(std::string redirectUrl, Request &req, std::vector<ConfigNode> ConfigPars, int statusCode);
        int                            checkLocation(Request &req, std::string meth, std::string directive, std::vector<ConfigNode> ConfigPars);
        void                           prepareRedirectResponse(std::vector<std::string> redirect, Request &req, std::vector<ConfigNode> ConfigPars);
};


std::string                            frontPage(std::string uri);
std::string                            getInfoConfig(std::vector<ConfigNode> ConfigPars, std::string what, std::string location, Request &req);
std::vector<std::string>               getInfoConfigMultiple(std::vector<ConfigNode> ConfigPars, std::string what, std::string location, Request &req);