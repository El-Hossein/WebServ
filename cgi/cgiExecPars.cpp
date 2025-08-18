#include "cgiHeader.hpp"
#include "../AllServer/HttpServer.hpp"

extern int globalKq;

void     Cgi::parseOutput()
{
    cgiStatusCode = 200;
    std::ifstream file(outFile.c_str());
    std::string line;
    bool readHeader = true;
    cgiHeader = "";
    cgiBody = "";

    while (std::getline(file, line))
    {
        if (!line.empty() && line[line.size()-1] == '\r')
            line.erase(line.size() - 1);
        if (readHeader)
        {
            if (line.empty() || line == "\r")
            {
                readHeader = false;
                continue;
            }
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos)
            {
                std::string headerName = line.substr(0, colonPos);
                std::string headerValue = line.substr(colonPos + 1);
                while (!headerValue.empty() && (headerValue[0] == ' ' || headerValue[0] == '\t'))
                    headerValue.erase(0, 1);
                while (!headerValue.empty() && (headerValue[headerValue.size() - 1] == '\r' || headerValue[headerValue.size() - 1] == '\n'))
                    headerValue.erase(headerValue.size() - 1);
                if (headerName == "Status")
                    cgiStatusCode = std::atoi(headerValue.c_str());
                else if (headerName == "Content-Length")
                    cgiContentLength = std::atoi(headerValue.c_str());
                else
                    cgiHeader += line + "\r\n";
            }
            else
            {
                readHeader = false;
                cgiBody += line + "\n";
            }
        }
        else
            cgiBody += line + "\n";
    }
    cgiFileSize = cgiBody.size();
    if (cgiHeader.find("Content-Type:") == std::string::npos)
        cgiHeader += "Content-Type: text/html\r\n";
}

char    **cgiEnvVariables(Request &req, std::vector<ConfigNode> ConfigPars, std::string _pathInfo)
{
    char **envp = new char*[14];

    envp[0] = new char[strlen("REQUEST_METHOD=") + strlen(req.GetHeaderValue("method").c_str()) + 1];
    strcpy(envp[0], "REQUEST_METHOD=");
    strcat(envp[0], req.GetHeaderValue("method").c_str());
    envp[1] = new char[strlen("SCRIPT_NAME=") + strlen(req.GetHeaderValue("path").c_str()) + 1];
    strcpy(envp[1], "SCRIPT_NAME=");
    strcat(envp[1], req.GetHeaderValue("path").c_str());
    envp[2] = new char[strlen("SCRIPT_FILENAME=") + strlen(req.GetFullPath().c_str()) + 1];
    strcpy(envp[2], "SCRIPT_FILENAME=");
    strcat(envp[2], req.GetFullPath().c_str());
    envp[3] = new char[strlen("QUERY_STRING=") + strlen(req.GetHeaderValue("query").c_str()) + 1];
    strcpy(envp[3], "QUERY_STRING=");
    strcat(envp[3], req.GetHeaderValue("query").c_str());
    envp[4] = new char[strlen("SERVER_PROTOCOL=") + strlen("HTTP/1.1") + 1];
    strcpy(envp[4], "SERVER_PROTOCOL=");
    strcat(envp[4], "HTTP/1.1");
    envp[5] = new char[strlen("GATEWAY_INTERFACE=") + strlen("CGI/1.1") + 1];
    strcpy(envp[5], "GATEWAY_INTERFACE=");
    strcat(envp[5], "CGI/1.1");
    envp[6] = new char[strlen("SERVER_SOFTWARE=") + strlen("webSERV/1.0") + 1];
    strcpy(envp[6], "SERVER_SOFTWARE=");
    strcat(envp[6], "webSERV/1.0");
    envp[7] = new char[strlen("PATH_INFO=") + strlen(_pathInfo.c_str()) + 1];
    strcpy(envp[7], "PATH_INFO=");
    strcat(envp[7], _pathInfo.c_str());
    envp[8] = new char[strlen("CONTENT_TYPE=") + strlen(req.GetHeaderValue("content-type").c_str()) + 1];
    strcpy(envp[8], "CONTENT_TYPE=");
    strcat(envp[8], req.GetHeaderValue("content-type").c_str());
    envp[9] = new char[strlen("CONTENT_LENGTH=") + strlen(req.GetHeaderValue("content-length").c_str()) + 1];
    strcpy(envp[9], "CONTENT_LENGTH=");
    strcat(envp[9], req.GetHeaderValue("content-length").c_str());
    envp[10] = new char[strlen("SERVER_NAME=") + strlen(req.GetHeaderValue("host").c_str()) + 1];
    strcpy(envp[10], "SERVER_NAME=");
    strcat(envp[10], req.GetHeaderValue("host").c_str());
    envp[11] = new char[strlen("SERVER_PORT=") + strlen(req.GetServerDetails().ServerPort.c_str()) + 1];
    strcpy(envp[11], "SERVER_PORT=");
    strcat(envp[11], req.GetServerDetails().ServerPort.c_str());
    envp[12] = new char[strlen("HTTP_COOKIE=") + strlen(req.GetHeaderValue("cookie").c_str()) + 1];
    strcpy(envp[12], "HTTP_COOKIE=");
    strcat(envp[12], req.GetHeaderValue("cookie").c_str());
    envp[13] = NULL;
    return envp;
}

