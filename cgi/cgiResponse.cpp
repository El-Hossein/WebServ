#include "cgiHeader.hpp"

bool Cgi::formatHttpResponse(std::string cgiFilePath, Request &req)
{
    file.open(outFile.c_str(), std::ios::binary);
    if (!file.is_open())
        return false;
    
    cgiFilePos = 0;
    usingCgi = true;


    std::string httpResponse;
    if (cgiHeader.find("Status") == std::string::npos)
        httpResponse = "HTTP/1.1 " + intToString(cgiStatusCode);
    switch (cgiStatusCode)
    {
        case 200: httpResponse += " OK"; break;
        case 302: httpResponse += " Found"; break;
        case 403: httpResponse += " Forbidden"; break;
        case 404: httpResponse += " Not Found"; break;
        case 500: httpResponse += " Internal Server Error"; break;
        case 501: httpResponse += " Method not implemented"; break;
    }
    httpResponse += "\r\n";
    httpResponse += cgiHeader;
    if (cgiHeader.find("Content-Type") == std::string::npos)
        httpResponse += "Content-Type: text/html\r\n";
    if (cgiHeader.find("Content-Length") == std::string::npos)
        httpResponse += "Content-Length: " + intToString(cgiFileSize) + "\r\n";
    if (cgiHeader.find("Connection") == std::string::npos)
    {
        if (req.GetHeaderValue("connection") == "keep-alive")
        {
            httpResponse += "Connection: keep-alive\r\n\r\n";
            checkConnection = keepAlive;
        }
        else
        {
            httpResponse += "Connection: close\r\n\r\n";
            checkConnection = _close;
        }

    }
    cgiHeader = httpResponse;
    cgiHeaderSent = 0;
    
    return true;
}

std::string readFileToStringcgi(const std::string& path)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file)
        return ""; // if not found need to give the ones that are saved
    std::ostringstream contents;
    contents << file.rdbuf();
    file.close();
    return contents.str();
}

std::string handWritingErrorcgi(const std::string& message, int statusCode)
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

std::string readFileToStringCgi(const std::string& path)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file)
        return "";
    std::ostringstream contents;
    contents << file.rdbuf();
    file.close();
    return contents.str();
}

std::string Cgi::getInfoConfigCgi(std::vector<ConfigNode> ConfigPars, std::string what, std::string location, Request &req)
{
    ConfigNode a = req.GetRightServer();

    std::vector<std::string> temp = a.getValuesForKey(a, what, location);
    if (!temp.empty())
        return temp[0];
	return "";
}

bool Cgi::responseErrorcgi(int statusCode, std::string message, std::vector<ConfigNode> ConfigPars, Request &req)
{
    std::string body;
    std::string	 loc = req.GetRightServer().GetRightLocation(req.GetHeaderValue("path"));
    std::string root = getInfoConfigCgi(ConfigPars, "root", loc, req);
    std::vector<std::string> error_page = getInfoConfigMultipleCgi(ConfigPars, "error_page", loc, req);
    for (size_t i = 0; i + 1 < error_page.size(); i += 2)
	{
		if (std::atoi(error_page[i].c_str()) == statusCode)
		{
			std::string errorPath = root;
			if (!root.empty() && root.back() != '/')
				errorPath += "/";
			errorPath += error_page[i + 1];
			body = readFileToStringCgi(errorPath);
			break;
		}
	}
    if (body.empty())
        body = handWritingErrorcgi(message, statusCode);
    statCgiFileBody = body;
    statCgiFilePos = 0;
    usingCgiStatFile = true;


    cgiHeader = "HTTP/1.1 " + intToString(statusCode);
    switch (statusCode)
    {
        case 403: cgiHeader += " Forbidden"; break;
        case 404: cgiHeader += " Not Found"; break;
        case 500: cgiHeader += " Internal Server Error"; break;
        case 501: cgiHeader += " Method not implemented"; break;
        case 400: cgiHeader += " Bad Request"; break;
        case 413: cgiHeader += " Content Too Large"; break;
        case 414: cgiHeader += " URI Too Long"; break;
        case 505: cgiHeader += " HTTP Version Not Supported"; break;
        case 504: cgiHeader += " Gateway Timeout"; break;
        case 405: cgiHeader += " Method Not Allowed"; break;
    }
    cgiHeader += "\r\n";
    cgiHeader += "Content-Type: text/html\r\n";
    cgiHeader += "Content-Length: " + intToString(body.length()) + "\r\n";
    if (req.GetHeaderValue("connection") == "keep-alive")
    {
        cgiHeader += "Connection: keep-alive\r\n\r\n";
        checkConnection = keepAlive;
    }
    else
    {
        cgiHeader += "Connection: close\r\n\r\n";
        checkConnection = _close;
    }

    
    cgiHeaderSent = 0;
    return false;
}
