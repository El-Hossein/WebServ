#include "HttpServer.hpp"
HttpServer::HttpServer() {}

HttpServer::HttpServer(const HttpServer & other) {*this = other;}

HttpServer::~HttpServer() {}

void HttpServer::setup_server(std::vector<ConfigNode> ConfigPars){
    // struct sockaddr_in server_addr;
    std::vector<std::string>* ConfPorts = ConfigPars[0].getValuesForKey(ConfigPars[0], "listen");
    std::vector<int> ports;
    int server_fd = ConfigPars.size();
    std::cout << "Server : " << server_fd << "...\n";
    if (ConfPorts != NULL)
        for (size_t i = 0; i < ConfPorts->size(); i++)
            ports.push_back(std::stoi(ConfPorts->at(i)));
    

}

void HttpServer::run(){}