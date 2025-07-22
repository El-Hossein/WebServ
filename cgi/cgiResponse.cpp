#include "cgiHeader.hpp"

bool Cgi::formatHttpResponse(std::string cgiFilePath)
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
    //need to check connection 
    if (cgiHeader.find("Connection") == std::string::npos)
        httpResponse += "Connection: close\r\n\r\n";
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

bool Cgi::responseErrorcgi(int statusCode, std::string message, std::vector<ConfigNode> ConfigPars)
{
    std::string body;
    (void)ConfigPars;
    switch (statusCode)
    {
        // i need to read error pages provided by config file
        case 403: body = readFileToStringcgi("/Users/i61mail/Desktop/WebServ/Response/errorPages/403.html"); break;
        case 404: body = readFileToStringcgi("/Users/i61mail/Desktop/WebServ/Response/errorPages/404.html"); break;
        case 500: body = readFileToStringcgi("/Users/i61mail/Desktop/WebServ/Response/errorPages/500.html"); break;
        case 501: body = readFileToStringcgi("/Users/i61mail/Desktop/WebServ/Response/errorPages/501.html"); break;
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
    }
    cgiHeader += "\r\n";
    cgiHeader += "Content-Type: text/html\r\n";
    cgiHeader += "Content-Length: " + intToString(body.length()) + "\r\n";
    cgiHeader += "Connection: close\r\n\r\n";

    
    cgiHeaderSent = 0;
    return false;
}
