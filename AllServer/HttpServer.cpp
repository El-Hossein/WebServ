#include "HttpServer.hpp"
#include <stdexcept>


void	HttpServer::SetAllContexts(EventContext* ctx) { all_contexts.push_back(ctx);	}

HttpServer::HttpServer()	{ }

HttpServer::HttpServer(const HttpServer & other)	{ *this = other; }

HttpServer::~HttpServer()	{ }

std::set<int> GetAllPorts(std::vector<ConfigNode> &ConfigPars)
{
    std::set<int> allPorts;

    for (size_t i = 0; i < ConfigPars.size(); i++)
    {
        std::vector<std::string> ConfPort = ConfigPars[i].getValuesForKey(ConfigPars[i], "listen", "");

        for (size_t j = 0; j < ConfPort.size(); j++)
            allPorts.insert(atoi(ConfPort[j].c_str()));
    }

    return allPorts;
}

// Set up the server address for binding
void SetUpForBind(struct sockaddr_in &server_addr, int port)
{
	// Bind the socket to an address
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(port);
}

// Bind the socket to an address and listen on the socket
int BindAndListen(int server_fd, struct sockaddr_in server_addr)
{
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		return (1);
	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		return  (1);

	// Listen on the socket
	if (listen(server_fd, BACKLOG) < 0)
		return  (1);
	return  0;
}
// Add the socket to the kqueue
void HttpServer::AddToKqueue(struct kevent &event, int kq, intptr_t ident, int filter, int flags, void *udata, int fflags, intptr_t data)
{
	EV_SET(&event, ident, filter, flags, fflags, data, udata);
	kevent(kq, &event, 1, NULL, 0, NULL);
}

//done with the server setup
void HttpServer::setup_server(std::vector<ConfigNode> ConfigPars)
{
    // std::cout << "\033[34mSetting up server...\033[0m" << std::endl;
    kq = kqueue();
    if (kq == -1)
        throw ("\033[31mFailed to create kqueue\033[0m");

    std::set<int> AllPorts = GetAllPorts(ConfigPars);

    size_t i = 0;
    for (std::set<int>::iterator it = AllPorts.begin(); it != AllPorts.end(); ++it, ++i)
    {
        int port = *it;

        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1)
        {
            // std::cout << "\033[31m[-]\033[0m \033[31mSocket creation failed for port " << port << "\033[0m\n";
            continue;
        }

        fcntl(server_fd, F_SETFD, FD_CLOEXEC);

        fcntl(server_fd, F_SETFL, O_NONBLOCK);

        // int opt = 1;
        // setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        struct sockaddr_in server_addr;
        SetUpForBind(server_addr, port);
        if (BindAndListen(server_fd, server_addr) == 1)
        {
            close(server_fd);
            continue;
        }

        struct kevent event;
        // AddToKqueue(event, kq, server_fd, EVFILT_READ, EV_ADD | EV_ENABLE, NULL, 0, 0);
        EV_SET(&event, server_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	    if(kevent(kq, &event, 1, NULL, 0, NULL) == -1)
            throw ("\033[31mkevent failed\033[0m");
        server_fds.push_back(server_fd);

        std::cout << "\033[32m[+]\033[0m \033[32mServer " << i << " listening on port " << port << "\033[0m" << std::endl;
    }

    std::cout << "------------------------------------------------------------------------------" << std::endl;
}


