#include "responseHeader.hpp"

Response::Response(){
    
}

Response::Response(Request	&req, int _clientFd)
{
    uri = req.GetFullPath();
    method = req.GetHeaderValue("method");
    pathRequested = req.GetHeaderValue("path");
    clientFd = _clientFd;
}

Response::~Response(){
    
}

int Response::getClientFd()
{
    return clientFd;
}

void    Response::setClientFd(int _clientFd)
{
    clientFd = _clientFd;
}

std::string Response::getMethod()
{
    return method;
}

std::string Response::getUri()
{
    return uri;
}

std::string Response::getFinalResponse()
{
    return finalResponse;
}

void    Response::setFinalResponse(std::string _finalResponse)
{
    finalResponse = _finalResponse;
}


std::string Response::deleteResponseSuccess(const std::string& message)
{
    std::string html = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Error</title></head>";
    html += "<body><h1>";
    html += message;
    html += "</h1></body></html>";

    return html;
}

std::string Response::generateListingDir()
{
    DIR *dirCheck = opendir(uri.c_str());
    if (!dirCheck)
        return "<html><body><h1>Cannot open directory</h1></body></html>";

    std::string html = "<html><head><title>Index of " + uri + "</title></head><body>";
    html += "<h1>Index of " + uri + "</h1><ul>";

    struct dirent *dir;
    while ((dir = readdir(dirCheck)) != NULL)
    {
        std::string name = pathRequested + (pathRequested[pathRequested.length() - 1] == '/' ? "": "/") + dir->d_name;
        if (name != ".")
            html += "<li><a href=\"" + name + "\">" + dir->d_name + "</a></li>";
    }
    closedir(dirCheck);
    html += "</ul></body></html>";
    return html;
}

std::string getInfoConfig(std::vector<ConfigNode> ConfigPars, std::string what, std::string location, int index)
{
    ConfigNode a = ConfigNode::GetServer(ConfigPars, "myserver1.com");// need to handle

    std::vector<std::string> autoIndex = a.getValuesForKey(a, what, location);
    if (autoIndex.empty())
        return "";
    return autoIndex[index];
}

std::string Response::generateAutoIndexOn()
{
    std::string body = generateListingDir();

    finalResponse = "HTTP/1.1 200 OK\r\n";
    finalResponse += "Content-Length: " + intToString(body.size()) + "\r\n";
    finalResponse += "Content-Type: text/html\r\n";
    finalResponse += "Connection: close\r\n\r\n";
    finalResponse += body;
    return finalResponse;

}

void    Response::servListingDiren(std::vector<ConfigNode> ConfigPars)
{
    autoIndexOn = getInfoConfig(ConfigPars, "autoindex", "NULL", 0); // need to pass which location and server
    index = getInfoConfig(ConfigPars, "index", "NULL", 0); // need to pass which location and server

    if (index.empty() == 0)
    {
        htmlFound = uri + "/" + index; // i need to check if slash already there or not
        std::ifstream htmlStream(htmlFound.c_str(), std::ios::binary); 
        if (htmlStream)
        {
            std::ostringstream htmlContent;
            htmlContent << htmlStream.rdbuf();
            htmlStream.close();
            std::string body = htmlContent.str();

            finalResponse = "HTTP/1.1 200 OK\r\n";
            finalResponse += "Content-Length: " + intToString(body.size()) + "\r\n";
            finalResponse += "Content-Type: text/html\r\n";
            finalResponse += "Connection: close\r\n\r\n";
            finalResponse += body;
        }
        else if (autoIndexOn == "on")
            generateAutoIndexOn();
        else
            finalResponse = responseError(403, " Forbidden", ConfigPars);
    }
    else if (autoIndexOn == "on")
        generateAutoIndexOn();
    else
        finalResponse = responseError(403, " Forbidden", ConfigPars);
}


std::string Response::checkContentType()
{
    size_t dotPos = uri.find(".");
    if (dotPos != std::string::npos)
    {
        std::string extension = uri.substr(dotPos);
        if (extension.compare(".png") == 0)
            return "Content-Type: image/png\r\n";
        else if (extension.compare(".jpeg") == 0)
            return "Content-Type: image/jpeg\r\n";
        else if (extension.compare(".mp4") == 0)
            return "Content-Type: video/mp4\r\n";
        else if (extension.compare("mpeg") == 0)
            return "Content-Type: audio/mpeg\r\n";
        else if (extension.compare("vorbis") == 0)
            return "Content-Type: audio/vorbis\r\n";
        else
            return "Content-Type: text/html\r\n";
    }
    else
        return "Content-Type: text/html\r\n";
    return "";

}

std::string Response::getResponse( Request	&req, std::vector<ConfigNode> ConfigPars)
{
    struct stat st;

    if (IsCgiRequest(uri.c_str()))
        finalResponse = handleCgiRequest(req, ConfigPars);
    else if (method == "GET")
    {
        if (stat(uri.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
            servListingDiren(ConfigPars);
        else
        {
            // if (access(pathRequested.c_str(), R_OK | W_OK) == -1)
            // {
            //     finalResponse = responseError(403, " Forbidden", ConfigPars);
            //     return finalResponse;
            // }
            std::ifstream inFile(uri.c_str(), std::ios::binary);
            if (inFile)
            {
                std::ostringstream outStringFIle;
                outStringFIle << inFile.rdbuf();
                inFile.close();
                std::string fileBody = outStringFIle.str();
                finalResponse = "HTTP/1.1 200 OK\r\n";
                finalResponse += "Content-Length: ";
                finalResponse += intToString(fileBody.size()) + "\r\n";
                finalResponse += checkContentType();
                // finalResponse += "Accept-Ranges: bytes\r\n";
                finalResponse += "Connection: close\r\n";
                finalResponse += "\r\n";
                finalResponse += fileBody;
            }
            else
            {
                // and need to display given error page
                finalResponse = responseError(404, " Not Found", ConfigPars);
            }
        }
    }
    else if (method == "POST")
    {
        // POST here
        finalResponse = responseError(404, " Not Found", ConfigPars);
    }
    return finalResponse;
}

std::string Response::deleteResponse(std::vector<ConfigNode> ConfigPars)
{
    struct stat st;

    if (stat(uri.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
    {
        finalResponse = responseError(403, " Permission Denied", ConfigPars);
    }
    else if (std::remove(uri.c_str()) == 0)
    {
        std::string body = deleteResponseSuccess("file succesefuly deleted!!");
        finalResponse = "HTTP/1.1 200 OK\r\n";
        finalResponse += "Content-Type: text/html\r\n";
        finalResponse += "Content-Length: " + intToString(body.size()) + "\r\n";
        finalResponse += "Connection: close\r\n\r\n";
        finalResponse += body;
    }
    else
    {
        finalResponse = responseError(404, " Not Found", ConfigPars);
    }
    return finalResponse;
}

void    Response::moveToResponse(int &client_fd, Request	&req, std::vector<ConfigNode> ConfigPars)
{
    // Response obj(req, _client);

    if (method == "GET" || method == "POST")
    {
        getResponse(req, ConfigPars);
    }
    else if (method == "DELETE")
    {
        // if method not allowed return 405 Method Not Allowed

       deleteResponse(ConfigPars);
    }
    else
        responseError(501, " Method not implemented", ConfigPars);
    // response_map[client_fd] = obj.getFinalResponse();
}
