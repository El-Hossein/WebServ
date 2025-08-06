#pragma once

#define BACKLOG 128

#include "../allincludes.hpp"
#include "../pars_config/config.hpp"

#include "../Request/Request.hpp"
#include "../Response/responseHeader.hpp"

extern int globalKq;

class Response;

class HttpServer{
	public:
		HttpServer();
		HttpServer(const HttpServer& other);
		~HttpServer();

		void setup_server(std::vector<ConfigNode> ConfigPars);
		void run(std::vector<ConfigNode> ConfigPars);
		Request * accept_new_client(int server_fd, std::vector<ConfigNode> ConfigPars);
		void	traiteCgiProcess(std::vector<Response*> all_res, int kq, std::vector<Request*> all_request, std::vector<ConfigNode> ConfigPars, int i);
		void handle_client(int client_fd, struct kevent* event, std::vector<ConfigNode> ConfigPars, std::vector<Request * >& all, std::vector<Response *> & all_res);
		void remove_client(int client_fd);
	private:
   		struct kevent events[BACKLOG];
		int kq;
		std::vector<int> server_fds;
		std::map<int, std::string> response_map;
};