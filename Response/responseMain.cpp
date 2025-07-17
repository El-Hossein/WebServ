#include "responseHeader.hpp"

Response::Response(){
    
}

Response::Response(Request	&req, int _clientFd)
{
    clientFd = _clientFd;
    filePos = 0;
    fileSize = 0;
    headerSent = 0;
    autoIndexPos = 0;
    usingAutoIndex = false;
}

Response::~Response(){
    
}

size_t  Response::getHeaderSent()
{
    return headerSent;

}

void  Response::setHeaderSent(size_t _aa)
{
    headerSent = _aa;
}

int Response::getClientFd()
{
    return clientFd;
}

void    Response::setClientFd(int _clientFd)
{
    clientFd = _clientFd;
}

bool Response::getNextChunk(std::string &out, size_t chunkSize)
{
    out.clear();

    // Send remaining header first
    if (headerSent < headers.size())
    {
        size_t left = headers.size() - headerSent;
        size_t sendNow = std::min(chunkSize, left);
        out = headers.substr(headerSent, sendNow);
        headerSent += sendNow;
        return true;
    }

    // Send from autoIndex buffer if active
    if (usingAutoIndex)
    {
        if (autoIndexPos < autoIndexBody.size())
        {
            size_t left = autoIndexBody.size() - autoIndexPos;
            size_t sendNow = std::min(chunkSize, left);
            out = autoIndexBody.substr(autoIndexPos, sendNow);
            autoIndexPos += sendNow;
            return autoIndexPos < autoIndexBody.size(); // true if more left
        }
        else
        {
            usingAutoIndex = false; // done
            return false;
        }
    }

    // Send file content (existing behavior)
    if (file.is_open() && filePos < fileSize)
    {
        std::vector<char> buffer(chunkSize);
        file.read(buffer.data(), chunkSize);
        std::streamsize bytesRead = file.gcount();

        if (bytesRead > 0)
        {
            out.assign(buffer.data(), bytesRead);
            filePos += bytesRead;

            if (filePos >= fileSize)
                file.close();

            return filePos < fileSize;
        }
        else
        {
            file.close();
        }
    }

    return false;
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

bool Response::generateAutoIndexOn()
{
    autoIndexBody = generateListingDir(); // Save full body to string
    autoIndexPos = 0;
    usingAutoIndex = true;

    headers = "HTTP/1.1 200 OK\r\n";
    headers += "Content-Length: " + std::to_string(autoIndexBody.size()) + "\r\n";
    headers += "Content-Type: text/html\r\n";
    headers += "Connection: close\r\n\r\n";

    headerSent = 0;
    return true;
}

void Response::servListingDiren(std::vector<ConfigNode> ConfigPars)
{
    autoIndexOn = getInfoConfig(ConfigPars, "autoindex", "NULL", 0);
    index = getInfoConfig(ConfigPars, "index", "NULL", 0);

    std::cout << index << std::endl;
    // Check if we need to append slash
    if (!index.empty())
    {
        htmlFound = uri;
        if (!uri.empty() && uri.back() != '/')
            htmlFound += "/";
        htmlFound += index;
        std::cout << htmlFound << std::endl;

        // Try to serve index file
        if (prepareFileResponse(htmlFound, "Content-Type: text/html\r\n"))
            return;  // success — ready for getNextChunk()

        // If index file not found and autoindex is on
        if (autoIndexOn == "on")
            generateAutoIndexOn();  // sets up headers + autoIndexBody
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
            if (prepareFileResponse(uri.c_str(), checkContentType()) == false)
                return finalResponse = responseError(404, " Not Found", ConfigPars);
            
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

bool Response::prepareFileResponse(const std::string& filepath, const std::string& contentType)
{
    // Open the file in binary mode
    file.open(filepath.c_str(), std::ios::binary);
    if (!file.is_open())
        return false;  // file not found or can't open

    // Get file size
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Build HTTP response headers
    headers = "HTTP/1.1 200 OK\r\n";
    headers += "Content-Length: " + std::to_string(fileSize) + "\r\n";
    headers += contentType;
    headers += "Connection: close\r\n";
    headers += "\r\n";

    // Reset position trackers for sending
    headerSent = 0;
    filePos = 0;

    return true;
}

void    Response::moveToResponse(int &client_fd, Request	&req, std::vector<ConfigNode> ConfigPars)
{

    uri = req.GetFullPath();
    method = req.GetHeaderValue("method");
    pathRequested = req.GetHeaderValue("path");
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
