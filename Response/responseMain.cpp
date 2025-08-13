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
    _cgi.sethasPendingCgi(false);
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
    std::string html = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">";
    html += "<title>Success</title>";
    html += "<style>";
    html += "body { background: linear-gradient(to bottom, #7C0A02, #5A0000, #7C0A02); "
            "font-family: 'Segoe UI', sans-serif; color: #f8e1e1; margin: 0; padding: 40px; "
            "display: flex; justify-content: center; align-items: center; height: 100vh; }\n";

    html += ".success-container { max-width: 600px; background: rgba(0,0,0,0.25); padding: 30px; "
            "border-radius: 12px; box-shadow: 0 8px 32px rgba(0,0,0,0.5); text-align: center; "
            "backdrop-filter: blur(10px); animation: fadeIn 0.6s ease-in-out; }\n";

    html += "h1 { font-size: 48px; color: #b3ffb3; margin-bottom: 20px; "
            "text-shadow: 0 0 10px rgba(179, 255, 179, 0.8); animation: slideUp 0.5s ease-in-out; }\n";

    html += "@keyframes fadeIn { from { opacity: 0; } to { opacity: 1; } }\n";
    html += "@keyframes slideUp { from { transform: translateY(20px); opacity: 0; } "
            "to { transform: translateY(0); opacity: 1; } }\n";
    html += "</style></head><body>";
    html += "<div class=\"success-container\">";
    html += "<h1>✅ " + message + "</h1>";
    html += "</div></body></html>";

    return html;
}

std::string Response::postResponseSuccess(const std::string& message)
{
    std::string html = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">";
    html += "<title>Success</title>";
    html += "<style>";
    html += "body { background: linear-gradient(to bottom, #7C0A02, #5A0000, #7C0A02); "
            "font-family: 'Segoe UI', sans-serif; color: #f8e1e1; margin: 0; padding: 40px; "
            "display: flex; justify-content: center; align-items: center; height: 100vh; }\n";

    html += ".success-container { max-width: 600px; background: rgba(0,0,0,0.25); padding: 30px; "
            "border-radius: 12px; box-shadow: 0 8px 32px rgba(0,0,0,0.5); text-align: center; "
            "backdrop-filter: blur(10px); animation: fadeIn 0.6s ease-in-out; }\n";

    html += "h1 { font-size: 48px; color: #b3ffb3; margin-bottom: 20px; "
            "text-shadow: 0 0 10px rgba(179, 255, 179, 0.8); animation: slideUp 0.5s ease-in-out; }\n";

    html += "@keyframes fadeIn { from { opacity: 0; } to { opacity: 1; } }\n";
    html += "@keyframes slideUp { from { transform: translateY(20px); opacity: 0; } "
            "to { transform: translateY(0); opacity: 1; } }\n";
    html += "</style></head><body>";
    html += "<div class=\"success-container\">";
    html += "<h1>📤 " + message + "</h1>";
    html += "</div></body></html>";

    return html;
}

