#include "cgiHeader.hpp"


Cgi::Cgi()
{

}

Cgi::~Cgi()
{

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


int fileChecking(const std::string& path)
{
    struct stat st;

    if (stat(path.c_str(), &st) != 0)
        return 404; // path invalid
    if (S_ISDIR(st.st_mode))
        return 403; // path is a directory, not a file
    if (S_ISREG(st.st_mode))
    {
        if ((st.st_mode & S_IXUSR) == 0)
            return 403; // no execute permission for owner
    }
    return 0;
}

pathInfo splitPathInfo(const Request &req)
{
    pathInfo _pathinfo;
    _pathinfo._pathInfo = "";
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

    _pathinfo.scriptFile = fullPath.substr(0, posLength);
    if (posLength < fullPath.length())
        _pathinfo._pathInfo = fullPath.substr(posLength);
    return _pathinfo;
}

std::string handleCgiRequest(const Request &req, std::vector<ConfigNode> ConfigPars)
{
    Cgi obj;

    pathInfo _pathinfo = splitPathInfo(req);
    obj.setScriptPath(_pathinfo.scriptFile);
    int code = fileChecking(obj.getScriptPath());
    if (code == 403)
        return responseError(403, " Forbidden", ConfigPars);
    else if (code == 404)
        return responseError(404, " not found", ConfigPars);
    obj.setScriptOutput(obj.executeCgiScript(req, ConfigPars, _pathinfo._pathInfo));
    // need to check time out
    CgiResponse parsedCgi = obj.parseOutput(obj.getScriptOutput());
    return obj.formatHttpResponse(parsedCgi);
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
        return 1;
    return 0;
}
