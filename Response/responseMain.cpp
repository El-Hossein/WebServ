#include "responseHeader.hpp"

void    moveToResponse(int &client_fd, std::map<int, std::string>& response_map, Request	&req)
{
    std::string uri = req.GetFullPath();
    std::string finalResponse;

    if (IsCgiRequest(uri.c_str()))
    {
        // std::cout << "la hna" << std::endl;
        // exit (2);
        finalResponse = handleCgiRequest(req);
    }
    else
    {
        std::string _PATH = "." + uri;

        std::ifstream inFile(_PATH.c_str(), std::ios::binary);
        if (inFile)
        {
            std::ostringstream outStringFIle;
            outStringFIle << inFile.rdbuf(); // so i can read the whole file

            std::string fileBody = outStringFIle.str();
            finalResponse = "HTTP/1.1 200 OK\r\n";
            finalResponse += "Content-Length: ";
            finalResponse += intToString(fileBody.size());
            finalResponse += "\r\n";
            finalResponse += "Content-Type: text/html\r\n";
            finalResponse += "\r\n";
            finalResponse += fileBody;
        }
        else
        {
            finalResponse = "HTTP/1.1 404 Not Found\r\n\r\n";
        }
        // std::cout << "hna" << std::endl;
        // exit (1);

    }
    // std::cout << "hna " << std::endl;
    response_map[client_fd] = finalResponse;
}