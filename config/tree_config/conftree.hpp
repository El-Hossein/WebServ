#ifndef CONFTREE_HPP
#define CONFTREE_HPP

#include "../../allincludes.hpp"

class ConfTree {
    public:
        ConfTree();
        ConfTree(const ConfTree& other);
        ConfTree& operator=(const ConfTree& other);
        ~ConfTree();

        // Port & Address
        int GetPort() const;
        void SetPort(int Port);
        std::string GetAddress() const;
        void SetAddress(std::string Address);

        // Server Name
        std::string GetServerName() const;
        void SetServerName(std::string ServerName);

        // Error Pages
        std::string GetErrorPage(int code) const;
        void SetErrorPage(int code, std::string path);

        // Client Max Body Size
        std::string GetClientMaxBodySize() const;
        void SetClientMaxBodySize(std::string size);

        // Root Directory
        std::string GetRoot() const;
        void SetRoot(std::string Root);

        // Index File
        std::string GetIndex() const;
        void SetIndex(std::string Index);

        // Autoindex (on/off)
        std::string GetAutoindex() const;
        void SetAutoindex(std::string Autoindex);

        // Redirect / Return
        int GetReturnCode() const;
        std::string GetReturnPath() const;
        void SetReturn(int code, std::string path);

        // Allowed Methods
        std::vector<std::string> GetAllowedMethods() const;
        void SetAllowedMethods(std::vector<std::string> methods);

        // Upload Store
        std::string GetUploadStore() const;
        void SetUploadStore(std::string path);

        // CGI Execution
        std::string GetPhpCgi() const;
        void SetPhpCgi(std::string path);
        std::string GetPyCgi() const;
        void SetPyCgi(std::string path);

    private:
        int Port;
        std::string Address;
        std::string ServerName;
        std::map<int, std::string> ErrorPages; // {404: "/errors/404.html", 500: "/errors/500.html"}
        std::string ClientMaxBodySize;
        std::string Root;
        std::string Index;
        std::string Autoindex;
        int ReturnCode;
        std::string ReturnPath;
        std::vector<std::string> AllowedMethods;
        std::string UploadStore;
        std::string PhpCgi;
        std::string PyCgi;
};


#endif