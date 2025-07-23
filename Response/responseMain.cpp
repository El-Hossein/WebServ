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
    hasPendingCgi = false;
}

Response::~Response(){

}


bool    Response::gethasPendingCgi()
{
    return hasPendingCgi;
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

void    Response::sethasPendingCgi(bool pendingcgi)
{
    hasPendingCgi = pendingcgi;
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

std::string readFileToString(const std::string& path)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file)  // if cant open need to give the ones are saved
        return "";
    std::ostringstream contents;
    contents << file.rdbuf();
    file.close();
    return contents.str();
}

std::string handWritingError(const std::string& message, int statusCode)
{
    std::string _code = intToString(statusCode);

    std::string html = "<!DOCTYPE html><html lang=\"en\">";
    html += "<head><meta charset=\"UTF-8\">";
    html += "<title>Error ";
    html += _code + "</title>";
    html += "<style>";
    html += "body { background-color: #f8f9fa; font-family: Arial, sans-serif; text-align: center; padding-top: 100px; }";
    html += "h1 { font-size: 48px; color: #dc3545; }";
    html += "p { font-size: 20px; color: #6c757d; }";
    html += "</style></head>";
    html += "<body>";
    html += "<h1>Error " + _code + "</h1>";
    html += "<p>" + message + "</p>";
    html += "</body></html>";

    return html;
}

void     Response::responseError(int statusCode, const std::string& message, std::vector<ConfigNode> ConfigPars)
{
    std::string body;
    (void)ConfigPars;

    switch (statusCode)
    {
        //need to read the ones provided in config file
        case 403: body = readFileToString("/Users/i61mail/Desktop/WebServ/Response/errorPages/403.html"); break;
        case 404: body = readFileToString("/Users/i61mail/Desktop/WebServ/Response/errorPages/404.html"); break;
        case 500: body = readFileToString("/Users/i61mail/Desktop/WebServ/Response/errorPages/500.html"); break;
        case 501: body = readFileToString("/Users/i61mail/Desktop/WebServ/Response/errorPages/501.html"); break;
        case 400: body = readFileToString("/Users/i61mail/Desktop/WebServ/Response/errorPages/400.html"); break;
        case 413: body = readFileToString("/Users/i61mail/Desktop/WebServ/Response/errorPages/413.html"); break;
        case 414: body = readFileToString("/Users/i61mail/Desktop/WebServ/Response/errorPages/414.html"); break;
    }

    if (body.empty())
        body = handWritingError(message, statusCode);

    staticFileBody = body;
    staticFilePos = 0;
    usingStaticFile = true;

    headers = "HTTP/1.1 " + intToString(statusCode);
    switch (statusCode)
    {
        case 403: headers += " Forbidden"; break;
        case 404: headers += " Not Found"; break;
        case 500: headers += " Internal Server Error"; break;
        case 501: headers += " Method not implemented"; break;
        case 400: headers += " Bad Request"; break;
        case 413: headers += " Content Too Large"; break;
        case 414: headers += " URI Too Long"; break;
    }
    headers += "\r\n";
    headers += "Content-Type: text/html\r\n";
    headers += "Content-Length: " + intToString(staticFileBody.size()) + "\r\n";
    headers += "Connection: close\r\n\r\n";

    headerSent = 0;
}

