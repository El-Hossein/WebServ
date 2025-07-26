#include "cgiHeader.hpp"

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
                    cgiStatusCode = atoi(headerValue.c_str());
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
        cgiHeader = "Content-Type: text/html\r\n" + cgiHeader;
    
}

char    **cgiEnvVariables(const Request &req, std::vector<ConfigNode> ConfigPars, std::string _pathInfo)
{
    char **envp = new char*[9];


    // REQUEST_METHOD
    envp[0] = new char[strlen("REQUEST_METHOD=") + strlen(req.GetHeaderValue("method").c_str()) + 1];
    strcpy(envp[0], "REQUEST_METHOD=");
    strcat(envp[0], req.GetHeaderValue("method").c_str());
    // std::cout << envp[0] << std::endl;
    //SCRIPT_NAME
    envp[1] = new char[strlen("SCRIPT_NAME=") + strlen(req.GetHeaderValue("path").c_str()) + 1];
    strcpy(envp[1], "SCRIPT_NAME=");
    strcat(envp[1], req.GetHeaderValue("path").c_str()); 
    // std::cout << envp[1] << std::endl;
    //SCRIPT_FILENAME
    envp[2] = new char[strlen("SCRIPT_FILENAME=") + strlen(req.GetFullPath().c_str()) + 1];
    strcpy(envp[2], "SCRIPT_FILENAME=");
    strcat(envp[2], req.GetFullPath().c_str());
    // std::cout << envp[2] << std::endl;
    //QUERY_STRING
    envp[3] = new char[strlen("QUERY_STRING=") + strlen(req.GetHeaderValue("query").c_str()) + 1];
    strcpy(envp[3], "QUERY_STRING=");
    strcat(envp[3], req.GetHeaderValue("query").c_str());
    // std::cout << envp[3] << std::endl;
    //SERVER_PROTOCOL
    envp[4] = new char[strlen("SERVER_PROTOCOL=") + strlen("HTTP/1.1") + 1];
    strcpy(envp[4], "SERVER_PROTOCOL=");
    strcat(envp[4], "HTTP/1.1");
    // std::cout << envp[4] << std::endl;
    //GATEWAY_INTERFACE
    envp[5] = new char[strlen("GATEWAY_INTERFACE=") + strlen("CGI/1.1") + 1];
    strcpy(envp[5], "GATEWAY_INTERFACE=");
    strcat(envp[5], "CGI/1.1");
    // std::cout << envp[5] << std::endl;
    //SERVER_SOFTWARE
    envp[6] = new char[strlen("SERVER_SOFTWARE=") + strlen("webSERV/1.0") + 1];
    strcpy(envp[6], "SERVER_SOFTWARE=");
    strcat(envp[6], "webSERV/1.0");
    // std::cout << envp[6] << std::endl;
    //PATH_INFO
    envp[7] = new char[strlen("PATH_INFO=") + strlen(_pathInfo.c_str()) + 1];
    strcpy(envp[7], "PATH_INFO=");
    strcat(envp[7], _pathInfo.c_str());
    // std::cout << envp[7] << std::endl;
    // need to add SERVER_NAME, SERVER_PORT, CONTENT_TYPE, CONTENT_LENGTH
    envp[8] = NULL;
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
    exit(1);
}


int Cgi::executeCgiScript(Request &req, std::vector<ConfigNode> ConfigPars)
{
    std::ostringstream inp;
    std::ostringstream out;
    inp << "/tmp/cgiInput_" << uniq;
    out << "/tmp/cgiOutput_" << uniq;
    inpFile = inp.str();
    outFile = out.str();
    pid = fork();
    if (pid == -1)
       return responseErrorcgi(500, " Internal Server Error", ConfigPars, req);
    else if (pid == 0)
    {
        std::freopen(inpFile.c_str(), "r", stdin);
        std::freopen(outFile.c_str(), "w", stdout);
        std::freopen(outFile.c_str(), "w", stderr);
        envp = cgiEnvVariables(req, ConfigPars, pathInfo);
        execCgi(scriptFile.c_str(), envp);
    }
    else
    {
        startTime = time(NULL);
        
        // Write POST data
        // if (req.GetHeaderValue("method") == "POST")
        // {
        //     std::ofstream inputFileStream(inpFile.c_str());
        //     if (inputFileStream.is_open())
        //     {
        //         if (postRequestBody && strlen(postRequestBody) > 0)
        //             inputFileStream << postRequestBody;
        //         inputFileStream.close();
        //     }
        // }
        cgistatus = CGI_RUNNING;

        struct kevent kev;
        EV_SET(&kev, pid, EVFILT_PROC, EV_ADD | EV_ENABLE, NOTE_EXIT, 0, NULL);
        kevent(globalKq, &kev, 1, NULL, 0, NULL);
        return 2;
    }
    return true;
}