std::string Response::generateListingDir()
{
    DIR *dirCheck = opendir(uri.c_str());
    if (!dirCheck)
        return "";

    std::string html;
    html += "<!DOCTYPE html>\n<html>\n<head>\n";
    html += "<meta charset=\"UTF-8\">\n<title>Index of " + uri + "</title>\n";
    html += "<style>\n"
            "body { background: linear-gradient(to bottom, #0b3d2e, #14532d, #0b3d2e); font-family: 'Segoe UI', sans-serif; color: #e0e0e0; padding: 40px; margin: 0; }\n"
            ".container { max-width: 950px; margin: auto; background: rgba(0,0,0,0.25); padding: 30px; border-radius: 12px; box-shadow: 0 8px 32px rgba(0,0,0,0.5); backdrop-filter: blur(10px); animation: fadeIn 0.6s ease-in-out; }\n"
            "h1 { color: #a5d6a7; margin-bottom: 20px; text-shadow: 0 0 10px rgba(165, 214, 167, 0.8); animation: slideDown 0.5s ease-in-out; }\n"
            "ul { list-style-type: none; padding: 0; margin: 0; }\n"
            "li { margin: 10px 0; padding: 12px; border-radius: 6px; background: rgba(255,255,255,0.05); display: flex; align-items: center; transform: translateX(-10px); opacity: 0; animation: slideIn 0.4s forwards; }\n"
            "li:hover { background: rgba(76,175,80,0.25); transform: scale(1.02); transition: all 0.3s ease; }\n"
            "a { text-decoration: none; color: #80cbc4; font-weight: bold; flex-grow: 1; transition: color 0.3s ease; }\n"
            "a:hover { color: #b2dfdb; }\n"
            ".icon { margin-right: 12px; font-size: 20px; }\n"

            /* Upload section */
            ".upload-section { margin-top: 30px; padding: 25px; background: rgba(0,0,0,0.25); border-radius: 12px; box-shadow: 0 4px 20px rgba(0,0,0,0.4); text-align: center; color: #c8e6c9; animation: fadeIn 1s ease-in-out; }\n"
            ".upload-section h2 { margin-bottom: 15px; font-size: 1.3rem; }\n"
            ".file-input { display: inline-block; padding: 10px 20px; background: #2e7d32; color: #fff; border-radius: 6px; cursor: pointer; transition: background 0.3s ease; }\n"
            ".file-input:hover { background: #388e3c; }\n"
            "input[type=file] { display: none; }\n"
            "input[type=submit] { margin-top: 15px; padding: 10px 20px; background: #43a047; border: none; border-radius: 6px; color: #fff; font-weight: bold; cursor: pointer; transition: background 0.3s ease; }\n"
            "input[type=submit]:hover { background: #4caf50; }\n"

            /* Animations */
            "@keyframes fadeIn { from { opacity: 0; } to { opacity: 1; } }\n"
            "@keyframes slideIn { to { opacity: 1; transform: translateX(0); } }\n"
            "@keyframes slideDown { from { transform: translateY(-20px); opacity: 0; } to { transform: translateY(0); opacity: 1; } }\n"
            "</style>\n</head>\n<body>\n";

    html += "<div class=\"container\">\n";
    html += "<h1>📂 Index of " + uri + "</h1>\n<ul>\n";

    struct dirent *dir;
    while ((dir = readdir(dirCheck)) != NULL)
    {
        std::string name = dir->d_name;
        if (name[0] == '.' && name != "." && name != "..")
            continue;

        std::string path = pathRequested + (pathRequested[pathRequested.length() - 1] == '/' ? "" : "/") + name;
        std::string icon = (dir->d_type == DT_DIR) ? "📁" : "📄";
        html += "<li><span class=\"icon\">" + icon + "</span><a href=\"" + path + "\">" + name + "</a></li>\n";
    }
    closedir(dirCheck);

    html += "</ul>\n";

    // Upload section
    html += "<div class=\"upload-section\">\n";
    html += "<h2>⬆️ Upload a File</h2>\n";
    html += "<form action=\"" + uri + "\" method=\"post\" enctype=\"multipart/form-data\">\n";
    html += "<label class=\"file-input\">Choose File<input type=\"file\" name=\"file\"></label><br>\n";
    html += "<input type=\"submit\" value=\"Upload\">\n";
    html += "</form>\n</div>\n";

    html += "</div>\n</body>\n</html>\n";

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

bool Response::generateAutoIndexOn(std::vector<ConfigNode> ConfigPars, Request &req)
{
    staticFileBody = generateListingDir();
    if (staticFileBody.empty())
    {
        responseError(403, " Forbidden", ConfigPars, req);
        return false;
    }
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
    size_t dotPos = exte.find_last_of(".");
    if (dotPos != std::string::npos)
    {
        std::string extension = exte.substr(dotPos);
        if (extension == ".png")   return "Content-Type: image/png\r\n";
        if (extension == ".jpg" || extension == ".jpeg") return "Content-Type: image/jpeg\r\n";
        if (extension == ".gif")   return "Content-Type: image/gif\r\n";
        if (extension == ".bmp")   return "Content-Type: image/bmp\r\n";
        if (extension == ".webp")  return "Content-Type: image/webp\r\n";
        if (extension == ".svg")   return "Content-Type: image/svg+xml\r\n";
        if (extension == ".mp4")   return "Content-Type: video/mp4\r\n";
        if (extension == ".webm")  return "Content-Type: video/webm\r\n";
        if (extension == ".ogv")   return "Content-Type: video/ogg\r\n";
        if (extension == ".avi")   return "Content-Type: video/x-msvideo\r\n";
        if (extension == ".mov")   return "Content-Type: video/quicktime\r\n";
        if (extension == ".mkv")   return "Content-Type: video/x-matroska\r\n";
        if (extension == ".mp3")   return "Content-Type: audio/mpeg\r\n";
        if (extension == ".wav")   return "Content-Type: audio/wav\r\n";
        if (extension == ".ogg")   return "Content-Type: audio/ogg\r\n";
        if (extension == ".flac")  return "Content-Type: audio/flac\r\n";
        if (extension == ".aac")   return "Content-Type: audio/aac\r\n";
        if (extension == ".pdf")   return "Content-Type: application/pdf\r\n";
        if (extension == ".zip")   return "Content-Type: application/zip\r\n";
        if (extension == ".tar")   return "Content-Type: application/x-tar\r\n";
        if (extension == ".gz")    return "Content-Type: application/gzip\r\n";
        if (extension == ".bz2")   return "Content-Type: application/x-bzip2\r\n";
        if (extension == ".7z")    return "Content-Type: application/x-7z-compressed\r\n";
        if (extension == ".rar")   return "Content-Type: application/vnd.rar\r\n";
        if (extension == ".html" || extension == ".htm") return "Content-Type: text/html\r\n";
        if (extension == ".css")   return "Content-Type: text/css\r\n";
        if (extension == ".js")    return "Content-Type: application/javascript\r\n";
        if (extension == ".json")  return "Content-Type: application/json\r\n";
        if (extension == ".xml")   return "Content-Type: application/xml\r\n";
        if (extension == ".txt")   return "Content-Type: text/plain\r\n";
        if (extension == ".csv")   return "Content-Type: text/csv\r\n";
        if (extension == ".md")    return "Content-Type: text/markdown\r\n";
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
    if (contentType == "video/mp4\r\n" == 0)
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
                    generateAutoIndexOn(ConfigPars, req);
                else
                    responseError(403, " Forbidden", ConfigPars, req);
                return ;
            }
            if (access(htmlFound.c_str(), R_OK) != 0)
            {
                if (autoIndexOn == "on")
                    generateAutoIndexOn(ConfigPars, req);
                else
                    responseError(403, " Forbidden", ConfigPars, req);
                return ;
            }
            prepareFileResponse(htmlFound, checkContentType(1), req);
        }
        else if (autoIndexOn == "on")
            generateAutoIndexOn(ConfigPars, req);
        else
            responseError(403, " Forbidden", ConfigPars, req);
    }
    else if (autoIndexOn == "on")
        generateAutoIndexOn(ConfigPars, req);
    else
        responseError(403, " Forbidden", ConfigPars, req);
}