void    execCgi(const char *scriptPath, char **envp)
{
    char* argv[5];
    if (strstr(scriptPath, ".py"))
    {
        argv[0] = (char*)"python3";
        argv[1] = (char*)scriptPath;
        argv[2] = NULL;
        execve("/usr/bin/python3", argv, envp);
    }
    else if (strstr(scriptPath, ".cgi"))
    {
        argv[0] = (char*)scriptPath;
        argv[1] = NULL;
        execve(scriptPath, argv, envp);
    }
    else if (strstr(scriptPath, ".php"))
    {
        argv[0] = (char*)"php";
        argv[1] = (char*)"-d";
        argv[2] = (char*)"display_errors=0";
        argv[3] = (char*)scriptPath;
        argv[4] = NULL;
        execve("/usr/bin/php", argv, envp);
    }
    perror("execve failed");
    if (envp)
    {
        for (int i = 0; envp[i]; i++)
            delete[] envp[i];
        delete[] envp;
    }
    exit(1);
}


int Cgi::executeCgiScript(Request &req, std::vector<ConfigNode> ConfigPars)
{
    std::ostringstream out;
    out << "/tmp/cgiOutput_" << uniq;
    inpFile = req.GetCgiFileName();
    outFile = out.str();
    pid = fork();
    if (pid == -1)
       return responseErrorcgi(500, " Internal Server Error", ConfigPars, req);
    else if (pid == 0)
    {
        int inFd = open(inpFile.c_str(), O_RDONLY);
        if (inFd != -1)
        { 
            dup2(inFd, STDIN_FILENO);
            close(inFd);
        }
        int outFd = open(outFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (outFd != -1)
        {
            dup2(outFd, STDOUT_FILENO);
            dup2(outFd, STDERR_FILENO);
            close(outFd);
        }
        std::string scriptDir = scriptFile.substr(0, scriptFile.find_last_of("/"));
        if (!scriptDir.empty())
        {
            if (chdir(scriptDir.c_str()) == -1)
            {
                perror("chdir failed");
                exit(1);
            }
        }
        envp = cgiEnvVariables(req, ConfigPars, pathInfo);
        execCgi(scriptFile.c_str(), envp);
    }
    else
    {
        startTime = time(NULL);
        cgistatus = CGI_RUNNING;
        // std::cout << "\033[34mCGI PID : " << pid << "\033[0m" << std::endl;
        // link pid to request context
        if (req.ctx)
        {
            req.ctx->is_cgi = true;
            req.ctx->cgi_pid = pid;
        }

        struct kevent kev;
        // watch for child exit
        EV_SET(&kev, pid, EVFILT_PROC, EV_ADD | EV_ENABLE, NOTE_EXIT, 0, req.ctx);
        if (kevent(globalKq, &kev, 1, NULL, 0, NULL) == -1){
            std::cout << "\033[31mkevent failed for pid " << pid << ": " << strerror(errno) << " EVFILT_PROC CGI\033[0m" << std::endl;
        }
        else
        {
            // record we registered this proc for this context
            if (req.ctx)
            {
                req.ctx->registered_procs.push_back(pid);
                req.ctx->cgi_pid = pid; // already set but reinforce
            }
        }        // add a pid timer with 1s tick for responsive CGI timeout checking
        EV_SET(&kev, pid, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, 10, req.ctx);
        if (kevent(globalKq, &kev, 1, NULL, 0, NULL) == -1)
            std::cout << "\033[31mkevent failed for pid " << pid << ": " << strerror(errno) << " EVFILT_TIMER CGI\033[0m" << std::endl;

		req.SetTimeOut(std::time(NULL));
        // std::cout << "CGI CTX: CLIENT: " << (req.ctx ? req.ctx->ident : -1) << " | CGI ID: " << (req.ctx ? req.ctx->cgi_pid : pid) << " | is_cgi : " << (req.ctx ? req.ctx->is_cgi : true)  << std::endl;

        return 2;
    }
    return true;
}