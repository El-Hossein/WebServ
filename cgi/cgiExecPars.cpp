#include "cgiHeader.hpp"

CgiResponse parseOutput(const std::string& scriptOutput)
{
    CgiResponse response;
    response.status_code = 200;

    std::istringstream stream(scriptOutput);
    std::string line;
    bool readHeader = true;

    while (std::getline(stream, line))
    {
        if (!line.empty() && line[line.size()-1] == '\r')
            line.erase(line.size() - 1);
        if (readHeader)
        {
            if (line.empty() || line == "\r") //    we reached the end of headers
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
                    response.status_code = atoi(headerValue.c_str());
                else
                    response.headers += line + "\r\n";
            }
            else
            {
                readHeader = false;
                response.body += line + "\n";
            }
        }
        else
            response.body += line + "\n";
    }
    if (response.headers.find("Content-Type:") == std::string::npos)
        response.headers = "Content-Type: text/html\r\n" + response.headers;

    return response;
}

char    **cgiEnvVariables(const Request &req, std::vector<ConfigNode> ConfigPars)
{
    char **envp = new char*[8];


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
    // need to add SERVER_NAME, SERVER_PORT, PATH_INFO, CONTENT_TYPE, CONTENT_LENGTH
    envp[7] = NULL;
    return envp;
}

void    execCgi(const char *scriptPath, char **envp)
{
    char* argv[3];
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
        argv[1] = (char*)scriptPath;
        argv[2] = NULL;
        execve("/bin/php", argv, envp);
    }
    perror("execve failed");
    exit(1);
}

std::string executeCgiScript(const char* scriptPath, const Request &req, std::vector<ConfigNode> ConfigPars)
{
    const char *postRequestBody = "name=ismail";// testing purposes
    std::string inpFile = "/tmp/cgiInput";
    std::string outFile = "/tmp/cgiOutput";
    std::string output;

    pid_t pid = fork();
    if (pid == -1)
        return "";
    else if (pid == 0)
    {
        freopen(inpFile.c_str(), "r", stdin);
        freopen(outFile.c_str(), "w", stdout);
        freopen(outFile.c_str(), "w", stderr);
        char **envp;
        envp = cgiEnvVariables(req, ConfigPars);
        execCgi(scriptPath, envp);
    }
    else
    {
        // this is for POST data 
        if (req.GetHeaderValue("method") == "POST")
        {
            std::ofstream inputFileStream(inpFile.c_str());
            if (inputFileStream.is_open())
            {
                if (postRequestBody && strlen(postRequestBody) > 0)
                    inputFileStream << postRequestBody;
                inputFileStream.close();
            }
        }

        int status;
        waitpid(pid, &status, 0);
        if (status != 0)
        {
            unlink(inpFile.c_str());
            unlink(outFile.c_str());
            return "";
        }
        std::ifstream outFileStream(outFile.c_str());
        if (outFileStream.is_open())
        {
            std::ostringstream buffer;
            buffer << outFileStream.rdbuf();
            output = buffer.str();
            outFileStream.close();
        }
        unlink(inpFile.c_str());
        unlink(outFile.c_str());
    }
    return output;
}