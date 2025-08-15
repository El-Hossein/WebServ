#include "responseHeader.hpp"

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
    
    headers = "HTTP/1.1 200 OK\r\n";
    headers += contentType;
    if (contentType == "Content-Type: video/mp4\r\n")
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

std::string frontPage(std::string uri)
{
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
    return html;
}

std::string Response::generateListingDir(Request &req)
{
    DIR *dirCheck = opendir(uri.c_str());
    if (!dirCheck)
        return "";

    std::string html = frontPage(uri);
    struct dirent *dir;
    while ((dir = readdir(dirCheck)) != NULL)
    {
        std::string name = dir->d_name;
        if (name[0] == '.' && name != "." && name != "..")
            continue;

        std::string path = pathRequested;
        if (pathRequested[pathRequested.size() - 1] != '/')
            path += '/';
        path += name;
        std::string icon = (dir->d_type == DT_DIR) ? "📁" : "📄";
        html += "<li><span class=\"icon\">" + icon + "</span><a href=\"" + path + "\">" + name + "</a></li>\n";
    }
    closedir(dirCheck);
    html += "</ul>\n";
    html += "<div class=\"upload-section\">\n";
    html += "<h2>⬆️ Upload a File</h2>\n";
    html += "<form action=\"" + req.GetHeaderValue("path") + "\" method=\"post\" enctype=\"multipart/form-data\">\n";
    html += "<label class=\"file-input\" id=\"fileLabel\">Choose File<input type=\"file\" name=\"file\" id=\"fileInput\"></label><br>\n";
    html += "<input type=\"submit\" value=\"Upload\">\n";
    html += "</form>\n";
    html += "</div>\n</body>\n</html>\n";
    return html;
}

bool Response::generateAutoIndexOn(std::vector<ConfigNode> ConfigPars, Request &req)
{
    staticFileBody = generateListingDir(req);
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

void Response::servListingDiren(std::vector<ConfigNode> ConfigPars, Request	&req)
{
	std::string	 loc = req.GetRightServer().GetRightLocation(req.GetHeaderValue("path"));
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
            if (S_ISDIR(st.st_mode) || access(htmlFound.c_str(), R_OK) != 0)
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
        case 100: headers += " Continue"; break;
        case 101: headers += " Switching Protocols"; break;
        case 102: headers += " Processing"; break;
        case 200: headers += " OK"; break;
        case 201: headers += " Created"; break;
        case 202: headers += " Accepted"; break;
        case 203: headers += " Non-Authoritative Information"; break;
        case 204: headers += " No Content"; staticFileBody = ""; redirectUrl.clear(); break;
        case 205: headers += " Reset Content"; break;
        case 206: headers += " Partial Content"; break;
        case 400: headers += " Bad Request"; break;
        case 401: headers += " Unauthorized"; break;
        case 402: headers += " Payment Required"; break;
        case 403: headers += " Forbidden"; break;
        case 404: headers += " Not Found"; break;
        case 405: headers += " Method Not Allowed"; break;
        case 406: headers += " Not Acceptable"; break;
        case 408: headers += " Request Timeout"; break;
        case 409: headers += " Conflict"; break;
        case 410: headers += " Gone"; break;
        case 411: headers += " Length Required"; break;
        case 413: headers += " Payload Too Large"; break;
        case 414: headers += " URI Too Long"; break;
        case 415: headers += " Unsupported Media Type"; break;
        case 429: headers += " Too Many Requests"; break;
        case 500: headers += " Internal Server Error"; break;
        case 501: headers += " Not Implemented"; break;
        case 502: headers += " Bad Gateway"; break;
        case 503: headers += " Service Unavailable"; break;
        case 504: headers += " Gateway Timeout"; break;
        case 505: headers += " HTTP Version Not Supported"; break;
    }
    headers += "\r\n";
    headers += "Content-Type: text/html\r\n";
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

void     Response::prepareRedirectResponse(std::vector<std::string> redirect, Request &req, std::vector<ConfigNode> ConfigPars)
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
        return ;
    }
    staticFilePos = 0;
    usingStaticFile = true;
    filePos = 0;
    headers = "HTTP/1.1 " + intToString(statusCode);
    std::string mess;
    switch (statusCode)
    {
        case 301: headers += " Moved Permanently"; mess = " Moved Permanently";break;
        case 302: headers += " Moved Temporarily"; mess = " Found"; break;
        case 303: headers += " See Other"; mess = " See Other";break;
        case 304: headers += " Not Modified"; mess = " Not Modified";break;
        case 307: headers += " Temporary Redirect"; mess = " Temporary Redirect";break;
        case 308: headers += " Permanent Redirect"; mess = " Permanent Redirect";break;
    }
    staticFileBody = "<html>\n"
            "<head><title>" + intToString(statusCode) + mess + "</title></head>\n"
            "<body>\n"
            "<center><h1>" + intToString(statusCode) + mess + "</h1></center>\n"
            "<center><h1>webSERV/1.1</h1></center>\n"
            "</body>\n"
            "</html>\n";

    headers += "\r\n";
    headers += "Content-Type: text/html\r\n";
    headers += "Location: " + redirectUrl + "\r\n";
    if (statusCode == 304 || (statusCode != 301 && statusCode != 302 && statusCode != 303 
        && statusCode != 307 && statusCode != 308))
        staticFileBody.clear();
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

void    Response::getResponse(Request	&req, std::vector<ConfigNode> ConfigPars)
{
    struct stat st;

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