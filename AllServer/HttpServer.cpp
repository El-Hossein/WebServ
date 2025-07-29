#include "HttpServer.hpp"
#include <vector>
#include "../Response/responseHeader.hpp"

int globalKq = -1;

HttpServer::HttpServer()	{ }

HttpServer::HttpServer(const HttpServer & other)	{ *this = other; }

HttpServer::~HttpServer()	{ }

// Get all the ports from the configuration file
void GetAllPorts(std::vector<ConfigNode> ConfigPars, std::vector<std::vector<int> > &AllPorts)
{
	for (size_t i = 0; i < ConfigPars.size(); i++)
	{
		std::vector<std::string> ConfPort = ConfigPars[i].getValuesForKey(ConfigPars[i], "listen", "NULL");
		if (!ConfPort.empty())
		{
			std::vector<int> ports;
			for (size_t j = 0; j < ConfPort.size(); j++)
				ports.push_back(atoi(ConfPort.at(j).c_str()));
			AllPorts.push_back(ports);
		}
	}
}

// Set up the server address for binding
void SetUpForBind(struct sockaddr_in &server_addr, int port)
{
	// Bind the socket to an address
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);
}
// Bind the socket to an address and listen on the socket
int BindAndListen(int server_fd, struct sockaddr_in server_addr, int port, int i)
{
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		return (std::cerr << "setsockopt failed for server " << i << "\n", 1);
	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		return  (std::cerr << "Bind failed on port " << port << " for server " << i << "\n", 1);

	// Listen on the socket
	if (listen(server_fd, BACKLOG) < 0)
		return  (std::cerr << "Listen failed on port " << port << " for server " << i << "\n", 1);
	return  0;
}
// Add the socket to the kqueue
void AddToKqueue(struct kevent &event, int kq, int fd, int filter, int flags)
{
	EV_SET(&event, fd, filter, flags, 0, 0, NULL);
	if (kevent(kq, &event, 1, NULL, 0, NULL) == -1)
	{
		std::cerr << "kevent failed for fd " << fd << ": " << strerror(errno) << std::endl;
		// Don’t throw yet—subject wants resilience, so log and continue
	}
}

//done with the server setup
void HttpServer::setup_server(std::vector<ConfigNode> ConfigPars) {
	std::cout << "Setting up server..." << std::endl;
	kq = kqueue();
	if (kq == -1) throw std::runtime_error("Failed to create kqueue");

	globalKq = kq;
	
	std::vector<std::vector<int> > AllPorts;
	GetAllPorts(ConfigPars, AllPorts);

	for (size_t i = 0; i < AllPorts.size(); i++) {
		for (size_t j = 0; j < AllPorts[i].size(); j++) {
			int server_fd = socket(AF_INET, SOCK_STREAM, 0);
			if (server_fd == -1) {
				std::cerr << "Socket creation failed for server " << i << " on port " << AllPorts[i][j] << "\n";
				continue;
			}

			int opt = 1;
			setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
			fcntl(server_fd, F_SETFL, O_NONBLOCK);

			struct sockaddr_in server_addr;
			SetUpForBind(server_addr, AllPorts[i][j]);
			if (BindAndListen(server_fd, server_addr, AllPorts[i][j], i) == 1) {
				close(server_fd);
				continue;
			}

			struct kevent event;
			AddToKqueue(event, kq, server_fd, EVFILT_READ, EV_ADD | EV_ENABLE);
			server_fds.push_back(server_fd);
			std::cout << "Server " << i << " listening on port " << AllPorts[i][j] << std::endl;
		}
	}
	std::cout << "------------------------------------------------------------------------------" << std::endl;
}


Request* HttpServer::accept_new_client(int server_fd, std::vector<ConfigNode> ConfigPars) {
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);
	int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
	
	if (client_fd < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			std::cerr << "Accept failed on server_fd " << server_fd << ": " << strerror(errno) << std::endl;
		}
		int tmp(0);return new Request(-1, ReadHeader,ConfigPars, tmp);
	}

	char client_ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
	int client_port = ntohs(client_addr.sin_port);
	
	struct sockaddr_in server_addr;
	socklen_t server_addr_len = sizeof(server_addr);
	getsockname(client_fd, (struct sockaddr*)&server_addr, &server_addr_len);
	
	char server_ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &server_addr.sin_addr, server_ip, INET_ADDRSTRLEN);
	int server_port = ntohs(server_addr.sin_port);

	std::cout << "-----------------------------------------------------------------------------" << std::endl;
	std::cout << "Client " << client_ip << ":" << client_port << " connected to server at " 
			<< server_ip << ":" << server_port << "\n" << std::endl;

	fcntl(client_fd, F_SETFL, O_NONBLOCK);
	
	struct kevent event;
	AddToKqueue(event, kq, client_fd, EVFILT_READ, EV_ADD | EV_ENABLE);
	AddToKqueue(event, kq, client_fd, EVFILT_WRITE, EV_ADD | EV_DISABLE);
	Request * new_request = new Request(client_fd, ReadHeader, ConfigPars, server_port);
	std::cout << "[" << client_fd << "]" << std::endl;
	return new_request;
}


