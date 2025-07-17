#pragma once

#include "../AllServer/HttpServer.hpp"
#include <dirent.h>


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
        std::string autoIndexBody;
        size_t autoIndexPos;
        bool usingAutoIndex;
        size_t errorPos;
        bool usingError;
        std::string errorBody;
        ssize_t  bytesSent;
        ssize_t  bytesWritten;
        std::string chunk;
        bool hasMore;

    public :
        Response();
        Response(Request &req, int _clientFd);
        ~Response();
        
        std::string deleteResponse(std::vector<ConfigNode> ConfigPars);
        void        servListingDiren(std::vector<ConfigNode> ConfigPars);
        bool        generateAutoIndexOn();
        std::string getResponse(Request	&req, std::vector<ConfigNode> ConfigPars);
        std::string generateListingDir();
        std::string deleteResponseSuccess(const std::string& message);
        std::string getMethod();
        std::string getUri();
        std::string getFinalResponse();
        void        setFinalResponse(std::string _finalResponse);
        std::string        checkContentType();
        void        moveToResponse(int &client_fd, Request	&req, std::vector<ConfigNode> ConfigPars);
        int        getClientFd();
        void       setClientFd(int _clientFd);
        bool    getNextChunk(size_t chunkSize);
        bool    prepareFileResponse(const std::string& filepath, const std::string& contentType, Request &req);
        size_t  getHeaderSent();
        void    setHeaderSent(size_t _aa);
        bool responseError(int statusCode, const std::string& message, std::vector<ConfigNode> ConfigPars);
        ssize_t getBytesSent();
        void    setBytesSent(ssize_t _bytessent);
        ssize_t getBytesWritten();
        void    setBytesWritten(ssize_t _byteswritten);
        bool    getHasMore();
        void    setHasMore(bool _hasmore);
        std::string getChunk();
};


std::string getInfoConfig(std::vector<ConfigNode> ConfigPars, std::string what, std::string location, int index);