#pragma once

#define BACKLOG 128

#include "../allincludes.hpp"
#include "../pars_config/config.hpp"

#include "../Request/Request.hpp"
#include "../cgi/cgiHeader.hpp"
#include "../Response/responseHeader.hpp"

class HttpServer{
    public:
        HttpServer();
        HttpServer(const HttpServer& other);
        ~HttpServer();

        void setup_server(std::vector<ConfigNode> ConfigPars);
        void run(std::vector<ConfigNode> ConfigPars);
        void accept_new_client(int server_fd);
        void handle_client(int client_fd, int filter, std::vector<ConfigNode> ConfigPars);
        void remove_client(int client_fd);
    private:
        int kq;
        std::vector<int> server_fds;
        std::map<int, std::string> response_map;
};