int	stringtoint(const char *e)
{
	int num = 0;

	std::stringstream ss(e);
	ss >> num;
	return num;
}

void	SetUpResponse(int &client_fd, Response * res, Request	&Request, std::vector<ConfigNode> ConfigPars, const char *e)
{
	// int num = stringtoint(e);
	// switch (num)
	// {
	// 	case 500: res->responseError(500, " Internal Server Error", ConfigPars); return;
	// 	case 501: res->responseError(501, " Not Implemented", ConfigPars); return;
	// 	case 400: res->responseError(400, " Bad Request", ConfigPars); return;
	// 	case 413: res->responseError(413, " Content Too Large", ConfigPars); return;
	// 	case 414: res->responseError(414, " URI Too Long", ConfigPars); return;
	// }
	(void)e;
	res->moveToResponse(client_fd, Request, ConfigPars);
}

// remove the client from the kqueue and close the connection
void HttpServer::remove_client(int client_fd)
{
	struct kevent event;
	// Remove the client socket from the kqueue for reading
	AddToKqueue(event, kq, client_fd, EVFILT_READ, EV_DELETE);
	// Remove the client socket from the kqueue for writing
	AddToKqueue(event, kq, client_fd, EVFILT_WRITE, EV_DELETE);
	close(client_fd);
}

void remove_request(int fd, std::vector<Request*>& all) {
	for (std::vector<Request*>::iterator it = all.begin(); it != all.end(); ++it) {
		if ((*it)->GetClientFd() == fd) {
			delete *it;        // Free memory
			all.erase(it);     // Remove pointer from vector
			return;
		}
	}
}
Request * RightRequest(int client_fd, std::vector<Request*>& all_req)
{
	for (size_t i = 0; i < all_req.size(); ++i) {
		if (all_req[i]->GetClientFd() == client_fd) {
			return  all_req[i];
		}
	}
	return NULL;
}
Response * RightResponse(int client_fd, std::vector<Response*>& all_res)
{
	for (size_t i = 0; i < all_res.size(); ++i) {
		if (all_res[i]->getClientFd() == client_fd) {
			return all_res[i];
		}
	}
	return NULL;
}

void HttpServer::handle_client(int client_fd, struct kevent* event, std::vector<ConfigNode> ConfigPars, std::vector<Request*>& all_req, std::vector<Response*>& all_res) {

	// Find the corresponding Request object
	Request		*request = RightRequest(client_fd, all_req);
	Response	*response = RightResponse(client_fd, all_res);

	if (!request)
		return; // Request requestect not found
	if (event->filter == EVFILT_READ)
	{
		try { request->SetUpRequest(); }
			catch (const char* e) { return; }
		if (request->GetClientStatus() != EndReading)
			return;

		// std::cout << "++++++++++++++++++++++++++++++" << std::endl;
		// std::map<std::string, std::string> all_header = request->GetHeaders();
		// for (std::map<std::string, std::string>::const_iterator it = all_header.begin(); it != all_header.end(); it++)
		// 	std::cout << it->first << ": " << it->second << std::endl;
		// std::cout << "++++++++++++++++++++++++++++++" << std::endl;

							SetUpResponse(client_fd, response, *request, ConfigPars, NULL);

		// std::cout << "++++++++++++++++++++++++++++++" << std::endl;
		// response->setHasMore(response->getNextChunk(100000));
		// response->setBytesSent(0);
		// std::cout << "response: " << response->getChunk().c_str() << std::endl;
		// std::cout << "++++++++++++++++++++++++++++++" << std::endl;
							if (response->_cgi.gethasPendingCgi())
								return;
							else
							{
								struct kevent ev;
								AddToKqueue(ev, kq, client_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE);
								AddToKqueue(ev, kq, client_fd, EVFILT_READ, EV_DISABLE);
							}
	}

	if (event->filter == EVFILT_WRITE)
	{
		// Get first chunk or next chunk if current one is fully sent
		if (response->getChunk().empty() || response->getBytesSent() >= response->getChunk().size())
		{
			response->setHasMore(response->getNextChunk(100000));
			response->setBytesSent(0);
		}
		if (!response->getChunk().empty())
		{
			response->setBytesWritten(send(client_fd, response->getChunk().c_str() + response->getBytesSent(), response->getChunk().size() - response->getBytesSent(), 0));
			if (response->getBytesWritten() < 0)
				return;
			else if (response->getBytesWritten() == 0)
			{
				remove_client(client_fd);
				return;
			}
			response->setBytesSent(response->getBytesSent() + response->getBytesWritten());
			if (response->getBytesSent() < response->getChunk().size())
				return;
		}

		if (!response->getHasMore())
		{
			response->setHeaderSent(0);
			response->_cgi.setCgiHeaderSent(0);
			

			struct kevent ev;
			AddToKqueue(ev, kq, client_fd, EVFILT_WRITE, EV_DISABLE);
			if (true)
			{
				response->_cgi.setCheckConnection(_Empty);
				remove_client(client_fd);
				for (std::vector<Request*>::iterator it = all_req.begin(); it != all_req.end(); ++it)
				{
					if ((*it)->GetClientFd() == client_fd)
					{
						delete *it;
						all_req.erase(it);
						break;
					}
				}
				for (std::vector<Response*>::iterator it = all_res.begin(); it != all_res.end(); ++it) {
					if ((*it)->getClientFd() == client_fd) {
						delete *it;
						all_res.erase(it);
						break;
					}
				}
			} else {
				AddToKqueue(ev, kq, client_fd, EVFILT_READ, EV_ADD | EV_ENABLE);
			}
		}
	}
}

