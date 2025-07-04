#include "cgiHeader.hpp"

std::string formatHttpResponse(const CgiResponse& cgiResponse)
{
    // protocol version and status code and reason phrase 
    std::string httpResponse = "HTTP/1.1 " + intToString(cgiResponse.status_code);
    httpResponse += " OK";
    // switch (cgiResponse.status_code)
    // {
    //     case 200: httpResponse += " OK"; break;
    //     // case 403: httpResponse += " Forbidden"; break;
    //     // case 404: httpResponse += " Not Found"; break;
    //     // case 500: httpResponse += " Internal Server Error"; break;
    // }
    // headers
    httpResponse += "\r\n";
    httpResponse += cgiResponse.headers;
    httpResponse += "Content-Length: " + intToString(cgiResponse.body.length()) + "\r\n\r\n";
    // body
    httpResponse += cgiResponse.body;
    return httpResponse;
}

std::string readFileToString(const std::string& path)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file)  // need to give error pages and check if they are valid
        return "";
    std::ostringstream contents;
    contents << file.rdbuf(); // read the whole file
    file.close();
    return contents.str();
}

std::string handWritingError(const std::string& message, int statusCode)
{
    std::string _code = intToString(statusCode);

    std::string html = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Error</title></head>";
    html += "<body><h1>";
    html += _code;
    html += " ";
    html += message;
    html += "</h1></body></html>";

    return html;
}

std::string responseError(int statusCode, const std::string& message)
{
    std::string body;
    switch (statusCode)
    {
        case 403: body = readFileToString("/Users/i61mail/Desktop/WebServ/Response/errorPages/403.html"); break;
        case 404: body = readFileToString("/Users/i61mail/Desktop/WebServ/Response/errorPages/404.html"); break;
        case 500: body = readFileToString("/Users/i61mail/Desktop/WebServ/Response/errorPages/500.html"); break;
        case 501: body = readFileToString("/Users/i61mail/Desktop/WebServ/Response/errorPages/501.html"); break;
    }
    if (body.empty())
        body = handWritingError(message, statusCode);
    std::string response = "HTTP/1.1 " + intToString(statusCode);
    switch (statusCode)
    {
        case 403: response += " Forbidden"; break;
        case 404: response += " Not Found"; break;
        case 500: response += " Internal Server Error"; break;
        case 501: response += " Method not implemented"; break;
    }
    response += "\r\n";
    response += "Content-Type: text/html\r\n"; // its not always text/html i need to search abt this
    response += "Content-Length: " + intToString(body.length()) + "\r\n";
    response += "Connection: close\r\n\r\n";
    response += body;
    
    return response;
}