void HttpServer::accept_new_client_fd(int server_fd, std::vector<ConfigNode> ConfigPars)
{
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);

    if (client_fd < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            // std::cout << "\033[31m[-]\033[0m \033[31mAccept failed on server_fd " << server_fd << ": " << strerror(errno) << "\033[0m" << std::endl;
        }
        return ;
    }

    // Immediately set FD_CLOEXEC so future exec()'d children won't inherit this client socket
    fcntl(client_fd, F_SETFD, FD_CLOEXEC);

    // set non-blocking (preserve existing flags)
    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(client_addr.sin_port);

    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(server_addr);
    getsockname(client_fd, (struct sockaddr*)&server_addr, &server_addr_len);

    char server_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &server_addr.sin_addr, server_ip, INET_ADDRSTRLEN);
    int server_port = ntohs(server_addr.sin_port);

    // std::cout << "-----------------------------------------------------------------------------" << std::endl;
    std::cout << "\033[32m[+]\033[0m \033[32mClient " << client_ip << ":" << client_port << " connected to server at " << server_ip << ":" << server_port << "\033[0m\n" << std::endl;

    Request* req = new Request(client_fd, ReadHeader, ConfigPars, server_port);
    req->SetTimeOut(std::time(NULL));
    Response* res = new Response(client_fd, kq);

    // Create EventContext
	EventContext* ctx = new EventContext;
	ctx->ident = client_fd;
	ctx->cgi_pid = 0;
	ctx->req = req;
	ctx->res = res;
	ctx->is_cgi = false;

	SetAllContexts(ctx);
    // Register with kqueue
    struct kevent kev;
	AddToKqueue(kev, kq, client_fd, EVFILT_READ, EV_ADD | EV_ENABLE, ctx, 0, NULL);

	AddToKqueue(kev, kq, client_fd, EVFILT_WRITE, EV_ADD | EV_DISABLE, ctx, 0, NULL);

    EV_SET(&kev, client_fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, 30, ctx);
	kevent(kq, &kev, 1, NULL, 0, NULL);
    return ;
}

void	SetUpResponse(EventContext* ctx, Response * res, Request	*Request, int &e)
{
    if (e != 200 && e != 201 && e != -1 && e != 42)
       res->setE(e);
    else
       res->setE(0);
	switch (e)
	{
		case 500: res->responseError(500, " Internal Server Error", *Request); return;
		case 505: res->responseError(505, " HTTP Version Not Supported", *Request); return;
		case 501: res->responseError(501, " Not Implemented", *Request); return;
		case 400: res->responseError(400, " Bad Request", *Request); return;
		case 403: res->responseError(403, " Forbidden", *Request); return;
		case 404: res->responseError(404, " Not found", *Request); return;
		case 405: res->responseError(405, " Method Not Allowed", *Request); return;
        case 411: res->responseError(411, " Length Required", *Request); return;
		case 413: res->responseError(413, " Content Too Large", *Request); return;
		case 414: res->responseError(414, " URI Too Long", *Request); return;
		case 415: res->responseError(415, " Unsupported Media Type", *Request); return;
	}
	Request->SetContext(ctx);
	res->moveToResponse(*Request, e);
}


