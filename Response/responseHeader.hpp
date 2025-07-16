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
    public :
        Response();
        Response(Request &req, int _clientFd);
        ~Response();
        
        std::string deleteResponse(std::vector<ConfigNode> ConfigPars);
        void        servListingDiren(std::vector<ConfigNode> ConfigPars);
        std::string generateAutoIndexOn();
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
};


std::string getInfoConfig(std::vector<ConfigNode> ConfigPars, std::string what, std::string location, int index);