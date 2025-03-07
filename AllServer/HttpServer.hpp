#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include "../allincludes.hpp"
#include "../pars_config/config.hpp"

class HttpServer {
    public:
        HttpServer();
        HttpServer(const HttpServer& other);
        ~HttpServer();

        void setup_server(std::vector<ConfigNode> ConfigPars);
        void run();
        void accept_new_client();
        void handle_client(int index);
    private:
        int kq;
        std::vector<int> server_fds;
        
};

#endif