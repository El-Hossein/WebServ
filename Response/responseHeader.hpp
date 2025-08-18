#pragma once

#include "../cgi/cgiHeader.hpp"

class Response
{
    private:
        int                            _E;
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
        std::string                    errorP;

    public :
        Cgi                            _cgi;
        Response();
        Response(Request &req, int _clientFd);
        ~Response();
        
        bool    sendBody(size_t chunkSize);
        bool    sendFile(size_t chunkSize);
        bool    sendHeaders(size_t chunkSize);
        bool    sendCgiScript(size_t chunkSize);
        bool    readAccordingToCL(size_t chunkSize, std::ifstream &f);

        void    setE(int _e);
        int     getE();

        std::string                    getChunk();
        bool                           getHasMore();
        int                            getClientFd();
        size_t                        getBytesSent();
        long                        getBytesWritten();
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
        void                           getResponse(Request	&req);
        void                           deleteMethod(Request &req);
        void                           deleteResponse(Request &req);
        bool                           checkPendingCgi(Request &req);
        void                           postMethod(Request &req, int e);
        void                           servListingDiren(Request	&req);
        bool                           generateAutoIndexOn(Request	&req);
        int                            prepareFileResponse(std::string filepath, std::string contentType, Request &req);
        void                           moveToResponse(Request	&req, int e);
        void                           responseError(int statusCode, std::string message, Request &req);
        void                           nonRedirect(std::string redirectUrl, Request &req, int statusCode);
        int                            checkLocation(Request &req, std::string meth, std::string directive);
        void                           prepareRedirectResponse(std::vector<std::string> redirect, Request &req);
};


std::string                            frontPage(std::string uri);
std::string                            getInfoConfig(std::string what, std::string location, Request &req);
std::vector<std::string>               getInfoConfigMultiple(std::string what, std::string location, Request &req);