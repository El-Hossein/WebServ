#include "../AllServer/HttpServer.hpp"
#include "responseHeader.hpp"

std::string handWritingError(const std::string& message, int statusCode)
{
    std::string _code = intToString(statusCode);

    std::string html = "<!DOCTYPE html><html lang=\"en\">";
    html += "<head><meta charset=\"UTF-8\">";
    html += "<title>Error " + _code + "</title>";
    html += "<style>";
    html += "body { background: linear-gradient(to bottom, #7C0A02, #5A0000, #7C0A02); "
            "font-family: 'Segoe UI', sans-serif; color: #f8e1e1; margin: 0; padding: 40px; "
            "display: flex; justify-content: center; align-items: center; height: 100vh; }\n";

    html += ".error-container { max-width: 600px; background: rgba(0,0,0,0.25); padding: 30px; "
            "border-radius: 12px; box-shadow: 0 8px 32px rgba(0,0,0,0.5); text-align: center; "
            "backdrop-filter: blur(10px); animation: fadeIn 0.6s ease-in-out; }\n";

    html += "h1 { font-size: 64px; color: #ffb3b3; margin-bottom: 20px; "
            "text-shadow: 0 0 10px rgba(255, 179, 179, 0.8); animation: slideDown 0.5s ease-in-out; }\n";

    html += "p { font-size: 20px; color: #ffd6d6; margin-bottom: 0; }\n";

    html += "@keyframes fadeIn { from { opacity: 0; } to { opacity: 1; } }\n";
    html += "@keyframes slideDown { from { transform: translateY(-20px); opacity: 0; } "
            "to { transform: translateY(0); opacity: 1; } }\n";

    html += "</style></head>";
    html += "<body>";
    html += "<div class=\"error-container\">";
    html += "<h1>🚫 Error " + _code + "</h1>";
    html += "<p>" + message + "</p>";
    html += "</div>";
    html += "</body></html>";

    return html;
}

std::string readFileToString(const std::string& path)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file)
        return "";
    std::ostringstream contents;
    contents << file.rdbuf();
    file.close();
    return contents.str();
}

void     Response::responseError(int statusCode, const std::string& message, std::vector<ConfigNode> ConfigPars, Request &req)
{
    std::string body;

    std::string	 loc = req.GetRightServer().GetRightLocation(req.GetHeaderValue("path"));
    std::string root = getInfoConfig(ConfigPars, "root", loc, req);
    std::vector<std::string> error_page = getInfoConfigMultiple(ConfigPars, "error_page", loc, req);
    for (size_t i = 0; i + 1 < error_page.size(); i += 2)
	{
		if (std::atoi(error_page[i].c_str()) == statusCode)
		{
			std::string errorPath = root;
			if (!root.empty() && root.back() != '/')
				errorPath += "/";
			errorPath += error_page[i + 1];
			body = readFileToString(errorPath);
			break;
		}
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
        case 505: headers += " HTTP Version Not Supported"; break;
        case 504: headers += " Gateway Timeout"; break;
        case 405: headers += " Method Not Allowed"; break;
    }
    headers += "\r\n";
    headers += "Content-Type: text/html\r\n";
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

bool Response::getNextChunk(size_t chunkSize)
{
    chunk.clear();

    // static file headers
    if (headerSent < headers.size())
    {
        size_t left = headers.size() - headerSent;
        size_t sendNow = std::min(chunkSize, left);
        chunk = headers.substr(headerSent, sendNow);
		// std::cout << "\033[34m++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ RESPONSE static headers\033[0m" << std::endl;
        // std::cout << Response::getClientFd() << " | " << pathRequested << std::endl;
        // std::cout << chunk << std::endl;
		// std::cout << "\033[34m++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ END RESPONSE static headers\033[0m" << std::endl;
        headerSent += sendNow;
        return true;
    }

    // cgi headers
    if (_cgi.getCgiHeaderSent() < _cgi.getCgiHeader().size())
    {
        size_t left = _cgi.getCgiHeader().size() - _cgi.getCgiHeaderSent();
        size_t sendNow = std::min(chunkSize, left);
        chunk = _cgi.getCgiHeader().substr(_cgi.getCgiHeaderSent(), sendNow);
		// std::cout << "\033[34m++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ RESPONSE Cgi headers\033[0m" << std::endl;
        // std::cout << Response::getClientFd() << " | " << pathRequested  << std::endl;
        // std::cout << chunk << std::endl;
		// std::cout << "\033[34m++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ END RESPONSE Cgi headers\033[0m" << std::endl;
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
		// std::cout << "\033[34m++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ RESPONSE Cgi\033[0m" << std::endl;
        // std::cout << Response::getClientFd() << " | " << pathRequested  << std::endl;
        // std::cout << chunk << std::endl;
		// std::cout << "\033[34m++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ END RESPONSE Cgi\033[0m" << std::endl;
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

		// std::cout << "\033[34m++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ RESPONSE static\033[0m" << std::endl;
        // std::cout << Response::getClientFd() << " | " << pathRequested  << std::endl;
        // std::cout << chunk << std::endl;
		// std::cout << "\033[34m++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ END RESPONSE static\033[0m" << std::endl;
            return true;
        }

        delete [] buffer;
        file.close();
    }
    return false;
}

bool Response::checkPendingCgi(std::vector<ConfigNode> ConfigPars, Request &req) 
{
    if (!_cgi.gethasPendingCgi())
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
            int exCode = WEXITSTATUS(status);
            if (exCode == 0)
            {
                _cgi.parseOutput();
                _cgi.formatHttpResponse(_cgi.getoutfile(), req);
                _cgi.setcgistatus(CGI_COMPLETED);
            }
            else
            {
                _cgi.responseErrorcgi(500, " Internal Server Error", ConfigPars, req);
                _cgi.setcgistatus(CGI_ERROR);
            }
        }
        else if (WIFSIGNALED(status))
        {
            _cgi.responseErrorcgi(500, " Internal Server Error", ConfigPars, req);
            _cgi.setcgistatus(CGI_ERROR);
        }
        else
        {
            _cgi.responseErrorcgi(500, " Internal Server Error", ConfigPars, req);
            _cgi.setcgistatus(CGI_ERROR);
        }

        if (!_cgi.getinfile().empty())
            unlink(_cgi.getinfile().c_str());
        if (!_cgi.getoutfile().empty())   
            unlink(_cgi.getoutfile().c_str());

        _cgi.sethasPendingCgi(false);
        return true;
    }
    else if (result == -1)
    {
        struct kevent kev;
        EV_SET(&kev, childPid, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
        kevent(globalKq, &kev, 1, NULL, 0, NULL);
        
        _cgi.responseErrorcgi(500, " Internal Server Error", ConfigPars, req);
        _cgi.setcgistatus(CGI_ERROR);
        _cgi.sethasPendingCgi(false);
        
        if (!_cgi.getinfile().empty())
            unlink(_cgi.getinfile().c_str());
        if (!_cgi.getoutfile().empty())   
            unlink(_cgi.getoutfile().c_str());
        
        return true;
    }

    return false;
}