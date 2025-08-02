#include "responseHeader.hpp"

Response::Response(){
    
}

Response::Response(Request	&req, int _clientFd)
{
    clientFd = _clientFd;
    filePos = 0;
    fileSize = 0;
    headerSent = 0;
    staticFilePos= 0;
    usingStaticFile = false;
    bytesSent = 0;
}

Response::~Response(){

}

std::string Response::getChunk()
{
    return chunk;
}

bool Response::getHasMore()
{
    return hasMore;
}

void    Response::setHasMore(bool _hasmore)
{
    hasMore = _hasmore;
}

ssize_t  Response::getBytesSent()
{
    return bytesSent;
}

void  Response::setBytesSent(ssize_t _bytessent)
{
    bytesSent = _bytessent;
}

ssize_t  Response::getBytesWritten()
{
    return bytesWritten;

}

void  Response::setBytesWritten(ssize_t _byteswritten)
{
    bytesWritten = _byteswritten;
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

std::string Response::postResponseSuccess(const std::string& message)
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

    std::string html = "<!DOCTYPE html>\n";
    html += "<html>\n<head>\n<title>Index of " + uri + "</title>\n";
    html += "<style>\n"
            "body { background-color: #f0f4f8; font-family: Arial, sans-serif; color: #333; padding: 40px; }\n"
            "h1 { color: #2c3e50; }\n"
            "ul { list-style-type: none; padding-left: 0; }\n"
            "li { margin: 10px 0; }\n"
            "a { text-decoration: none; color: #3498db; font-weight: bold; }\n"
            "a:hover { text-decoration: underline; color: #1abc9c; }\n"
            ".container { max-width: 800px; margin: auto; background-color: #fff; padding: 30px; box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1); border-radius: 8px; }\n"
            "</style>\n"
            "</head>\n<body>\n<div class=\"container\">\n"
            "<h1>Index of " + uri + "</h1>\n<ul>\n";

    struct dirent *dir;
    while ((dir = readdir(dirCheck)) != NULL)
    {
        std::string name = pathRequested + (pathRequested[pathRequested.length() - 1] == '/' ? "" : "/") + dir->d_name;
        html += "<li><a href=\"" + name + "\">" + dir->d_name + "</a></li>\n";
    }
    closedir(dirCheck);
    html += "</ul>\n</div>\n</body>\n</html>\n";
    return html;
}

std::string getInfoConfig(std::vector<ConfigNode> ConfigPars, std::string what, std::string location, Request &req)
{
    ConfigNode a = req.GetRightServer();

    std::vector<std::string> temp = a.getValuesForKey(a, what, location);
    if (!temp.empty())
        return temp[0];
	return "";
}

std::vector<std::string> getInfoConfigMultiple(std::vector<ConfigNode> ConfigPars, std::string what, std::string location, Request &req)
{
    ConfigNode a = req.GetRightServer();

    return a.getValuesForKey(a, what, location);
}

bool Response::generateAutoIndexOn(Request &req)
{
    staticFileBody = generateListingDir();
    staticFilePos = 0;
    usingStaticFile = true;

    headers = "HTTP/1.1 200 OK\r\n";
    headers += "Content-Length: " + intToString(staticFileBody.size()) + "\r\n";
    headers += "Content-Type: text/html\r\n";
    if (req.GetHeaderValue("connection") == "keep-alive")
    {
        headers += "Connection: keep-alive\r\n\r\n";
        _cgi.setCheckConnection(keepAlive);
    }
    else
    {
        headers += "Connection: close\r\n\r\n";
        _cgi.setCheckConnection(_close);
    }

    headerSent = 0;
    return true;
}

std::string Response::checkContentType(int index)
{
    std::string exte;
    if (index == 1)
        exte = htmlFound;
    else
        exte = uri;
    size_t dotPos = exte.find(".");
    if (dotPos != std::string::npos)
    {
        std::string extension = exte.substr(dotPos);
        if (extension.compare(".png") == 0)
            return "Content-Type: image/png\r\n";
        else if (extension.compare(".jpeg") == 0)
            return "Content-Type: image/jpeg\r\n";
        else if (extension.compare(".mp4") == 0)
            return "Content-Type: video/mp4\r\n";
        else if (extension.compare(".mpeg") == 0)
            return "Content-Type: audio/mpeg\r\n";
        else if (extension.compare(".pdf") == 0)
            return "Content-Type: application/pdf\r\n";
        else if (extension.compare(".zip") == 0)
            return "Content-Type: application/zip\r\n";
        else if (extension.compare(".svg") == 0)
            return "Content-Type: image/svg+xml\r\n";
        else if (extension.compare(".mp3") == 0)
            return "Content-Type: audio/mp3\r\n";
        else if (extension.compare(".vorbis") == 0)
            return "Content-Type: audio/vorbis\r\n";
        else if (extension.compare(".js") == 0)
            return "Content-Type: application/javascript\r\n";
        else if (extension.compare(".json") == 0)
            return "Content-Type: application/json\r\n";
        else if (extension.compare(".html") == 0)
            return "Content-Type: text/html\r\n";
        else
            return "Content-Type: text/plain\r\n";
    }
    if (access(exte.c_str(), X_OK) != 0)
        return "Content-Type: text/plain\r\n";
    else
        return "Content-Type: application/octet-stream\r\n";

}