void    Response::nonRedirect(std::string redirectUrl, Request &req, std::vector<ConfigNode> ConfigPars, int statusCode)
{
    staticFileBody = redirectUrl;
    staticFilePos = 0;
    usingStaticFile = true;
    filePos = 0;

    headers = "HTTP/1.1 " + intToString(statusCode);
    switch (statusCode)
    {
        case 200: headers += " Ok"; break;
        case 201: headers += " Created"; break;
        case 204: headers += " No Content"; staticFileBody = ""; redirectUrl.clear(); break;
        case 400: headers += " Bad Request"; break;
        case 403: headers += " Forbidden"; break;
        case 404: headers += " Not Found"; break;
        case 500: headers += " Internal Server Error"; break;
    }
    headers += "\r\n";
    headers += "Content-Type: text/plain\r\n";
    headers += "Content-Length: " + intToString(redirectUrl.size()) + "\r\n";
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

int Response::prepareRedirectResponse(std::vector<std::string> redirect, Request &req, std::vector<ConfigNode> ConfigPars)
{
    int statusCode = std::atoi(redirect[0].c_str());
    std::string redirectUrl;
    if (redirect.size() > 1)
        redirectUrl = redirect[1];
    else
        redirectUrl = "";
    if (statusCode < 301 || statusCode > 599)
    {
        nonRedirect(redirectUrl, req, ConfigPars, statusCode);
        return 0;
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
    headers += "Content-Type: text/plain\r\n";
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

void    Response::getResponse( Request	&req, std::vector<ConfigNode> ConfigPars, int e)
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
            switch (e)
            {
                case 200: headers = "HTTP/1.1 200 OK\r\n";
                case 201: headers = "HTTP/1.1 201 Created\r\n";
            }
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

void    Response::moveToResponse(int &client_fd, Request	&req, std::vector<ConfigNode> ConfigPars, int e)
{
    uri = req.GetFullPath();
    method = req.GetHeaderValue("method");
    pathRequested = req.GetHeaderValue("path");

    if (method == "GET" || method == "POST")
    {
        getResponse(req, ConfigPars, e);
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

