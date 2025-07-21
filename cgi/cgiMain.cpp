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
    ss << getpid() << "_" << time(NULL) << "_" << rand();
    uniq = ss.str();
}

Cgi::~Cgi()
{

}

std::string    Cgi::getinfile()
{
    return inpFile;
}

std::string    Cgi::getoutfile()
{
    return outFile;
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

size_t  Cgi::getFilePos()
{
    return cgiFilePos;
}

void Cgi::setCgiHeaderSent(size_t aa)
{
    cgiHeaderSent = aa;
}

ssize_t Cgi::getFileSize()
{
    return cgiFileSize;
}

std::string Cgi::getCgiChunk()
{
    return cgiChunk;
}

std::ifstream& Cgi::getFile()
{
    return file;
}

bool    Cgi::getUsingCgi()
{
    return usingCgi;
}

std::string Cgi::getScriptFile()
{
    return scriptFile;
}

void    Cgi::setScriptFile(std::string _scriptfile)
{
    scriptFile = _scriptfile;
}

std::string Cgi::getPathInfo()
{
    return scriptFile;
}

void    Cgi::setPathInfo(std::string _pathinfo)
{
    pathInfo = _pathinfo;
}

std::string Cgi::getScriptPath()
{
    return scriptPath;
}

void    Cgi::setScriptPath(std::string _scriptPath)
{
    scriptPath = _scriptPath;
}

int Cgi::getStatus()
{
    return status;
}

std::string Cgi::getScriptOutput()
{
    return scriptOutput;
}

void    Cgi::setScriptOutput(std::string _scriptOutput)
{
    scriptOutput = _scriptOutput;
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
        return 404; // path invalid
    if (S_ISDIR(st.st_mode))
        return 403;
    if (S_ISREG(st.st_mode))
    {
        if ((st.st_mode & S_IXUSR) == 0)
            return 403;
    }
    return 0;
}

void Cgi::splitPathInfo(const Request &req)
{
    std::string fullPath = req.GetFullPath();

    size_t quotationPos = fullPath.find('?');
    if (quotationPos != std::string::npos)
        fullPath = fullPath.substr(0, quotationPos);

    size_t posLength = std::string::npos;
    size_t cgiPos = fullPath.find(".cgi");
    size_t phpPos = fullPath.find(".php");
    size_t pyPos = fullPath.find(".py");
    if (cgiPos != std::string::npos && cgiPos < phpPos && cgiPos < pyPos)
        posLength = cgiPos + 4;
    else if (phpPos != std::string::npos && phpPos < cgiPos && phpPos < pyPos)
        posLength = phpPos + 4;
    else if (pyPos != std::string::npos && pyPos < cgiPos && pyPos < phpPos)
        posLength = pyPos + 3;

    scriptFile = fullPath.substr(0, posLength);
    if (posLength < fullPath.length())
        pathInfo = fullPath.substr(posLength);
}

void Cgi::handleCgiRequest(const Request &req, std::vector<ConfigNode> ConfigPars)
{
    splitPathInfo(req);
    int code = fileChecking(scriptFile);
    if (code == 403)
    {
        responseErrorcgi(403, " Forbidden", ConfigPars);
        return ;
    }
    else if (code == 404)
    {
        responseErrorcgi(404, " not found", ConfigPars);
        return ;
    }
    int _status = executeCgiScript(req, ConfigPars);
    pid_1 = pid;
    if (_status == 1)
    {
        parseOutput();
        formatHttpResponse(outFile);
    }
}

int IsCgiRequest(const char *uri)
{
    std::string uriString(uri);
    size_t index = uriString.find("/cgiScripts/");
    if (index == std::string::npos)
        return 0;
    std::string pathAfterCgi = uriString.substr(index + 12);
    if (pathAfterCgi.empty())
        return 0;
    size_t firstSlash = pathAfterCgi.find("/");
    std::string scriptFile;
    if (firstSlash != std::string::npos)
        scriptFile = pathAfterCgi.substr(0, firstSlash);
    else
        scriptFile = pathAfterCgi;
    if (scriptFile.find(".cgi") != std::string::npos || scriptFile.find(".py") != std::string::npos || scriptFile.find(".php") != std::string::npos)
    {
        return 1;
    }
    return 0;
}
