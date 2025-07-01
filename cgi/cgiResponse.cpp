#include "cgiHeader.hpp"

std::string formatHttpResponse(const CgiResponse& cgiResponse)
{
    // protocol version and status code and reason phrase 
    std::string httpResponse = "HTTP/1.1 " + intToString(cgiResponse.status_code);
    switch (cgiResponse.status_code)
    {
        case 200: httpResponse += " OK"; break;
        case 403: httpResponse += " Forbidden"; break;
        case 404: httpResponse += " Not Found"; break;
        case 500: httpResponse += " Internal Server Error"; break;
        // need to add more
    }
    // headers
    httpResponse += "\r\n";
    httpResponse += cgiResponse.headers;
    httpResponse += "Content-Length: " + intToString(cgiResponse.body.length()) + "\r\n\r\n";
    // body
    httpResponse += cgiResponse.body;
    return httpResponse;
}

std::string responseError(int status_code, const std::string& message)
{
    std::string body = "<html><body><h1>" + intToString(status_code) + message + "</p></body></html>";
    std::string response = "HTTP/1.1 " + intToString(status_code);
    switch (status_code)
    {
        case 403: response += " Forbidden"; break;
        case 404: response += " Not Found"; break;
        case 500: response += " Internal Server Error"; break;
        // default: response += " Error"; break;
    }
    response += "\r\n";
    response += "Content-Type: text/html\r\n"; // i guess its not always text/html i need to search abt this
    response += "Content-Length: " + intToString(body.length()) + "\r\n\r\n";
    response += body;
    return response;
}