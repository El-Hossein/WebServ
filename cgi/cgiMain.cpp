#include "cgiHeader.hpp"


std::string intToString(int n)
{
    char buffer[32];
    sprintf(buffer, "%d", n);
    return std::string(buffer);
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

std::string handleCgiRequest(const Request &req, std::vector<ConfigNode> ConfigPars)
{
    const char *requestBody = "";

    // need to check which methode if its not get or post error.
    std::string scriptPath = req.GetFullPath();
    int code = fileChecking(scriptPath);
    if (code == 403)
        return responseError(403, " Forbidden", ConfigPars);
    else if (code == 404)
        return responseError(404, " not found", ConfigPars);
    std::string scriptOutput = executeCgiScript(scriptPath.c_str(), req, ConfigPars);
    if (scriptOutput.empty())
        return responseError(500, " internal server error", ConfigPars);
    CgiResponse parsedCgi = parseOutput(scriptOutput);
    return formatHttpResponse(parsedCgi);
}

int IsCgiRequest(const char *uri)
{
    //make a specific folder for CGI scripts
    std::string uriString(uri);
    size_t index = uriString.find("/cgiScripts/");
    if (index == 0)
        return 0;
    std::string pathAfterCgi = uriString.substr(index + 12);
    if (pathAfterCgi.empty())
        return 0;
    size_t firstSlash = pathAfterCgi.find("/");
    std::string scriptFile;
    if (firstSlash != std::string::npos)
    {
        scriptFile = pathAfterCgi.substr(0, firstSlash);
    }
    // need to complete here
    
    std::cout << "hnaaa" << std::endl;

    exit(1);
    return 0;
}

    // const char *extension = std::strrchr(uri, '.');
    // if (extension == NULL)
    //     return 0;
    // if (extension && (std::strcmp(extension, ".cgi") == 0 || std::strcmp(extension, ".py") == 0) || std::strcmp(extension, ".php") == 0)
    // {
        
    //     return 1;
    // }