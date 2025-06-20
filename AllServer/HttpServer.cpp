#include "HttpServer.hpp"
#include <vector>

HttpServer::HttpServer()	{ }

HttpServer::HttpServer(const HttpServer & other)	{ *this = other; }

HttpServer::~HttpServer()	{ }

// Get all the ports from the configuration file
void GetAllPorts(std::vector<ConfigNode> ConfigPars, std::vector<std::vector<int> > &AllPorts)
{
    for (size_t i = 0; i < ConfigPars.size(); i++)
    {
        std::vector<std::string>* ConfPort = ConfigPars[i].getValuesForKey(ConfigPars[i], "listen", "NULL");
        if (ConfPort != NULL)
        {
            std::vector<int> ports;
            for (size_t j = 0; j < ConfPort->size(); j++)
                ports.push_back(atoi(ConfPort->at(j).c_str()));
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
void HttpServer::setup_server(std::vector<ConfigNode> ConfigPars){
    std::cout << "Setting up server..." << std::endl;
    kq = kqueue();
    if (kq == -1) throw std::runtime_error("Failed to create kqueue");
    
    std::vector<std::vector<int> > AllPorts;
    GetAllPorts(ConfigPars, AllPorts);

    for (size_t i = 0; i < AllPorts.size(); i++)
    {
        for (size_t j = 0; j < AllPorts[i].size(); j++)
        {
            // Create a socket
            int server_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (server_fd == -1)
            {
                std::cerr << "Socket creation failed for server " << i << " on port " << AllPorts[i][j] << "\n";
                continue;
            }
            // Forcefully attaching socket to the port
            int opt = 1;
            setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
            
            // Set the socket to non-blocking
            fcntl(server_fd, F_SETFL, O_NONBLOCK);

            // Bind the socket to an address
            struct sockaddr_in server_addr;
            SetUpForBind(server_addr, AllPorts[i][j]);
            if (BindAndListen(server_fd, server_addr, AllPorts[i][j], i) == 1)
            {
                close(server_fd);
                continue;
            }
            
            // Add the server socket to the kqueue
            struct kevent event;
            AddToKqueue(event, kq, server_fd, EVFILT_READ, EV_ADD);
            // Add the server socket to the list of server sockets
            server_fds.push_back(server_fd);
            std::cout << "Server " << i << " listening on port " << AllPorts[i][j] << std::endl;
        }
    }
    std::cout << "--------------------------------------------------------" << std::endl;
}

void HttpServer::accept_new_client(int server_fd)
{
    // Accept a new client
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK) // Non-blocking expected
            std::cerr << "Accept failed on server_fd " << server_fd << ": " << strerror(errno) << std::endl;
        return;
    }

    // Print client and server connection details
    // Get client connection details
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(client_addr.sin_port);
    
    // Get server connection details
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(server_addr);
    getsockname(client_fd, (struct sockaddr*)&server_addr, &server_addr_len);
    
    char server_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &server_addr.sin_addr, server_ip, INET_ADDRSTRLEN);
    int server_port = ntohs(server_addr.sin_port);

    std::cout << "Client " << client_ip << ":" << client_port << " connected to server at " 
              << server_ip << ":" << server_port << "\n" << std::endl;
    
    // Set the client socket to non-blocking
    fcntl(client_fd, F_SETFL, O_NONBLOCK);
    
    struct kevent event;
    // Add the client socket to the kqueue for reading
    AddToKqueue(event, kq, client_fd, EVFILT_READ, EV_ADD);
    
    AddToKqueue(event, kq, client_fd, EVFILT_WRITE, EV_ADD | EV_DISABLE);
    // Add the client socket to the kqueue for writing, but initially disabled
    std::cout << "Accept new clien: " << client_fd << std::endl;
}

void	SetUpResponse(int &client_fd, std::map<int, std::string>& response_map, Request	&request)
{
    std::string response;
	bool keep_alive = false;
    std::string body = "Hello, World! " + std::to_string(client_fd) + "\n";
    int content_length = body.length(); //

	PairedVectorSS	headers = request.GetHeaders();
    for (size_t i = 0; i < headers.size(); i++)
    {
        if (headers[i].first == "Protocol" && headers[i].second == "HTTP/1.1")
            keep_alive = true;  // HTTP/1.1 defaults to keep-alive unless told otherwise
        if (headers[i].first == "Connection" && headers[i].second == "keep-alive")
            keep_alive = true;  // Explicit keep-alive request
        if (headers[i].first == "Connection" && headers[i].second == "close")
            keep_alive = false; // Client wants to close connection
    }
    if (keep_alive)
        response = "HTTP/1.1 200 OK\r\n"
                   "Connection: keep-alive\r\n"
                   "Keep-Alive: timeout=1, max=100\r\n"
                   "Content-Length: " + std::to_string(content_length) + "\r\n"
                   "\r\n" + body;
    else
        response = "HTTP/1.1 200 OK\r\n"
                   "Connection: close\r\n"
                   "Content-Length: " + std::to_string(content_length) + "\r\n"
                   "\r\n" + body;
    // Create the response
    response_map[client_fd] = response;
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

void	HttpServer::handle_client(int client_fd, int filter, std::vector<ConfigNode> ConfigPars)
{
    (void)ConfigPars;
	if (filter == EVFILT_READ)
    {
		// -------------	Process request	 ------------- //
		Request obj(client_fd, ConfigPars);
		obj.SetUpRequest();
        // -------------	Process respons		 ------------- //
        SetUpResponse(client_fd, response_map, obj);


        // Enable writing
        struct kevent event;
        AddToKqueue(event, kq, client_fd, EVFILT_WRITE, EV_ENABLE);
    }
    else if (filter == EVFILT_WRITE)
    {
        if (response_map.find(client_fd) != response_map.end())
        {
            const std::string &response = response_map[client_fd];
            ssize_t total_written = 0;
            ssize_t to_write = response.length();

            while (total_written < to_write)
            {
                ssize_t bytes_written = write(client_fd, response.c_str() + total_written, to_write - total_written);
                if (bytes_written < 0)
                {
                    // Can't write more now, keep the response in map and wait
                    if (errno == EAGAIN || errno == EWOULDBLOCK) return;
                    remove_client(client_fd);
                    response_map.erase(client_fd);
                    return;
                }
                total_written += bytes_written;
            }
            bool should_close = (response.find("Connection: close") != std::string::npos);
            response_map.erase(client_fd);

            // Disable writing
            struct kevent event;
            AddToKqueue(event, kq, client_fd, EVFILT_WRITE, EV_DISABLE);

            if (!should_close)
            {
                // std::cout << "Waiting for next request from client " << client_fd << std::endl;
                AddToKqueue(event, kq, client_fd, EVFILT_READ, EV_ENABLE);
            }
            else
                remove_client(client_fd);
        }
    }
}

void HttpServer::run(std::vector<ConfigNode> ConfigPars)
{
    struct kevent events[BACKLOG];

    while (true)
    {
        int nev = kevent(kq, NULL, 0, events, BACKLOG, NULL);
        if (nev < 0) throw std::runtime_error("kevent error");
        for (int i = 0; i < nev; i++)
        {
            int fd = events[i].ident;
            // Check if it's a server socket
            bool is_server = false;
            for (size_t j = 0; j < server_fds.size(); j++)
            {
                if (server_fds[j] == fd)
                {
                    accept_new_client(fd);
                    is_server = true;
                    break;
                }
            }
            if (!is_server)
                handle_client(fd, events[i].filter, ConfigPars);
        }
    }
}