void HttpServer::RemoveClient(int client_fd)
{
    // std::cout << "\033[31m[-]\033[0m \033[31mClose Client FD : " << client_fd << "\033[0m" << std::endl;

    struct kevent kev;

    // Remove socket related registrations (safe to call even if not present)
    EV_SET(&kev, client_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    kevent(kq, &kev, 1, NULL, 0, NULL);
    EV_SET(&kev, client_fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    kevent(kq, &kev, 1, NULL, 0, NULL);
    EV_SET(&kev, client_fd, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
    kevent(kq, &kev, 1, NULL, 0, NULL);

    if (client_fd >= 0)
    {
        close(client_fd);
        // std::cout << "\033[32m[-]\033[0m RemoveClient : Closed client fd: " << client_fd << std::endl;
    }

    // Find the context for this fd in all_contexts
    for (std::vector<EventContext*>::iterator it = all_contexts.begin(); it != all_contexts.end(); ++it)
    {
        EventContext* ctx = *it;
        if (ctx->ident == client_fd)
        {
            // 1) Deregister any process/timer we recorded in ctx->registered_procs
            if (!ctx->registered_procs.empty())
            {
                // std::cout << "\033[32m[-]\033[0m RemoveClient: deregistering EVFILT_PROC/EVFILT_TIMER for pids:";
                for (size_t pi = 0; pi < ctx->registered_procs.size(); ++pi)
                {
                    pid_t p = ctx->registered_procs[pi];
                    if (p <= 0) continue;
                    // std::cout << " " << p;
                    EV_SET(&kev, p, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
                    kevent(kq, &kev, 1, NULL, 0, NULL);
                    EV_SET(&kev, p, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
                    kevent(kq, &kev, 1, NULL, 0, NULL);
                }
                // std::cout << std::endl;
                ctx->registered_procs.clear();
            }
            else
            {
                pid_t proc_pid = ctx->cgi_pid;
                if (proc_pid == 0 && ctx->res)
                {
                    pid_t rpid = ctx->res->_cgi.getpid_1();
                    if (rpid > 0) proc_pid = rpid;
                }
                if (proc_pid > 0)
                {
                    // std::cout << "RemoveClient: deregistering EVFILT_PROC/EVFILT_TIMER for pid "<< proc_pid << std::endl;
                    EV_SET(&kev, proc_pid, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
                    kevent(kq, &kev, 1, NULL, 0, NULL);
                    EV_SET(&kev, proc_pid, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
                    kevent(kq, &kev, 1, NULL, 0, NULL);
                }
            }

            // 2) Remove any entries in proc_map that point to this ctx
            if (!proc_map.empty())
            {
                std::vector<int> keys_to_erase;
                for (std::map<int, EventContext*>::iterator mit = proc_map.begin(); mit != proc_map.end(); ++mit)
                {
                    if (mit->second == ctx)
                        keys_to_erase.push_back(mit->first);
                }
                for (size_t kk = 0; kk < keys_to_erase.size(); ++kk)
                    proc_map.erase(keys_to_erase[kk]);
            }

            // 3) mark for deletion and push to pending list; do NOT delete here
            ctx->marked_for_deletion = true;
            pending_deletions.push_back(ctx);

            // remove from active contexts list
            all_contexts.erase(it);
            break;
        }
    }
}


void HttpServer::RemoveReqRes(int client_fd)
{
    // std::cout << "\033[31mReset Request/Response for FD : " << client_fd << "\033[0m" << std::endl;

    for (std::vector<EventContext*>::iterator it = all_contexts.begin(); it != all_contexts.end(); ++it)
    {
        EventContext* ctx = *it;
        if (ctx->ident == client_fd)
        {
            if (ctx->req)
            {
                delete ctx->req;
                ctx->req = NULL;
            }
            if (ctx->res)
            {
                delete ctx->res;
                ctx->res = NULL;
            }
            ctx->is_cgi = false;
            return;
        }
    }
}

void HttpServer::handle_client_write(EventContext* ctx, Request * request, Response * response, std::vector<ConfigNode> ConfigPars)
{
	request->SetTimeOut(std::time(NULL));

	if (response->getChunk().empty() || response->getBytesSent() >= response->getChunk().size())
	{
		response->setHasMore(response->getNextChunk(8192));
			response->setBytesSent(0);
	}
	if (!response->getChunk().empty())
	{
		response->setBytesWritten(send(ctx->ident, response->getChunk().c_str() + response->getBytesSent(), response->getChunk().size() - response->getBytesSent(), 0));
		if (response->getBytesWritten() < 0)
			return;
		else if (response->getBytesWritten() == 0)
		{
			RemoveClient(ctx->ident);
			return;
		}
		response->setBytesSent(response->getBytesSent() + response->getBytesWritten());
        // std::cout << "send: " << response->getBytesSent() <<  std::endl;
		if (response->getBytesSent() < response->getChunk().size())
			return;
	}
	// std::cout << "\033[32m[+]\033[0m End Writing" << std::endl;
	if (!response->getHasMore())
	{
		response->setHeaderSent(0);
		response->_cgi.setCgiHeaderSent(0);
		
        // std::cout << "EEE: " << response->getE() << std::endl;
		if (response->_cgi.getCheckConnection() == keepAlive && response->getE() == 0)
		{
			// std::cout << "\033[32m[+]\033[0m keep-alive" << std::endl;
			int fd = ctx->ident;
			int server_port = ctx->req ? ctx->req->GetServerDetails().RealPort : -1;

			RemoveReqRes(fd);

			Request* new_request = new Request(fd, ReadHeader, ConfigPars, server_port);
			Response* new_response = new Response(fd, kq);

			ctx->req = new_request;
			ctx->res = new_response;
			ctx->is_cgi = 0;
			ctx->cgi_pid = 0;
			new_request->SetTimeOut(std::time(NULL));

			struct kevent ev;
			
			AddToKqueue(ev, kq, fd, EVFILT_WRITE, EV_DISABLE, ctx, 0, 0);
			AddToKqueue(ev, kq, fd, EVFILT_READ, EV_ADD | EV_ENABLE, ctx, 0, 0);
		}
		else 
		{
			response->_cgi.setCheckConnection(_Empty);
			RemoveClient(ctx->ident);
		}
	}

}

void HttpServer::handle_client_read(EventContext* ctx, Request * request, Response * response)
{
		request->SetTimeOut(std::time(NULL));
		// std::cout << request->GetTimeOut() << std::endl;
		try { request->SetUpRequest(); }
		catch (int	&e)
		{
			if (request->GetClientStatus() != EndReading || e == -1)
				return;
			// std::cout << "\033[34m++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ REQUEST CLIENT\033[0m" << std::endl;
			// std::cout << "CLIENT: " <<  ctx->ident <<  std::endl;
			// std::map<std::string, std::string> all_header = request->GetHeaders();
			// for (std::map<std::string, std::string>::const_iterator it = all_header.begin(); it != all_header.end(); it++)
				// std::cout << it->first << ": " << it->second << std::endl;
			// std::cout << "\033[34m++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\033[0m" << std::endl;

			SetUpResponse(ctx, response, request, e);
			// std::cout << "\033[32m[+]\033[0m End building response" << std::endl;
			if (response->_cgi.gethasPendingCgi())
				return;
			else
			{
				// std::cout << "\033[32m[+]\033[0m change to write" << std::endl;
				struct kevent ev;
				AddToKqueue(ev, kq, ctx->ident, EVFILT_WRITE, EV_ADD | EV_ENABLE, ctx, 0, 0);
				AddToKqueue(ev, kq, ctx->ident, EVFILT_READ, EV_DISABLE, ctx, 0, 0);
			}
		}
}

void HttpServer::handle_cgi_exit(EventContext* ctx, Request * request, Response * response)
{
	pid_t exitedPid = ctx->cgi_pid;
	if (response->_cgi.gethasPendingCgi() && response->_cgi.getpid_1() == exitedPid)
	{
		if (response->checkPendingCgi(*request))
		{
			pid_t pid = response->_cgi.getpid_1();
			struct kevent kev;
			EV_SET(&kev, pid, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
			kevent(kq, &kev, 1, NULL, 0, NULL);

			struct kevent ev;
			int client_fd = response->getClientFd();
			AddToKqueue(ev, kq, client_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, ctx, 0, 0);
			AddToKqueue(ev, kq, client_fd, EVFILT_READ, EV_DISABLE, ctx, 0, 0);
			// AddToKqueue(ev, kq, client_fd, EVFILT_TIMER, EV_DISABLE, ctx, 0, 0);
			request->SetTimeOut(std::time(NULL));
			ctx->is_cgi = false;
		}
		response->_cgi.sethasPendingCgi(false);
	}
}

void HttpServer::handle_cgi_timeout(EventContext* ctx, Request & request, Response & response)
{
	if (ctx->cgi_pid != 0)
	{
        time_t currentTime = time(NULL);
        if(currentTime - request.GetTimeOut() >= 30)
        {
            pid_t pid = response._cgi.getpid_1();
            struct kevent kev;
            EV_SET(&kev, pid, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
            kevent(kq, &kev, 1, NULL, 0, NULL);
            kill(pid, SIGTERM);
            ctx->cgi_pid = 0;
            ctx->is_cgi = false;
            response._cgi.responseErrorcgi(504, " Gateway Timeout", request);
            response._cgi.setcgistatus(CGI_ERROR);
            if (!response._cgi.getinfile().empty())
                unlink(response._cgi.getinfile().c_str());
            if (!response._cgi.getoutfile().empty())   
                unlink(response._cgi.getoutfile().c_str());
            response._cgi.sethasPendingCgi(false);
            request.SetTimeOut(std::time(NULL));
            struct kevent ev;
            int client_fd = response.getClientFd();
            AddToKqueue(ev, kq, client_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, ctx, 0, 0);
            AddToKqueue(ev, kq, client_fd, EVFILT_READ, EV_DISABLE, ctx, 0, 0);
            return;
        }
	}
}

void HttpServer::handle_timeout(EventContext* ctx, Request & request)
{

	if (!ctx || !ctx->req || !ctx->res)
	{
		// std::cout << "handle_timeout: null ctx/req/res -> removing client if possible\n";
		if (ctx) RemoveClient(ctx->ident);
		return;
	}

	if (ctx->cgi_pid != 0 && ctx->is_cgi == true)
	{
		// std::cout << "\033[31m[-]\033[0m timeout but CGI - skipping client timeout handling" << std::endl;
		return;
	}

	// std::cout << "\033[32m[+]\033[0m TIMES IS UP FOR: " << ctx->ident << std::endl;
	time_t currentTime = time(NULL);
	// std::cout << "--cur: " << currentTime << "  my: " << request.GetTimeOut() << std::endl;
	if(currentTime - request.GetTimeOut() >= 30)
		RemoveClient(ctx->ident);
}

void HttpServer::run(std::vector<ConfigNode> ConfigPars)
{
	std::cout << "\033[34m--Start Loop Infinite\033[0m" << std::endl;
    while (true)
    {
        int nev = kevent(kq, NULL, 0, events, BACKLOG, NULL);
		std::cout << "\033[34m------------------------Requests Send------------------------\033[0m" << std::endl;
        if (nev < 0)
        {
            if (errno == EINTR) continue; // interrupted by signal, safe to retry
            std::cout << "\033[31mkevent error: " << strerror(errno) << "\033[0m" << std::endl;
            continue;
        }

        for (int i = 0; i < nev; ++i)
        {
            // Check if this event is from a listening server socket
            bool isServerSocket = false;
            for (size_t j = 0; j < server_fds.size(); ++j)
            {
                if (server_fds[j] == (int)events[i].ident)
                {
                    std::cout << "\033[34mserver FD : " << events[i].ident << "\033[0m" << std::endl;
                    accept_new_client_fd(static_cast<int>(events[i].ident), ConfigPars);
                    isServerSocket = true;
                    break;
                }
            }
            if (isServerSocket) continue;

            // Non-server events: safe to read fields once
            int filter = events[i].filter;
            int flags = events[i].flags;
            unsigned int fflags = events[i].fflags;

            EventContext* ctx = static_cast<EventContext*>(events[i].udata);
            std::cout << "client fd: "  << " | filter: " << filter << std::endl;

            if (ctx == NULL)
                continue;
			if (ctx->marked_for_deletion)
			{
				/* std::cout << "Skipping event for ctx marked for deletion: " << (void*)ctx << std::endl; */
				continue;
			}

            std::cout << "\033[32m[+]\033[0m CLIENT -> CTX: CLIENT: " << ctx->ident << " | CGI ID: " << ctx->cgi_pid << " | is_cgi : " << ctx->is_cgi  << std::endl;

            if (filter == EVFILT_PROC)
            {
                // process/child events (CGI)
                if (fflags & NOTE_EXIT)
                {
                    std::cout << "\033[32m[+]\033[0m \033[34mEnter PROC CGI: " << ctx->ident << " | CGI PID: " << ctx->cgi_pid << " | IS CGI: " << ctx->is_cgi <<  "\033[0m" << std::endl;
                    handle_cgi_exit(ctx, ctx->req, ctx->res);
                    continue;
                }
            }

            // Treat socket EOF only for actual socket filters (READ/WRITE).
            if ((filter == EVFILT_READ || filter == EVFILT_WRITE) && (flags & EV_EOF))
            {
                std::cout << "\033[31m[-]\033[0m Enter EOF client" << std::endl;
                RemoveClient(ctx->ident);
                continue;
            }

            if (flags & EV_ERROR)
            {
                std::cout << "\033[31mkevent error on fd " << ctx->ident << " : fflags=" << fflags << "\033[0m" << std::endl;
                RemoveClient(ctx->ident);
                continue;
            }

            // CGI And Normal Request
            if (ctx->is_cgi == false)
            {
                if (filter == EVFILT_READ)
                {
                    std::cout << "\033[32m[+]\033[0m \033[34mEnter Read client: " << ctx->ident << " | CGI PID: " << ctx->cgi_pid << " | IS CGI: " << ctx->is_cgi <<  "\033[0m" << std::endl;
                    handle_client_read(ctx, ctx->req, ctx->res);
                }
                else if (filter == EVFILT_WRITE)
                {
                    std::cout << "\033[32m[+]\033[0m \033[34mEnter Write client: " << ctx->ident << " | CGI PID: " << ctx->cgi_pid << " | IS CGI: " << ctx->is_cgi <<  "\033[0m" << std::endl;
                    handle_client_write(ctx, ctx->req, ctx->res, ConfigPars);
                }
                else if (filter == EVFILT_TIMER)
                {
                    std::cout << "\033[32m[+]\033[0m \033[34mEnter TimeOut Client: " << ctx->ident << " | CGI PID: " << ctx->cgi_pid << " | IS CGI: " << ctx->is_cgi <<  "\033[0m" << std::endl;
                    handle_timeout(ctx, *ctx->req);
                }
            }
            else
            {
                if (filter == EVFILT_TIMER)
                {
                    std::cout << "\033[32m[+]\033[0m \033[34mEnter TimeOut CGI: " << ctx->ident << " | CGI PID: " << ctx->cgi_pid << " | IS CGI: " << ctx->is_cgi <<  "\033[0m" << std::endl;
                    handle_cgi_timeout(ctx, *ctx->req, *ctx->res);
                }
            }
        }

        // ---- Deferred deletions: actually free contexts removed during this batch ----
		if (!pending_deletions.empty())
		{
			for (std::vector<EventContext*>::size_type k = 0; k < pending_deletions.size(); ++k)
			{
				EventContext* to_delete = pending_deletions[k];

				// Defensive: deregister any process/timer for every pid we recorded
				struct kevent kev;
				if (!to_delete->registered_procs.empty())
				{
					for (size_t pi = 0; pi < to_delete->registered_procs.size(); ++pi)
					{
						pid_t p = to_delete->registered_procs[pi];
						if (p <= 0) continue;
						EV_SET(&kev, p, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
						kevent(kq, &kev, 1, NULL, 0, NULL);
						EV_SET(&kev, p, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
						kevent(kq, &kev, 1, NULL, 0, NULL);
					}
					to_delete->registered_procs.clear();
				}
				else
				{
					pid_t proc_pid = to_delete->cgi_pid;
					if (proc_pid == 0 && to_delete->res)
						proc_pid = to_delete->res->_cgi.getpid_1();

					if (proc_pid > 0)
					{
						EV_SET(&kev, proc_pid, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
						kevent(kq, &kev, 1, NULL, 0, NULL);
						EV_SET(&kev, proc_pid, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
						kevent(kq, &kev, 1, NULL, 0, NULL);
					}
				}

				// Remove mapping entries in proc_map that point to this ctx (defensive)
				if (!proc_map.empty())
				{
					std::vector<int> keys_to_erase;
					for (std::map<int, EventContext*>::iterator mit = proc_map.begin(); mit != proc_map.end(); ++mit)
					{
						if (mit->second == to_delete)
							keys_to_erase.push_back(mit->first);
					}
					for (size_t kk = 0; kk < keys_to_erase.size(); ++kk)
						proc_map.erase(keys_to_erase[kk]);
				}

				// std::cout << "Final cleanup delete ctx: " << (void*)to_delete << std::endl;

				if (to_delete->req) { delete to_delete->req; to_delete->req = NULL; }
				if (to_delete->res) { delete to_delete->res; to_delete->res = NULL; }
				delete to_delete;
			}
			pending_deletions.clear();
		}
	}


}