bool Response::getNextChunk(size_t chunkSize)
{
    chunk.clear();

    // static file headers
    if (headerSent < headers.size())
    {
        size_t left = headers.size() - headerSent;
        size_t sendNow = std::min(chunkSize, left);
        chunk = headers.substr(headerSent, sendNow);
        headerSent += sendNow;
        return true;
    }

    // cgi headers
    if (_cgi.getCgiHeaderSent() < _cgi.getCgiHeader().size())
    {
        size_t left = _cgi.getCgiHeader().size() - _cgi.getCgiHeaderSent();
        size_t sendNow = std::min(chunkSize, left);
        chunk = _cgi.getCgiHeader().substr(_cgi.getCgiHeaderSent(), sendNow);
        _cgi.setCgiHeaderSent(_cgi.getCgiHeaderSent() + sendNow);
        return true;
    }

    // error cgi
    if (_cgi.getUsingStatCgiFile())
    {
        if (_cgi.getStatCgiFilePos() < _cgi.getStatCgiFileBody().size())
        {
            size_t left = _cgi.getStatCgiFileBody().size() - _cgi.getStatCgiFilePos();
            size_t sendNow = std::min(chunkSize, left);
            chunk = _cgi.getStatCgiFileBody().substr(_cgi.getStatCgiFilePos(), sendNow);
            staticFilePos += sendNow;
            _cgi.setStatCgiFilePos(_cgi.getStatCgiFilePos() + sendNow);
            return _cgi.getStatCgiFilePos() < _cgi.getStatCgiFileBody().size();
        }
        else
        {
            _cgi.setUsingStatCgiFile(false);
            // return false;
        }
    }
    

    //error static file
    if (usingStaticFile)
    {
        if (staticFilePos < staticFileBody.size())
        {
            size_t left = staticFileBody.size() - staticFilePos;
            size_t sendNow = std::min(chunkSize, left);
            chunk = staticFileBody.substr(staticFilePos, sendNow);
            staticFilePos += sendNow;
            return staticFilePos < staticFileBody.size();
        }
        else
        {
            usingStaticFile = false;
            // return false;
        }
    }

    // body cgi
    if (_cgi.getUsingCgi())
    {
        std::ifstream& f = _cgi.getFile();

        if (_cgi.getFilePos() == 0)
        {
            std::string line;
            while (std::getline(f, line))
            {
                if (!line.empty() && line[line.size() - 1] == '\r')
                    line.erase(line.size() - 1);
                if (line.empty()) 
                    break;
            }
            _cgi.setFilePos(f.tellg());
        }

        f.seekg(_cgi.getFilePos());

        char *buffer = new char[chunkSize];
        f.read(buffer, chunkSize);
        int bytesRead = f.gcount();

        if (bytesRead > 0)
        {
            chunk.assign(buffer, bytesRead);
            _cgi.setFilePos(_cgi.getFilePos() + bytesRead);

            delete [] buffer;
            if (_cgi.getFilePos() >= _cgi.getFileSize())
                f.close();

            return true;
        }
        delete [] buffer;
        f.close();
    }

    // body static file
    if (file.is_open())
    {

        char *buffer = new char[chunkSize];
        file.read(buffer, chunkSize);
        int bytesRead = file.gcount();

        if (bytesRead > 0)
        {
            chunk.assign(buffer, bytesRead);
            filePos += bytesRead;
            delete [] buffer;
            if (filePos >= fileSize)
                file.close();

            return true;
        }

        delete [] buffer;
        file.close();
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
    staticFileBody = generateListingDir();
    staticFilePos = 0;
    usingStaticFile = true;

    headers = "HTTP/1.1 200 OK\r\n";
    headers += "Content-Length: " + intToString(staticFileBody.size()) + "\r\n";
    headers += "Content-Type: text/html\r\n";
    // need to check connection from request
    headers += "Connection: close\r\n\r\n";

    headerSent = 0;
    return true;
}

void Response::servListingDiren(std::vector<ConfigNode> ConfigPars, Request	&req)
{
    autoIndexOn = getInfoConfig(ConfigPars, "autoindex", "NULL", 0);
    index = getInfoConfig(ConfigPars, "index", "NULL", 0);

    if (!index.empty())
    {
        htmlFound = uri;
        if (uri.back() != '/')
            htmlFound += "/";
        htmlFound += index;
        int code = prepareFileResponse(htmlFound, "Content-Type: text/html\r\n", req);
        if (code == 0)
            return ;
        else if (code == 403)
        {
            responseError(403, " Forbidden", ConfigPars);
            return;
        }
        if (autoIndexOn == "on" && code == 404)
            generateAutoIndexOn();
        else
            responseError(403, " Forbidden", ConfigPars);
    }
    else if (autoIndexOn == "on")
        generateAutoIndexOn();
    else
        responseError(403, " Forbidden", ConfigPars);
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
    else
    {
        if (access(uri.c_str(), X_OK) != 0)
            return "Content-Type: text/plain\r\n";
        else
            return "Content-Type: application/octet-stream\r\n";
    }
    return "";

}

void    Response::getResponse( Request	&req, std::vector<ConfigNode> ConfigPars)
{
    struct stat st;


    _cgi.setcgiHeader("");
    if (IsCgiRequest(uri.c_str()))
    {
        _cgi.handleCgiRequest(req, ConfigPars);
        if (_cgi.getcgistatus() == CGI_RUNNING)
        {
            hasPendingCgi = true;
            return;
        }
        hasPendingCgi = false;
    }
    else if (method == "GET")
    {
        stat(uri.c_str(), &st);
        if (S_ISDIR(st.st_mode))
            servListingDiren(ConfigPars, req);
        else
        {
            int code = prepareFileResponse(uri.c_str(), checkContentType(), req);
            switch (code)
            {
                case 404: responseError(404, " Not Found", ConfigPars); return;
                case 403: responseError(403, " Forbidden", ConfigPars); return;
            }
            
        }
    }
    else if (method == "POST")
    {
        // POST here
    }
}

void Response::deleteResponse(std::vector<ConfigNode> ConfigPars, Request &req)
{
    struct stat st;

    if (stat(uri.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
    {
        responseError(403, " Permission Denied", ConfigPars);
    }
    else if (std::remove(uri.c_str()) == 0)
    {
        staticFileBody = deleteResponseSuccess("file succesefuly deleted!!");
        staticFilePos = 0;
        usingStaticFile = true;
        headers = "HTTP/1.1 200 OK\r\n";
        headers += checkContentType();
        headers += "Content-Length: " + intToString(staticFileBody.size()) + "\r\n";
        //need to check for connection
        headers += "Connection: close\r\n\r\n";

        headerSent = 0;
    }
    else
    {
        responseError(404, " Not Found", ConfigPars);
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

    // need to check for Connection
    headers += "Connection: Keep-Alive\r\n";
    headers += "\r\n";

    headerSent = 0;
    filePos = 0;

    return 0;
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
       deleteResponse(ConfigPars, req);
    }
    else
        responseError(501, " Method not implemented", ConfigPars);
}

bool Response::checkPendingCgi(std::vector<ConfigNode> ConfigPars) 
{
    if (!hasPendingCgi)
        return false;

    int status;
    pid_t childPid = _cgi.getpid_1();
    int result = waitpid(childPid, &status, WNOHANG);

    if (result > 0) // Process finished
    {
        struct kevent kev;
        EV_SET(&kev, childPid, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
        kevent(globalKq, &kev, 1, NULL, 0, NULL);
        
        if (WIFEXITED(status))
        {
            int exitCode = WEXITSTATUS(status);
            if (exitCode == 0)
            {
                _cgi.parseOutput();
                _cgi.formatHttpResponse(_cgi.getoutfile());
                _cgi.setcgistatus(CGI_COMPLETED);
            }
            else
            {
                _cgi.responseErrorcgi(500, " Internal Server Error", ConfigPars);
                _cgi.setcgistatus(CGI_ERROR);
            }
        }
        else if (WIFSIGNALED(status))
        {
            _cgi.responseErrorcgi(500, " Internal Server Error", ConfigPars);
            _cgi.setcgistatus(CGI_ERROR);
        }
        else
        {
            _cgi.responseErrorcgi(500, " Internal Server Error", ConfigPars);
            _cgi.setcgistatus(CGI_ERROR);
        }

        if (!_cgi.getinfile().empty())
            unlink(_cgi.getinfile().c_str());
        if (!_cgi.getoutfile().empty())   
            unlink(_cgi.getoutfile().c_str());

        hasPendingCgi = false;
        return true;
    }
    else if (result == -1)
    {
        struct kevent kev;
        EV_SET(&kev, childPid, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
        kevent(globalKq, &kev, 1, NULL, 0, NULL);
        
        _cgi.responseErrorcgi(500, " Internal Server Error", ConfigPars);
        _cgi.setcgistatus(CGI_ERROR);
        hasPendingCgi = false;
        
        if (!_cgi.getinfile().empty())
            unlink(_cgi.getinfile().c_str());
        if (!_cgi.getoutfile().empty())   
            unlink(_cgi.getoutfile().c_str());
        
        return true;
    }

    return false;
}