#include "responseHeader.hpp"

std::string deleteResponseSuccess(const std::string& message)
{
    std::string html = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Error</title></head>";
    html += "<body><h1>";
    html += message;
    html += "</h1></body></html>";

    return html;
}

std::string generateListingDir(const std::string& uri, std::string &hh)
{
    DIR *dirCheck = opendir(uri.c_str());
    if (!dirCheck)
        return "<html><body><h1>Cannot open directory</h1></body></html>";

    std::string html = "<html><head><title>Index of " + uri + "</title></head><body>";
    html += "<h1>Index of " + uri + "</h1><ul>";

    struct dirent *dir;
    while ((dir = readdir(dirCheck)) != NULL)
    {
        // i need to check hh if has /
        std::string name = hh + (hh[hh.length() - 1] == '/' ? "": "/") + dir->d_name;

        if (name != ".")
            html += "<li><a href=\"" + name + "\">" + name + "</a></li>";
    }
    closedir(dirCheck);

    html += "</ul></body></html>";
    return html;
}

void    servListingDiren(std::string &uri, std::string &finalResponse, std::string &hh)
{
    std::string autoIndexOn = "on"; 
    std::string htmlFound = uri + "/index.html";


    std::ifstream htmlStream(htmlFound.c_str(), std::ios::binary);
    if (htmlStream)
    {
        std::ostringstream htmlContent;
        htmlContent << htmlStream.rdbuf();
        htmlStream.close();
        std::string body = htmlContent.str();

        finalResponse = "HTTP/1.1 200 OK\r\n";
        finalResponse += "Content-Length: " + intToString(body.size()) + "\r\n";
        finalResponse += "Content-Type: text/html\r\n";
        finalResponse += "Connection: close\r\n\r\n";
        finalResponse += body;
        
    }
    else if (autoIndexOn == "on")
    {
        std::string body = generateListingDir(uri, hh);

        finalResponse = "HTTP/1.1 200 OK\r\n";
        finalResponse += "Content-Length: " + intToString(body.size()) + "\r\n";
        finalResponse += "Content-Type: text/html\r\n";
        finalResponse += "Connection: close\r\n\r\n";
        finalResponse += body;
    }
    else
    {
        finalResponse = responseError(403, " Forbidden");
    }
}


void    moveToResponse(int &client_fd, std::map<int, std::string>& response_map, Request	&req)
{
    std::string uri = req.GetFullPath();
    std::string finalResponse;
    struct stat st;

    std::string hh = req.GetHeaderValue("Path");
    std::string method = "GET";
    if (method == "GET") // i need to check for allowed methods
    {
        //if method not allowed return 405 Method Not Allowed
        struct stat stList;

        if (stat(uri.c_str(), &stList) == 0 && S_ISDIR(stList.st_mode))
        {
            servListingDiren(uri, finalResponse, hh);
        }
        else if (IsCgiRequest(uri.c_str()))
        {
            finalResponse = handleCgiRequest(req);
        }
        else
        {
            // i need to check the errors thrown for the file before open it
            std::ifstream inFile(uri.c_str(), std::ios::binary);
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
                //need to check content type
                finalResponse += "Content-Type: text/html\r\n";
                finalResponse += "\r\n";
                finalResponse += fileBody;
            }
            else
            {
                // and need to display given error page
                finalResponse = responseError(404, " Not Found"); // just a test i have to catsh error codes ziad throw
            }
        }
    }
    else if (method == "DELETE") // i need to check for allowed methods
    {
        // if method not allowed return 405 Method Not Allowed
        // i need to check the errors thrown for the file before open it
        if (stat(uri.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
        {
            finalResponse = responseError(403, " Permission Denied");
        }
        else if (std::remove(uri.c_str()) == 0)
        {
            std::string body = deleteResponseSuccess("file succesefuly deleted!!");
            finalResponse = "HTTP/1.1 200 OK\r\n";
            finalResponse += "Content-Type: text/html\r\n";
            finalResponse += "Content-Length: " + intToString(body.size()) + "\r\n";
            finalResponse += "Connection: close\r\n\r\n";
            finalResponse += body;
        }
        else
        {
            finalResponse = responseError(404, " Not Found");
        }
    }

    else
    {
        finalResponse = responseError(501, " Method not implemented");
    }
    // std::cout << finalResponse << std::endl;
    response_map[client_fd] = finalResponse;
}
