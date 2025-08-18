#pragma once

#define BACKLOG 128

// #include "../allincludes.hpp"
#include "../pars_config/config.hpp"
#include "../Request/Request.hpp"
#include "../Response/responseHeader.hpp"
extern int globalKq;

class Response;
class Request;

struct EventContext {
    int ident;         // fd for client
    pid_t cgi_pid;         //  pid for CGI
    Request* req;
    Response* res;
    bool is_cgi;
	bool marked_for_deletion;
	bool registered_read;
	bool registered_write;
	bool registered_timer;
	std::vector<pid_t> registered_procs; // pids for which EVFILT_PROC/EVFILT_TIMER were registered // <- new
    EventContext() : ident(-1), cgi_pid(0),req(NULL), res(NULL), 
                 is_cgi(false), marked_for_deletion(false),
                 registered_read(false), registered_write(false),
                 registered_timer(false) {}
};

class HttpServer{
	public:
		HttpServer();
		HttpServer(const HttpServer& other);
		~HttpServer();
		void	SetAllContexts(EventContext* ctx);
		void setup_server(std::vector<ConfigNode> ConfigPars);
		void run(std::vector<ConfigNode> ConfigPars);
		void  accept_new_client_fd(int server_fd, std::vector<ConfigNode> ConfigPars);
		void	traiteCgiProcess(std::vector<Response*> all_res, int kq, std::vector<Request*> all_request, std::vector<ConfigNode> ConfigPars, int i);
		void handle_client(int client_fd, struct kevent* event, std::vector<ConfigNode> ConfigPars, std::vector<Request * >& all, std::vector<Response *> & all_res);
		void remove_client(int client_fd);
		void RemoveClient(int client_fd);
		void RemoveReqRes(int client_fd);

		void handle_client_read(EventContext* ctx, Request * request, Response * response);
		void handle_client_write(EventContext* ctx, Request * request, Response * response, std::vector<ConfigNode> ConfigPars);
		void handle_cgi_exit(EventContext* ctx, Request *request, Response *response);
		void handle_cgi_timeout(EventContext* ctx, Request & request, Response & response);
		void AddToKqueue(struct kevent &event, int kq, intptr_t ident, int filter, int flags, void *udata, int fflags, intptr_t data);
		void handle_timeout(EventContext* ctx, Request & request);
		std::vector<EventContext*> all_contexts;
		void FreeContexts();
	private:
		int kq;
		std::map<int, EventContext*> proc_map; 
   		struct kevent events[BACKLOG];
		std::vector<EventContext*> pending_deletions;
		// int kq;
		std::vector<int> server_fds;
};