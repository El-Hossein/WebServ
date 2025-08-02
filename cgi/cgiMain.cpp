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
    checkConnection = _Empty;
    hasPendingCgi = false;
}

Cgi::~Cgi()
{

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
        if (access(path.c_str(), X_OK | R_OK) != 0)
            return 403;
    }
    return 0;
}

void Cgi::splitPathInfo(Request &req)
{
    std::string fullPath = req.GetFullPath();
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



std::vector<std::string> Cgi::getInfoConfigMultipleCgi(std::vector<ConfigNode> ConfigPars, std::string what, std::string location, Request &req)
{
    ConfigNode a = req.GetRightServer();

    return a.getValuesForKey(a, what, location);
}

int Cgi::checkLocationCgi(Request &req, std::string meth, std::string directive, std::vector<ConfigNode> ConfigPars)
{
    std::string	 loc = req.GetRightServer().GetRightLocation(req.GetHeaderValue("path"));
    std::cout << loc << std::endl;
    std::vector<std::string> allowed_cgi = getInfoConfigMultipleCgi(ConfigPars, directive, loc, req);
    if (std::find(allowed_cgi.begin(), allowed_cgi.end(), meth) == allowed_cgi.end())
    {
        responseErrorcgi(403, " Forbidden", ConfigPars, req);
        return -1;
    }
    return 0;
}

void Cgi::handleCgiRequest(Request &req, std::vector<ConfigNode> ConfigPars)
{
    splitPathInfo(req);
    int code = fileChecking(scriptFile);
    if (code == 403)
    {
        responseErrorcgi(403, " Forbidden", ConfigPars, req);
        return ;
    }
    else if (code == 404)
    {
        responseErrorcgi(404, " not found", ConfigPars, req);
        return ;
    }
    if (!memoExt.empty())
    {
        if (checkLocationCgi(req, memoExt, "allow_cgi", ConfigPars) == -1)
            return ;
        memoExt = "";
    }
    int _status = executeCgiScript(req, ConfigPars);
    pid_1 = pid;
    if (_status == 1)
    {
        parseOutput();
        formatHttpResponse(outFile, req);
    }
}

std::string Cgi::generateListingDirCgi(Request &req)
{
    std::string pathRequest = req.GetHeaderValue("path");
    std::string _uri = req.GetFullPath();
    DIR *dirCheck = opendir(_uri.c_str());
    if (!dirCheck)
        return "<html><body><h1>Cannot open directory</h1></body></html>";

    std::string html = "<!DOCTYPE html>\n";
    html += "<html>\n<head>\n<title>Index of " + _uri + "</title>\n";
    html += "<style>\n"
            "body { background-color: #f0f4f8; font-family: Arial, sans-serif; color: #333; padding: 40px; }\n"
            "h1 { color: #2c3e50; }\n"
            "ul { list-style-type: none; padding-left: 0; }\n"
            "li { margin: 10px 0; }\n"
            "a { text-decoration: none; color: #3498db; font-weight: bold; }\n"
            "a:hover { text-decoration: underline; color: #1abc9c; }\n"
            ".container { max-width: 800px; margin: auto; background-color: #fff; padding: 30px; box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1); border-radius: 8px; }\n"
            "</style>\n"
            "</head>\n<body>\n<div class=\"container\">\n"
            "<h1>Index of " + _uri + "</h1>\n<ul>\n";

    struct dirent *dir;
    while ((dir = readdir(dirCheck)) != NULL)
    {
        std::string name = pathRequest + (pathRequest[pathRequest.length() - 1] == '/' ? "" : "/") + dir->d_name;
        html += "<li><a href=\"" + name + "\">" + dir->d_name + "</a></li>\n";
    }
    closedir(dirCheck);
    html += "</ul>\n</div>\n</body>\n</html>\n";
    return html;
}

bool Cgi::generateAutoIndexOnCgi(Request &req)
{
    statCgiFileBody = generateListingDirCgi(req);
    statCgiFilePos = 0;
    usingCgiStatFile = true;

    cgiHeader = "HTTP/1.1 200 OK\r\n";
    cgiHeader += "Content-Length: " + intToString(statCgiFileBody.size()) + "\r\n";
    cgiHeader += "Content-Type: text/html\r\n";
    if (req.GetHeaderValue("connection") == "keep-alive")
    {
        cgiHeader += "Connection: keep-alive\r\n\r\n";
        checkConnection = keepAlive;
    }
    else
    {
        cgiHeader += "Connection: close\r\n\r\n";
        checkConnection = _close;
    }

    cgiHeaderSent = 0;
    return true;
}

std::string getInfoConfigcgi(std::vector<ConfigNode> ConfigPars, std::string what, std::string location, Request &req)
{
    ConfigNode a = req.GetRightServer();

    std::vector<std::string> temp = a.getValuesForKey(a, what, location);
    if (!temp.empty())
        return temp[0];
	return "";
}

int    Cgi::servListingDirenCgi(Request &req, std::vector<ConfigNode> ConfigPars, std::string uri)
{
    std::string	 loc = req.GetRightServer().GetRightLocation(req.GetHeaderValue("path"));
    std::string autoIndexOn = getInfoConfigcgi(ConfigPars, "autoindex", loc, req);
    std::string index = getInfoConfigcgi(ConfigPars, "index", loc, req);

    if (!index.empty())
    {
        std::string htmlFound = uri;
        if (uri.back() != '/')
            htmlFound += "/";
        htmlFound += index;
        struct stat st;
        if (stat(htmlFound.c_str(), &st) == 0)
        {
            if (S_ISDIR(st.st_mode))
            {
                if (autoIndexOn == "on")
                    generateAutoIndexOnCgi(req);
                else
                    responseErrorcgi(403, " Forbidden", ConfigPars, req);
                return -1;
            }
            if (access(htmlFound.c_str(), R_OK) != 0)
            {
                if (autoIndexOn == "on")
                   generateAutoIndexOnCgi(req);
                else
                    responseErrorcgi(403, " Forbidden", ConfigPars, req);
                return -1;
            }
            cgiHeader = "";
            int checkCode = IsCgiRequest(htmlFound, req, ConfigPars);
            if (checkCode == 1)
            {
                req.SetFullSystemPath(htmlFound);
                handleCgiRequest(req, ConfigPars);
                req.SetFullSystemPath(uri);
                if (getcgistatus() == CGI_RUNNING)
                {
                    hasPendingCgi = true;
                    return -1;
                }
                hasPendingCgi = false;
                return -1;
            }
            else if (checkCode == 0)
                return 0;
        }
        else if (autoIndexOn == "on")
            generateAutoIndexOnCgi(req);
        else
            responseErrorcgi(403, " Forbidden", ConfigPars, req);
    }
    else if (autoIndexOn == "on")
        generateAutoIndexOnCgi(req);
    else
        responseErrorcgi(403, " Forbidden", ConfigPars, req);
    return -1;
}

int Cgi::IsCgiRequest(std::string uri, Request &req, std::vector<ConfigNode> ConfigPars)
{
    size_t index = uri.find("/cgiScripts/");
    if (index == std::string::npos)
    {
        if (uri.find("/cgiScripts") != std::string::npos)
        {
            if (servListingDirenCgi(req, ConfigPars, uri) == -1)
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
    if (scriptFile.find(".cgi") != std::string::npos || scriptFile.find(".py") != std::string::npos || scriptFile.find(".php") != std::string::npos)
    {
        return 1;
    }
    return 0;
}