#include "responseHeader.hpp"

void    moveToResponse(int &client_fd, std::map<int, std::string>& response_map, Request	&req)
{
    std::string uri = req.GetFullPath();
    std::string finalResponse;

    if (IsCgiRequest(uri.c_str()))
    {
        finalResponse = handleCgiRequest(req);
    }
    else
    {
        std::string _PATH = uri; // i need to combine the root directory with the uri

        std::ifstream inFile(_PATH.c_str(), std::ios::binary);
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
            finalResponse += "Content-Type: text/html\r\n";
            finalResponse += "\r\n";
            finalResponse += fileBody;
        }
        else
        {
            // and need to display given error page
            finalResponse += responseError(404, " Not Found"); // just a test i have to catsh error codes ziad throw
        }
    }
    std::cout << finalResponse << std::endl;
    response_map[client_fd] = finalResponse;
}
