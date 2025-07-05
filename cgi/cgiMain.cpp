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

std::string modifyingPath(const char* uri)
{
    std::string path;

    path = std::string(uri);
    return path;
}

std::string handleCgiRequest(const Request &req, std::vector<ConfigNode> ConfigPars)
{
    const char *requestBody = "";

    // need to check which methode if its not get or post error.
    std::string scriptPath = modifyingPath(req.GetFullPath().c_str());
    int code = fileChecking(scriptPath);
    if (code == 403)
        return responseError(403, " Forbidden", ConfigPars);
    else if (code == 404)
        return responseError(404, " not found", ConfigPars);
    std::string scriptOutput = executeCgiScript(scriptPath.c_str(), req);
    if (scriptOutput.empty())
        return responseError(500, " internal server error", ConfigPars);

    CgiResponse parsedCgi = parseOutput(scriptOutput);
    return formatHttpResponse(parsedCgi);
}

int IsCgiRequest(const char *uri)
{
    //make a specific folder for CGI scripts
    const char *extension = strrchr(uri, '.');
    if (extension == NULL)
        return 0;
    if (extension && (strcmp(extension, ".cgi") == 0 || strcmp(extension, ".py") == 0) || strcmp(extension, ".php") == 0)
        return 1;
    return 0;
}

// int main()
// {
//     // if (IsCgiRequest("/cgiScripts/file.php") == 1)
//     // {
//     //     std::string getResponse = handleCgiRequest("GET", "/cgiScripts/file.php", "name=ismail");
//     //     std::cout << "GET Response:\n" << getResponse << std::endl;
        
//     //     // std::string post_response = handleCgiRequest("POST", "/cgiScripts/file.py", "name=ismail");
//     //     // std::cout << "POST Response:\n" << post_response << std::endl;
//     // }
//     // else
//     //     std::cout << "is NOT a CGI request" << std::endl;

    
//     return 0;
// }
