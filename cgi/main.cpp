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

    path = "." + std::string(uri);
    return path;
}

std::string handleCgiRequest(const char* method, const char* uri, const char* queryString)
{
    const char *requestBody = "";

    std::string scriptPath = modifyingPath(uri);
    int code = fileChecking(scriptPath);    
    if (code == 403)
        return responseError(403, " Forbidden");
    else if (code == 404)
        return responseError(404, " not found");

    std::string scriptOutput = executeCgiScript(scriptPath.c_str(), method, uri, queryString);
    if (scriptOutput.empty())
        return responseError(500, " internal server error");

    CgiResponse parsedCgi = parseOutput(scriptOutput);
    return formatHttpResponse(parsedCgi);
}

int IsCgiRequest(const char *uri)
{
    //make a specific folder for CGI scripts
    const char *extension = strrchr(uri, '.');
    if (extension && (strcmp(extension, ".cgi") == 0 || strcmp(extension, ".py") == 0))
        return 1;
    return 0;
}

int main()
{
    if (IsCgiRequest("/cgiScripts/file.cgi") == 1)
    {
        std::string getResponse = handleCgiRequest("GET", "/cgiScripts/file.cgi", "name=ismail");
        std::cout << "GET Response:\n" << getResponse << std::endl;
        
        // std::string post_response = handleCgiRequest("POST", "/cgiScripts/file.py", "name=ismail");
        // std::cout << "POST Response:\n" << post_response << std::endl;
    }
    else
        std::cout << "is NOT a CGI request" << std::endl;

    return 0;
}
