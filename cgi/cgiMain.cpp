#include "cgiHeader.hpp"

Cgi::Cgi()
{
    cgiChunk = "";
    usingCgi = false;
    cgiFilePos = 0;
    cgiFileSize = 0;
    cgiHeaderSent = 0;
    usingCgiStatFile = false;
    statCgiFilePos = 0;
    std::ostringstream ss;
    int random;
    ss << "_" << std::time(NULL) << "_" << std::rand() << &random;
    uniq = ss.str();
    checkConnection = _Empty;
    cgiContentLength = -1;
}

Cgi::~Cgi()
{

}

std::string    Cgi::getcgiBody()
{
    return this->cgiBody;
}

void     Cgi::setKq(int _kq)
{
    kq = _kq;
}

void    Cgi::setCgiCL(long _cgiCL)
{
    cgiContentLength = _cgiCL;
}

long     Cgi::getCgiCL()
{
    return cgiContentLength;
}

void    Cgi::sethasPendingCgi(bool pendingcgi)
{
    hasPendingCgi = pendingcgi;
}

bool    Cgi::gethasPendingCgi()
{
    return hasPendingCgi;
}

bool    Cgi::getCheckConnection()
{
    return checkConnection;
}

void    Cgi::setCheckConnection(int conn)
{
    checkConnection = conn;
}

std::string    Cgi::getoutfile()
{
    return outFile;
}

std::string    Cgi::getinfile()
{
    return inpFile;
}

time_t   Cgi::gettime()
{
    return startTime;
}

pid_t    Cgi::getpid_1()
{
    return pid_1;
}

void    Cgi::setcgistatus(int _cgistatus)
{
    cgistatus = _cgistatus;
}

int   Cgi::getcgistatus()
{
    return cgistatus;
}

void    Cgi::setcgiHeader(std::string _cgiHeader)
{
    cgiHeader = _cgiHeader;
}

size_t Cgi::getStatCgiFilePos()
{
    return statCgiFilePos;
}

void    Cgi::setStatCgiFilePos(size_t _statCgifilepos)
{
    statCgiFilePos = _statCgifilepos;
}

bool  Cgi::getUsingStatCgiFile()
{
    return usingCgiStatFile;
}

void    Cgi::setUsingStatCgiFile(bool _usingcgistatfile)
{
    usingCgiStatFile = _usingcgistatfile;
}

std::string Cgi::getStatCgiFileBody()
{
    return statCgiFileBody;
}

std::string    Cgi::getCgiHeader()
{
    return cgiHeader;
}

size_t  Cgi::getCgiHeaderSent()
{
    return cgiHeaderSent;
}

void    Cgi::setFilePos(size_t _filepos)
{
    cgiFilePos = _filepos;
}

long  Cgi::getFilePos()
{
    return cgiFilePos;
}

void Cgi::setCgiHeaderSent(size_t _cgiHeaderSent)
{
    cgiHeaderSent = _cgiHeaderSent;
}

long Cgi::getFileSize()
{
    return cgiFileSize;
}

std::ifstream& Cgi::getFile()
{
    return file;
}

bool    Cgi::getUsingCgi()
{
    return usingCgi;
}

std::string Cgi::getInfoConfigCgi(std::string what, std::string location, Request &req)
{
    ConfigNode a = req.GetRightServer();

    std::vector<std::string> temp = a.getValuesForKey(a, what, location);
    if (!temp.empty())
        return temp[0];
	return "";
}

std::vector<std::string> Cgi::getInfoConfigMultipleCgi(std::string what, std::string location, Request &req)
{
    ConfigNode a = req.GetRightServer();

    return a.getValuesForKey(a, what, location);
}

std::string intToString(int n)
{
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

int fileChecking(std::string path)
{
    struct stat st;

    if (stat(path.c_str(), &st) != 0)
        return 404;
    if (S_ISDIR(st.st_mode))
        return 403;
    if (S_ISREG(st.st_mode))
    {
        if (access(path.c_str(), X_OK | R_OK) != 0)
            return 403;
    }
    return 0;
}

void Cgi::splitPathInfo(Request &req)
{
    fullPath = req.GetFullPath();
    memoExt = "";

    size_t quotationPos = fullPath.find('?');
    if (quotationPos != std::string::npos)
        fullPath = fullPath.substr(0, quotationPos);

    size_t posLength = std::string::npos;
    size_t cgiPos = fullPath.find(".cgi");
    size_t phpPos = fullPath.find(".php");
    size_t pyPos = fullPath.find(".py");
    if (cgiPos != std::string::npos && cgiPos < phpPos && cgiPos < pyPos)
    {
        memoExt = "cgi";
        posLength = cgiPos + 4;
    }
    else if (phpPos != std::string::npos && phpPos < cgiPos && phpPos < pyPos)
    {
        memoExt = "php";
        posLength = phpPos + 4;
    }
    else if (pyPos != std::string::npos && pyPos < cgiPos && pyPos < phpPos)
    {
        memoExt = "py";
        posLength = pyPos + 3;
    }
    scriptFile = fullPath.substr(0, posLength);
    if (posLength < fullPath.length())
        pathInfo = fullPath.substr(posLength);
}

int Cgi::checkLocationCgi(Request &req, std::string meth, std::string directive)
{
    std::string	 loc = req.GetRightServer().GetRightLocation(req.GetHeaderValue("path"));
    std::vector<std::string> allowed_cgi = getInfoConfigMultipleCgi(directive, loc, req);
    if (std::find(allowed_cgi.begin(), allowed_cgi.end(), meth) == allowed_cgi.end())
    {
        responseErrorcgi(403, " Forbidden", req);
        return -1;
    }
    return 0;
}

void Cgi::handleCgiRequest(Request &req)
{
    splitPathInfo(req);
    int code = fileChecking(scriptFile);
    switch (code)
    {
        case 403: responseErrorcgi(403, " Forbidden", req); return;
        case 404: responseErrorcgi(404, " not found", req); return;
    }
    if (!memoExt.empty())
    {
        if (checkLocationCgi(req, memoExt, "allow_cgi") == -1)
            return ;
        memoExt = "";
    }
    executeCgiScript(req);
    pid_1 = pid;
}

int Cgi::IsCgiRequest(std::string uri, Request &req)
{
    size_t index = uri.find("/cgiScripts/");
    if (index == std::string::npos)
    {
        if (uri.find("/cgiScripts") != std::string::npos)
        {
            if (servListingDirenCgi(req, uri) == -1)
                return -1;
            return 0;
        }
        else
            return 0;
    }
    std::string pathAfterCgi = uri.substr(index + 12);
    if (pathAfterCgi.empty())
        return 0;
    size_t firstSlash = pathAfterCgi.find("/");
    std::string scriptFile;
    if (firstSlash != std::string::npos)
        scriptFile = pathAfterCgi.substr(0, firstSlash);
    else
        scriptFile = pathAfterCgi;
    if (scriptFile.find(".cgi") != std::string::npos || scriptFile.find(".py") != std::string::npos 
        || scriptFile.find(".php") != std::string::npos)
        return 1;
    return 0;
}