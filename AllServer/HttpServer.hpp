#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include "../allincludes.hpp"

class HttpServer {
    public:
        HttpServer();
        HttpServer(const HttpServer& other);
        ~HttpServer();

        void setup_server();
        void run();
        void accept_new_client();
        void handle_client(int index);
    private:
        std::vector<struct pollfd> poll_fds;
        int server_fd1;
        int server_fd2;
};

#endif