int Response::checkLocation(Request &req, std::string meth, std::string directive, std::vector<ConfigNode> ConfigPars)
{
    std::string	 loc = req.GetRightServer().GetRightLocation(req.GetHeaderValue("path"));
    std::vector<std::string> allowed_methods = getInfoConfigMultiple(ConfigPars, directive, loc, req);
    if (std::find(allowed_methods.begin(), allowed_methods.end(), meth) == allowed_methods.end())
    {
        responseError(405, " Method Not Allowed", ConfigPars, req);
        return -1;
    }
    return 0;
}


void Response::deleteResponse(std::vector<ConfigNode> ConfigPars, Request &req)
{
    struct stat st;

    if (stat(uri.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
    {
        responseError(403, " Permission Denied", ConfigPars, req);
    }
    else if (std::remove(uri.c_str()) == 0)
    {
        staticFileBody = deleteResponseSuccess("file succesefuly deleted!!");
        staticFilePos = 0;
        usingStaticFile = true;
        headers = "HTTP/1.1 200 OK\r\n";
        headers += checkContentType(0);
        headers += "Content-Length: " + intToString(staticFileBody.size()) + "\r\n";
        if (req.GetHeaderValue("connection") == "keep-alive")
        {
            headers += "Connection: keep-alive\r\n\r\n";
            _cgi.setCheckConnection(keepAlive);
        }
        else
        {
            headers += "Connection: close\r\n\r\n";
            _cgi.setCheckConnection(_close);
        }
        headerSent = 0;
    }
    else
    {
        responseError(404, " Not Found", ConfigPars, req);
    }
}

int Response::prepareFileResponse(std::string filepath, std::string contentType, Request &req)
{
    struct stat fileStat;
    
    if (stat(filepath.c_str(), &fileStat) != 0)
        return 404;

    if (access(filepath.c_str(), R_OK) != 0)
        return 403;

    file.open(filepath.c_str(), std::ios::binary);
    if (!file.is_open())
        return 404;

    fileSize = fileStat.st_size;


    (void)req;

    headers = "HTTP/1.1 200 OK\r\n";
    headers += contentType;
    if ( contentType == "video/mp4\r\n" == 0)
        headers += "Accept-Ranges: bytes\r\n";
    headers += "Content-Length: " + intToString(fileSize) + "\r\n";
    if (req.GetHeaderValue("connection") == "keep-alive")
    {
        headers += "Connection: keep-alive\r\n";
        _cgi.setCheckConnection(keepAlive);
    }
    else
    {
        headers += "Connection: close\r\n";
        _cgi.setCheckConnection(_close);
    }
    headers += "\r\n";
    headerSent = 0;
    filePos = 0;

    return 0;
}

void Response::servListingDiren(std::vector<ConfigNode> ConfigPars, Request	&req)
{
	std::string	 loc = req.GetRightServer().GetRightLocation(req.GetHeaderValue("path")); // ngulih ystori hada f class 3ndo
    autoIndexOn = getInfoConfig(ConfigPars, "autoindex", loc, req);
    index = getInfoConfig(ConfigPars, "index", loc, req);

    if (!index.empty())
    {
        htmlFound = uri;
        if (uri.back() != '/')
            htmlFound += "/";
        htmlFound += index;

        struct stat st;
        if (stat(htmlFound.c_str(), &st) == 0)
        {
            if (S_ISDIR(st.st_mode))
            {
                if (autoIndexOn == "on")
                    generateAutoIndexOn(req);
                else
                    responseError(403, " Forbidden", ConfigPars, req);
                return ;
            }
            if (access(htmlFound.c_str(), R_OK) != 0)
            {
                if (autoIndexOn == "on")
                    generateAutoIndexOn(req);
                else
                    responseError(403, " Forbidden", ConfigPars, req);
                return ;
            }
            prepareFileResponse(htmlFound, checkContentType(1), req);
        }
        else if (autoIndexOn == "on")
            generateAutoIndexOn(req);
        else
            responseError(403, " Forbidden", ConfigPars, req);
    }
    else if (autoIndexOn == "on")
        generateAutoIndexOn(req);
    else
        responseError(403, " Forbidden", ConfigPars, req);
}


int Response::prepareRedirectResponse(std::vector<std::string> redirect, Request &req, std::vector<ConfigNode> ConfigPars)
{
    int statusCode = std::atoi(redirect[0].c_str());
    std::string redirectUrl = redirect[1];
    if (statusCode != 301 && statusCode != 302 && statusCode != 303 && statusCode != 302 && statusCode != 307 && statusCode != 308)
    {
        responseError(500, "Invalid redirect status code", ConfigPars, req);
        return -1;
    }

    if (redirectUrl.empty())
    {
        responseError(500, "Invalid redirect URL", ConfigPars, req);
        return -1;
    }
    staticFileBody.clear();
    staticFilePos = 0;
    usingStaticFile = true;
    filePos = 0;

    headers = "HTTP/1.1 " + intToString(statusCode);
    switch (statusCode)
    {
        case 301: headers += " Moved Permanently"; break;
        case 302: headers += " Found"; break;
        case 303: headers += " See Other"; break;
        case 304: headers += " Not Modified"; break;
        case 307: headers += " Temporary Redirect"; break;
        case 308: headers += " Permanent Redirect"; break;
    }
    headers += "\r\n";
    headers += "Location: " + redirectUrl + "\r\n";
    headers += "Content-Length: 0\r\n";
    
    if (req.GetHeaderValue("connection") == "keep-alive")
    {
        headers += "Connection: keep-alive\r\n";
        _cgi.setCheckConnection(keepAlive);
    }
    else
    {
        headers += "Connection: close\r\n";
        _cgi.setCheckConnection(_close);
    }

    headers += "\r\n";
    headerSent = 0;

    return 0;
}

void    Response::getResponse( Request	&req, std::vector<ConfigNode> ConfigPars)
{
    struct stat st;


    if (method == "GET")
    {
        if (checkLocation(req, "GET", "allow_methods", ConfigPars) == -1)
            return ;
        std::string	 loc = req.GetRightServer().GetRightLocation(req.GetHeaderValue("path"));
        std::vector<std::string> redirect = getInfoConfigMultiple(ConfigPars, "return", loc, req);
        if (redirect.size() != 0)
        {
            prepareRedirectResponse(redirect, req, ConfigPars);
            return ;
        }
        _cgi.setcgiHeader("");
        int checkCode = _cgi.IsCgiRequest(uri.c_str(), req, ConfigPars);
        if (checkCode == 1)
        {
            _cgi.handleCgiRequest(req, ConfigPars);
            if (_cgi.getcgistatus() == CGI_RUNNING)
            {
                _cgi.sethasPendingCgi(true);
                return;
            }
            _cgi.sethasPendingCgi(false);
            return ;
        }
        else if (checkCode == -1)
            return ;
        stat(uri.c_str(), &st);
        if (S_ISDIR(st.st_mode))
            servListingDiren(ConfigPars, req);
        else
        {
            int code = prepareFileResponse(uri.c_str(), checkContentType(0), req);
            switch (code)
            {
                case 404: responseError(404, " Not Found", ConfigPars, req); return;
                case 403: responseError(403, " Forbidden", ConfigPars, req); return;
            }
            
        }
    }
    else if (method == "POST")
    {
        if (checkLocation(req, "POST", "allow_methods", ConfigPars) == -1)
            return ;
        _cgi.setcgiHeader("");
        int checkCode = _cgi.IsCgiRequest(uri.c_str(), req, ConfigPars);
        if (checkCode == 1)
        {
            _cgi.handleCgiRequest(req, ConfigPars);
            if (_cgi.getcgistatus() == CGI_RUNNING)
            {
                _cgi.sethasPendingCgi(true);
                return;
            }
            _cgi.sethasPendingCgi(false);
            return ;
        }
        else if (checkCode == -1)
            return ;
        else
        {
            staticFileBody = postResponseSuccess("file succesefuly uploaded!");
            staticFilePos = 0;
            usingStaticFile = true;
            headers = "HTTP/1.1 201 Created\r\n";
            headers += "Content-Type: text/plain\r\n";
            headers += "Content-Length: " + intToString(staticFileBody.size()) + "\r\n";
            if (req.GetHeaderValue("connection") == "keep-alive")
            {
                headers += "Connection: keep-alive\r\n";
                _cgi.setCheckConnection(keepAlive);
            }
            else
            {
                headers += "Connection: close\r\n";
                _cgi.setCheckConnection(_close);
            }
            headers += "\r\n";
            headerSent = 0;
        }
    }
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
        if (checkLocation(req, "DELETE", "allow_methods", ConfigPars) == -1)
            return ;
        _cgi.setcgiHeader("");
        int checkCode = _cgi.IsCgiRequest(uri.c_str(), req, ConfigPars);
        if (checkCode == 1)
        {
            _cgi.handleCgiRequest(req, ConfigPars);
            if (_cgi.getcgistatus() == CGI_RUNNING)
            {
                _cgi.sethasPendingCgi(true);
                // hasPendingCgi = true;
                return;
            }
            _cgi.sethasPendingCgi(false);
            // hasPendingCgi = false;
            return ;
        }
        else if (checkCode == -1)
            return ;
        deleteResponse(ConfigPars, req);
    }
    else
    {
        responseError(501, " Method not implemented", ConfigPars, req);
    }
}

