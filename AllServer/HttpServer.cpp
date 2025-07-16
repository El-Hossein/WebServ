#include "HttpServer.hpp"
#include <vector>
#include "../Response/responseHeader.hpp"
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
	std::cout << "--------------------------------------------------------" << std::endl;
}


Request* HttpServer::accept_new_client(int server_fd, std::vector<ConfigNode> ConfigPars) {
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);
	int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
	
	if (client_fd < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			std::cerr << "Accept failed on server_fd " << server_fd << ": " << strerror(errno) << std::endl;
		}
		return new Request(-1, ConfigPars);
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

	std::cout << "Client " << client_ip << ":" << client_port << " connected to server at " 
			<< server_ip << ":" << server_port << "\n" << std::endl;

	fcntl(client_fd, F_SETFL, O_NONBLOCK);
	
	struct kevent event;
	AddToKqueue(event, kq, client_fd, EVFILT_READ, EV_ADD | EV_ENABLE);
	AddToKqueue(event, kq, client_fd, EVFILT_WRITE, EV_ADD | EV_DISABLE);
	Request * new_request = new Request(client_fd, ConfigPars);
	new_request->SetNew(READ_HEADER);
	return new_request;
}


void	SetUpResponse(int &client_fd, Response * res, Request	&Request, std::vector<ConfigNode> ConfigPars)
{
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

void HttpServer::handle_client(int client_fd, struct kevent* event, std::vector<ConfigNode> ConfigPars, std::vector<Request*>& all, std::vector<Response*>& all_res) {
	// Find the corresponding Request object
	Request* request = NULL;
	for (size_t i = 0; i < all.size(); ++i) {
		if (all[i]->GetClientFd() == client_fd) {
			request = all[i];
			break;
		}
	}
	Response* response = NULL;
	for (size_t i = 0; i < all_res.size(); ++i) {
		if (all_res[i]->getClientFd() == client_fd) {
			response = all_res[i];
			break;
		}
	}
	// std::cout << request->GetClientFd() << std::endl;
	if (!request) return; // Request requestect not found

	if (event->filter == EVFILT_READ) {
		// std::cout << client_fd << std::endl;
		try {
			request->SetUpRequest();
		} catch (const char* e) {
			return;
		}
		if (request->GetNew() != END_BODY)
			return;
		SetUpResponse(client_fd, response, *request, ConfigPars);
		// std::cout << response_map[client_fd] << std::endl;
		struct kevent ev;
		AddToKqueue(ev, kq, client_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE);
		AddToKqueue(ev, kq, client_fd, EVFILT_READ, EV_DISABLE);
		// std::cerr << "Request processing failed: " << e << std::endl;
		// remove_client(client_fd);
		// response_map.erase(client_fd);
		// for (std::vector<Request*>::iterator it = all.begin(); it != all.end(); ++it) {
		//     if ((*it)->GetClientFd() == client_fd) {
		//         delete *it;
		//         all.erase(it);
		//         break;
		//     }
		// }
	}
	
	if (event->filter == EVFILT_WRITE) {
		std::string FullRes = response->getFinalResponse();
			ssize_t total_written = 0;
			ssize_t to_write = FullRes.length();

			while (total_written < to_write) {
				ssize_t bytes_written = write(client_fd, FullRes.c_str() + total_written, to_write - total_written);
				if (bytes_written < 0) {
					if (errno == EAGAIN || errno == EWOULDBLOCK) {
						return; // Wait for next write event
					}
					break; // Other errors, clean up
				}
				total_written += bytes_written;
			}
			unsigned long resp = FullRes.find("Connection: close");
			std::cout << resp << " : " << std::string::npos << std::endl;
			bool should_close = (resp != std::string::npos);
			// response_map.erase(client_fd);

			struct kevent ev;
			AddToKqueue(ev, kq, client_fd, EVFILT_WRITE, EV_DISABLE);
			
			if (true) {
				// std::cout << client_fd << std::endl;
				remove_client(client_fd);
				for (std::vector<Request*>::iterator it = all.begin(); it != all.end(); ++it) {
					if ((*it)->GetClientFd() == client_fd) {
						delete *it;
						all.erase(it);
						break;
					}
				}
			} else {
				AddToKqueue(ev, kq, client_fd, EVFILT_READ, EV_ADD | EV_ENABLE);
			}
	}
}

void HttpServer::run(std::vector<ConfigNode> ConfigPars) {
	struct kevent events[BACKLOG];
	std::vector<Request*> all_request;
	std::vector<Response*> all_res;
	
	while (true) {
		int nev = kevent(kq, NULL, 0, events, BACKLOG, NULL);
		if (nev < 0) {
			std::cerr << "kevent error: " << strerror(errno) << std::endl;
			continue;
		}

		for (int i = 0; i < nev; ++i) {
			int fd = events[i].ident;
			// Check if it's a server socket`
			for (size_t j = 0; j < server_fds.size(); ++j) {
				if (server_fds[j] == fd) {
					Request* new_request = accept_new_client(fd, ConfigPars);
					if (new_request->GetClientFd() != -1) {
						Response * res = new Response(*new_request, new_request->GetClientFd());
						all_request.push_back(new_request);
						all_res.push_back(res);
						handle_client(new_request->GetClientFd(), &events[i], ConfigPars, all_request, all_res);
					} else {
						delete new_request;
					}
					break;
				}
			}
			
			// Handle client socket
			for (size_t j = 0; j < all_request.size(); ++j) {
				if (all_request[j]->GetClientFd() == fd) {
					handle_client(fd, &events[i], ConfigPars, all_request, all_res);
					break;
				}
			}
		}
	}
}
