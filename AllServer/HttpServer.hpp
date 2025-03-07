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
        void accept_new_client(int server_fd);
        void handle_client(int client_fd, int filter);
        void remove_client(int client_fd);
    private:
        int kq;
        std::vector<int> server_fds;
        std::map<int, std::string> response_map;
};

#endif