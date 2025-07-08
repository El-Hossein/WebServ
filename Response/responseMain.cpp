#include "responseHeader.hpp"

std::string deleteResponseSuccess(const std::string& message)
{
    std::string html = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Error</title></head>";
    html += "<body><h1>";
    html += message;
    html += "</h1></body></html>";

    return html;
}

std::string generateListingDir(const std::string& uri, std::string &pathRequested)
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

std::string generateAutoIndexOn(std::string &finalResponse, std::string &uri, std::string &pathRequested)
{
    std::string body = generateListingDir(uri, pathRequested);

    finalResponse = "HTTP/1.1 200 OK\r\n";
    finalResponse += "Content-Length: " + intToString(body.size()) + "\r\n";
    finalResponse += "Content-Type: text/html\r\n";
    finalResponse += "Connection: close\r\n\r\n";
    finalResponse += body;
    return finalResponse;

}

void    servListingDiren(std::string &uri, std::string &finalResponse, std::string &pathRequested, std::vector<ConfigNode> ConfigPars)
{
    std::string autoIndexOn = getInfoConfig(ConfigPars, "autoindex", "NULL", 0); // need to pass which location and server
    std::string index = getInfoConfig(ConfigPars, "index", "NULL", 0); // need to pass which location and server

    if (index.empty() == 0)
    {
        std::string htmlFound = uri + "/" + index; // i need to check if slash already there or not
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
            generateAutoIndexOn(finalResponse, uri, pathRequested);
        else
            finalResponse = responseError(403, " Forbidden", ConfigPars);
    }
    else if (autoIndexOn == "on")
        generateAutoIndexOn(finalResponse, uri, pathRequested);
    else
        finalResponse = responseError(403, " Forbidden", ConfigPars);
}


std::string getResponse(std::string finalResponse, std::string uri, Request	&req, std::vector<ConfigNode> ConfigPars, std::string method)
{
    struct stat st;
    std::string pathRequested = req.GetHeaderValue("path");

    if (IsCgiRequest(uri.c_str()))
        finalResponse = handleCgiRequest(req, ConfigPars);
    else if (method == "GET")
    {
        if (stat(uri.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
            servListingDiren(uri, finalResponse, pathRequested, ConfigPars);
        else
        {
            if (access(pathRequested.c_str(), R_OK | W_OK) == -1)
            {
                finalResponse = responseError(403, " Forbidden", ConfigPars);
                return finalResponse;
            }
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
                finalResponse += "Connection: close\r\n";
                finalResponse += "Content-Type: text/html\r\n";
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
    }
    return finalResponse;
}

std::string deleteResponse(std::string &finalResponse, std::string uri, std::vector<ConfigNode> ConfigPars)
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

void    moveToResponse(int &client_fd, std::map<int, std::string>& response_map, Request	&req, std::vector<ConfigNode> ConfigPars)
{
    std::string uri = req.GetFullPath();
    std::string finalResponse;
    std::string method = req.GetHeaderValue("method");
    

    if (method == "GET" || method == "POST")
    {
        finalResponse = getResponse(finalResponse, uri, req, ConfigPars, method);
    }
    else if (method == "DELETE") 
    {
        // if method not allowed return 405 Method Not Allowed
       finalResponse = deleteResponse(finalResponse, uri, ConfigPars);
    }
    else
        finalResponse = responseError(501, " Method not implemented", ConfigPars);
    response_map[client_fd] = finalResponse;
}
