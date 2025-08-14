#include "../AllServer/HttpServer.hpp"
#include "responseHeader.hpp"

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

    int     status;
    pid_t   childPid = _cgi.getpid_1();
    int     result = waitpid(childPid, &status, WNOHANG);
    if (result > 0)
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
                _cgi.prepareFileResponseCgi(req);
                _cgi.setcgistatus(CGI_COMPLETED);
            }
            else
            {
                _cgi.responseErrorcgi(500, " Internal Server Error", ConfigPars, req);
                _cgi.setcgistatus(CGI_ERROR);
            }
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