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

std::string executeCgiScript(const char* script_path, const Request &req)
{
    const char *postRequestBody = "name=ismail"; //needs to be placed with the real one from the socket
    std::string output;

    std::string inpFile = "/tmp/cgiInput";
    std::string outFile = "/tmp/cgiOutput";

    pid_t pid = fork();
    if (pid == -1)
        return "";

    if (pid == 0)
    {
        freopen(inpFile.c_str(), "r", stdin);
        freopen(outFile.c_str(), "w", stdout);
        freopen(outFile.c_str(), "w", stderr);

        char* envp[12];
        char buffer1[64], buffer2[256], buffer3[256], buffer4[64], buffer5[64], buffer6[64], buffer7[256], buffer8[64], buffer9[128], buffer10[64], buffer11[64];

        // sprintf(buffer1, "REQUEST_METHOD=%s", "GET");
        // sprintf(buffer2, "SCRIPT_NAME=%s", req.GetFullPath().c_str());
        // sprintf(buffer3, "SCRIPT_FILENAME=%s", script_path);
        // sprintf(buffer4, "SERVER_SOFTWARE=YourWebServer/1.0");
        // sprintf(buffer5, "SERVER_PROTOCOL=HTTP/1.1");
        // sprintf(buffer6, "GATEWAY_INTERFACE=CGI/1.1");
        // if (strlen("ismail") > 0)
        //     sprintf(buffer7, "QUERY_STRING=%s", "ismail");
        // else
        //     sprintf(buffer7, "QUERY_STRING=");
        // if (strcmp("POST", "POST") == 0)
        //     sprintf(buffer8, "CONTENT_LENGTH=%ld", strlen(postRequestBody));
        // else
        //     sprintf(buffer8, "CONTENT_LENGTH=%d", 0);
        // sprintf(buffer9, "CONTENT_TYPE=application/x-www-form-urlencoded");
        // sprintf(buffer10, "SERVER_NAME=localhost");
        // sprintf(buffer11, "SERVER_PORT=8080");

        envp[0] = buffer1;
        envp[1] = buffer2;
        envp[2] = buffer3;
        envp[3] = buffer4;
        envp[4] = buffer5;
        envp[5] = buffer6;
        envp[6] = buffer7;
        envp[7] = buffer8;
        envp[8] = buffer9;
        envp[9] = buffer10;
        envp[10] = buffer11;
        envp[11] = NULL;

        char* argv[3];
        if (strstr(script_path, ".py"))
        {
            argv[0] = (char*)"python3";
            argv[1] = (char*)script_path;
            argv[2] = NULL;
            execve("/usr/bin/python3", argv, envp);
        }
        else if (strstr(script_path, ".cgi"))
        {
            argv[0] = (char*)script_path;
            argv[1] = NULL;
            execve(script_path, argv, envp);
        }
        else if (strstr(script_path, ".php"))
        {
            argv[0] = (char*)"php";
            argv[1] = (char*)script_path;
            argv[2] = NULL;
            execve("/bin/php", argv, envp);
        }
        perror("execve failed");
        exit(1);
    }
    else
    {
        // this is for POST data 
        FILE* inputFileStream = fopen(inpFile.c_str(), "w");
        if (inputFileStream)
        {
            if (postRequestBody && strlen(postRequestBody) > 0)
                fwrite(postRequestBody, 1, strlen(postRequestBody), inputFileStream);
            fclose(inputFileStream);
        }

        int status;
        waitpid(pid, &status, 0);
        if (status != 0)
        {
            unlink(inpFile.c_str());
            unlink(outFile.c_str());
            return "";
        }
        FILE* outFileStream = fopen(outFile.c_str(), "r"); // open the output file to read
        if (outFileStream)
        {
            char buffer[4096];
            size_t bytes_read;

            while ((bytes_read = fread(buffer, 1, sizeof(buffer) - 1, outFileStream)) > 0)
            {
                buffer[bytes_read] = '\0';
                output += buffer;
            }
            fclose(outFileStream);
        }

        unlink(inpFile.c_str());
        unlink(outFile.c_str());
    }
    return output;
}