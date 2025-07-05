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

std::string getInfoConfig(std::vector<ConfigNode> ConfigPars, std::string what)
{
    ConfigNode a = ConfigNode::GetServer(ConfigPars, "myserver1.com");

    std::vector<std::string> autoIndex = a.getValuesForKey(a, what, "NULL");
    if (autoIndex.empty())
    {
        return "";
    }
    return autoIndex[0];
}

void    servListingDiren(std::string &uri, std::string &finalResponse, std::string &pathRequested, std::vector<ConfigNode> ConfigPars)
{
    std::string autoIndexOn = getInfoConfig(ConfigPars, "autoindex"); // i need to know what should i do if autoindex isnt there
    std::string index = getInfoConfig(ConfigPars, "index");

    if (!index.empty())
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
    }
    else if (autoIndexOn == "on")
    {
        std::string body = generateListingDir(uri, pathRequested);

        finalResponse = "HTTP/1.1 200 OK\r\n";
        finalResponse += "Content-Length: " + intToString(body.size()) + "\r\n";
        finalResponse += "Content-Type: text/html\r\n";
        finalResponse += "Connection: close\r\n\r\n";
        finalResponse += body;
    }
    else
    {
        finalResponse = responseError(403, " Forbidden", ConfigPars);
    }
}

std::string getResponse(std::string finalResponse, std::string uri, Request	&req, std::vector<ConfigNode> ConfigPars)
{
    struct stat stList;
    std::string pathRequested = req.GetHeaderValue("path");

    if (stat(uri.c_str(), &stList) == 0 && S_ISDIR(stList.st_mode))
    {
        servListingDiren(uri, finalResponse, pathRequested, ConfigPars);
    }
    else if (IsCgiRequest(uri.c_str()))
    {
        finalResponse = handleCgiRequest(req, ConfigPars);
    }
    else
    {
        // i need to check the errors thrown for the file before open it
        std::ifstream inFile(uri.c_str(), std::ios::binary);
        if (inFile)
        {
            std::ostringstream outStringFIle;
            outStringFIle << inFile.rdbuf(); // so i can read the whole file
            inFile.close();
            std::string fileBody = outStringFIle.str();
            finalResponse = "HTTP/1.1 200 OK\r\n";
            finalResponse += "Content-Length: ";
            finalResponse += intToString(fileBody.size()) + "\r\n";
            finalResponse += "Connection: close\r\n";
            //need to check content type
            finalResponse += "Content-Type: text/html\r\n";
            finalResponse += "\r\n";
            finalResponse += fileBody;
        }
        else
        {
            // and need to display given error page
            finalResponse = responseError(404, " Not Found", ConfigPars); // just a test i have to catsh error codes ziad throw
        }
    }
    return finalResponse;
}

void    moveToResponse(int &client_fd, std::map<int, std::string>& response_map, Request	&req, std::vector<ConfigNode> ConfigPars)
{
    std::string uri = req.GetFullPath();
    std::string finalResponse;
    std::string method = req.GetHeaderValue("method");
    struct stat st;

    if (method == "GET") // i need to check for allowed methods
    {
        //if method not allowed return 405 Method Not Allowed
        finalResponse = getResponse(finalResponse, uri, req, ConfigPars);
    }
    else if (method == "DELETE") // i need to check for allowed methods
    {
        // if method not allowed return 405 Method Not Allowed
        // i need to check the errors thrown for the file before open it
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
    }

    else
    {
        finalResponse = responseError(501, " Method not implemented", ConfigPars);
    }
    // std::cout << finalResponse << std::endl;
    response_map[client_fd] = finalResponse;
}