void HttpServer::run(std::vector<ConfigNode> ConfigPars) {
    struct kevent events[BACKLOG];
    std::vector<Request*> all_request;
    std::vector<Response*> all_res;
    
    while (true) {
        struct timespec timeout;
        timeout.tv_sec = 0;
        timeout.tv_nsec = 100000000;
        
        int nev = kevent(kq, NULL, 0, events, BACKLOG, &timeout);
        if (nev < 0) {
            std::cerr << "kevent error: " << strerror(errno) << std::endl;
            continue;
        }

        // Handle all kevent events
		for (int i = 0; i < nev; ++i) {
			// 1. Handle CGI process exit notification
			if (events[i].filter == EVFILT_PROC && (events[i].fflags & NOTE_EXIT)) {
				pid_t exitedPid = events[i].ident;

				// Match this PID with the correct Response object
				for (size_t j = 0; j < all_res.size(); ++j) {
					if (all_res[j]->_cgi.gethasPendingCgi() &&
						all_res[j]->_cgi.getpid_1() == exitedPid) {
						int tempClientFd = all_res[j]->getClientFd();
						Request* _reqPtr = RightRequest(tempClientFd, all_request);
						if (all_res[j]->checkPendingCgi(ConfigPars, *_reqPtr)) {
							struct kevent ev;
							int client_fd = all_res[j]->getClientFd();
							AddToKqueue(ev, kq, client_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE);
							AddToKqueue(ev, kq, client_fd, EVFILT_READ, EV_DISABLE);
						}

						break; // Stop once found
					}
				}

				continue; // done with this event
			}

			// 2. Check if it's a server socket
			int fd = events[i].ident;
			for (size_t j = 0; j < server_fds.size(); ++j) {
				if (server_fds[j] == fd) {
					Request* new_request = accept_new_client(fd, ConfigPars);
					if (new_request->GetClientFd() != -1) {
						Response * res = new Response(*new_request, new_request->GetClientFd());
						all_request.push_back(new_request);
						all_res.push_back(res);
						handle_client(new_request->GetClientFd(), &events[i], ConfigPars, all_request, all_res);
					}
					else {
						delete new_request;
					}
					break;
				}
			}

			// 3. Handle client socket
			for (size_t j = 0; j < all_request.size(); ++j) {
				if (all_request[j]->GetClientFd() == fd) {
					handle_client(fd, &events[i], ConfigPars, all_request, all_res);
					break;
				}
			}
		}
        
        // Handle CGI processes
        for (size_t i = 0; i < all_res.size(); ++i)
        {
            if (all_res[i]->_cgi.gethasPendingCgi())
            {
                // Check for timeout
                time_t currentTime = time(NULL);
                time_t cgiStart = all_res[i]->_cgi.gettime();
                if (currentTime - cgiStart > 10)
                {
                    pid_t pid = all_res[i]->_cgi.getpid_1();
                    
                    struct kevent kev;
                    EV_SET(&kev, pid, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
                    kevent(globalKq, &kev, 1, NULL, 0, NULL);
                    
                    kill(pid, SIGTERM);
                    usleep(100000);
                    
                    int status;
                    int result = waitpid(pid, &status, WNOHANG);
                    if (result == 0)
                    {
                        kill(pid, SIGKILL);
                        usleep(50000);
                        waitpid(pid, &status, 0);
                    }
					int tempClientFd = all_res[i]->getClientFd();
					Request* reqPtr = RightRequest(tempClientFd, all_request);
                    all_res[i]->_cgi.responseErrorcgi(504, " Gateway Timeout", ConfigPars, *reqPtr);
                    all_res[i]->_cgi.setcgistatus(CGI_ERROR);
                    all_res[i]->_cgi.sethasPendingCgi(false);

                    struct kevent ev;
                    int client_fd = all_res[i]->getClientFd();
                    AddToKqueue(ev, kq, client_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE);
                    AddToKqueue(ev, kq, client_fd, EVFILT_READ, EV_DISABLE);
                }
            }
        }
    }
}