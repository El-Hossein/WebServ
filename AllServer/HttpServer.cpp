#include "HttpServer.hpp"
HttpServer::HttpServer() {}

HttpServer::HttpServer(const HttpServer & other) {*this = other;}

HttpServer::~HttpServer() {}

void HttpServer::setup_server(std::vector<ConfigNode> ConfigPars){
    // struct sockaddr_in server_addr;
    std::vector<std::string>* helo = ConfigPars[0].getValuesForKey(ConfigPars[0], "listen");
    std::vector<int> ports;

    if (helo != NULL)
    {
        for (size_t i = 0; i < helo->size(); i++)
            std::cout << "Ports: " << helo->at(i) << std::endl;

        for (size_t i = 0; i < helo->size(); i++)
            ports.push_back(std::stoi(helo->at(i)));
    }

    //print

    
}

void HttpServer::run(){}