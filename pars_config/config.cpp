#include "config.hpp"
#include <cstddef>
#include <string>
#include <vector>

ConfigNode::ConfigNode(){}

std::vector<std::string> split(const std::string& text) {
    std::istringstream stream(text);
    std::string word;
    std::vector<std::string> words;
    while (stream >> word) words.push_back(word);
    return words;
}

ConfigNode::~ConfigNode(){}

ConfigNode::ConfigNode(const std::string& name) : name(name) {}

void ConfigNode::addValue(const std::string& key, const std::string& value) {values[key].push_back(value);}

void ConfigNode::addChild(const ConfigNode& child) {children.push_back(child);}

ConfigNode& ConfigNode::getLastChild() {return children.back();}

const std::vector<ConfigNode>& ConfigNode::getChildren() const {return children;}

const std::string& ConfigNode::getName() const {return name;}

std::map<std::string, std::vector<std::string> >& ConfigNode::getValues()  {return values;}

std::string ConfigNode::GetLocationValue(ConfigNode& ConfNode, const std::string& key, size_t index) 
{
    if (index > ConfNode.children.size())
        return "";
    else
        return ConfNode.children[index].getName();
}

std::vector<std::string> ConfigNode::   getValuesForKey(ConfigNode& ConfNode, const std::string& key, std::string del) 
{
    std::vector<std::string> emptyResult;
    std::vector<std::string> arr ;
    //loop through the confnode check if the key is present
    if (del == "NULL")
    {
        std::map<std::string, std::vector<std::string> > keys = ConfNode.getValues();
        for (std::map<std::string, std::vector<std::string> >::const_iterator it = keys.begin(); it != keys.end(); ++it)
        {
            // std::cout << "keys: " << it->first << std::endl;
            if (it->first == key)
                return it->second;
        }
    }
    else {
        for (size_t i = 0; i < ConfNode.children.size(); i++)
        {
            arr = split(ConfNode.children[i].getName());
            if (arr[1] == del)
            {
                std::vector<std::string> a = getValuesForKey(ConfNode.children[i], key, "NULL");
                if (!a.empty())
                    return a;
                else
                    return getValuesForKey(ConfNode, key, "NULL");
            }
        }
    }
    return emptyResult;
}
std::vector<std::string> ConfigNode::   ConfgetValuesForKey(ConfigNode& ConfNode, const std::string& key, std::string del) 
{
    std::vector<std::string> emptyResult;
    std::vector<std::string> arr ;
    //loop through the confnode check if the key is present
    if (del == "NULL")
    {
        std::map<std::string, std::vector<std::string> > keys = ConfNode.getValues();
        for (std::map<std::string, std::vector<std::string> >::const_iterator it = keys.begin(); it != keys.end(); ++it)
        {
            // std::cout << "keys: " << it->first << std::endl;
            if (it->first == key)
                return it->second;
        }
    }
    else {
        for (size_t i = 0; i < ConfNode.children.size(); i++)
        {
            arr = split(ConfNode.children[i].getName());
            if (arr[1] == del)
                return ConfgetValuesForKey(ConfNode.children[i], key, "NULL");
        }
    }
    return emptyResult;
}

void ConfigNode::PutName(const std::string &name) {this->name = name;}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// trim spaces from the line
void trimSpaces(std::string &str)
{
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");

    if (first == std::string::npos)
    {
        str.clear();
        return;
    }

    str = str.substr(first, last - first + 1);
}

// split the line into words

// remove spaces from the line
std::string removeSpaces( std::string &input)
{
	std::string result = "";
	for (size_t i = 0; i < input.length(); i++)
		if (input[i] != ' ') result += input[i];
	return result;
}
// remove extra spaces from the line
std::string removeExtraSpaces(std::string line)
{
	std::stringstream result;
	std::string word;

	std::stringstream ss(line);
	while (ss >> word)
	{
		if (!result.str().empty()) result << " ";
		result << word;
	}

	return result.str();
}

// remove comments from the buffer
std::string RmComments(std::string buffer) {
    std::string result;
    std::istringstream iss(buffer);
    std::string line;
    while (std::getline(iss, line)) {
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) line.erase(commentPos);
        line = removeExtraSpaces(line);
        if (!line.empty()) result += line + " ";
    }
    size_t endPos = result.find_last_not_of(' ');
    if (endPos != std::string::npos) result.erase(endPos + 1);
    return result;
}

void CheckStartServer(std::vector<ConfigNode*> &nodeStack, std::string text, size_t pos)
{
    std::vector<std::string> words = split(text);
    if (pos == 0)
    {
        if (words.size() != 1) throw std::runtime_error("Error: Expected exactly one word in server.");
        if (words[0] != "server") throw std::runtime_error("Error: The first word must be 'server'.");
    }
    else
    {
        std::string name = nodeStack.back()->getName();
        std::vector<std::string> checkwords = split(name);
        if (checkwords[0] == "location") throw std::runtime_error("Error: Location block cannot be nested in another location block.");
        if (words.size() != 2) throw std::runtime_error("Error: Expected exactly two words in location");
        if (words[0] != "location") throw std::runtime_error("Error: The first word must be 'location'.");
    }
}

void checkAliasRoot(const std::string& key, ConfigNode& ConfNode)
{
    if (key == "alias")
    {
        std::vector<std::string> root = ConfNode.ConfgetValuesForKey(ConfNode, "root", "NULL");
        if (!root.empty())
            throw std::runtime_error("Error: Alias and Root cant be in the same location");
        return;
    }
    if (key == "root")
    {
        std::vector<std::string> alias = ConfNode.ConfgetValuesForKey(ConfNode, "alias", "NULL");
        if (!alias.empty())
            throw std::runtime_error("Error: Alias and Root cant be in the same location");
    }
}

void CheckAllError(std::vector<std::string>& KV, const std::string& key, ConfigNode& ConfNode, int max, int mult) {
    int count = 0;
    (void)ConfNode;
    count = KV.size() - 1;
    if (key != KV[0]) return;
    
    if (key == "alias" || key == "root")
        checkAliasRoot(key, ConfNode);
    if (key == "allow_methods")
    {
        int CheckNono = 0;
        int CheckOthers = 0;
        for (std::vector<std::string>::iterator it = KV.begin(); it != KV.end(); ++it) {
            if (*it == "GET" || *it == "POST" || *it == "DELETE")
                CheckOthers = 1;
            else if (*it == "NONE")
                CheckNono = 1;
        }
        if (CheckNono == 1 && CheckOthers == 1)
            throw std::runtime_error("Error: Cant be NONE and other Methods in the same location in allow_methods");
    }
    std::vector<std::string> helo = ConfNode.ConfgetValuesForKey(ConfNode, key, "NULL");
    if (!helo.empty())
    {
        if (max != -1 && (int)(helo.size() + count) > max) throw std::runtime_error("Error: Too many values for key '" + key + "'. Maximum allowed is " + std::to_string(max) + ".");
        if (mult != -1 && (helo.size() + count) % mult != 0) throw std::runtime_error("Error: Number of values for key '" + key + "' must be a multiple of " + std::to_string(mult) + ".");
    }
    else
    {
        if (max != -1 && (int)(count) > max) throw std::runtime_error("Error: Too many values for key '" + key + "'. Maximum allowed is " + std::to_string(max) + ".");
        if (mult != -1 && (count) % mult != 0) throw std::runtime_error("Error: Number of values for key '" + key + "' must be a multiple of " + std::to_string(mult) + ".");
    }
}

void ErrorHandle(std::vector<std::string>& KV, ConfigNode &ConfNode, std::string blockType)
{
    if (blockType == "server")
    {
        CheckAllError(KV, "listen", ConfNode, -1, 1);
        CheckAllError(KV, "server_names", ConfNode, -1, 1);
        CheckAllError(KV, "error_page", ConfNode, -1, 2);
        CheckAllError(KV, "client_max_body_size", ConfNode, 1, 1);
        CheckAllError(KV, "root", ConfNode, 1, 1);
        CheckAllError(KV, "index", ConfNode, 1, -1);
        CheckAllError(KV, "autoindex", ConfNode, 1, 1);
        CheckAllError(KV, "return", ConfNode, 2, 2);
    }
    else {
        CheckAllError(KV, "allow_methods", ConfNode, 3, 1);
        CheckAllError(KV, "autoindex", ConfNode, 1, 1);
        CheckAllError(KV, "return", ConfNode, 2, 2);
        CheckAllError(KV, "root", ConfNode, 1, 1);
        CheckAllError(KV, "alias", ConfNode, 1, 1);
        CheckAllError(KV, "index", ConfNode, 1, -1);
        CheckAllError(KV, "upload_store", ConfNode, 1, 1);
        CheckAllError(KV, "allow_cgi", ConfNode, 3, 1);
    }
}

// check if the key is allowed in the block
void AllowedIn(std::vector<std::string> VALID_KEYS, std::vector<std::string>& words, ConfigNode &ConfNode, const std::string& blockType)
{
    std::vector<std::string>::const_iterator it;
    for (it = VALID_KEYS.begin(); it != VALID_KEYS.end(); ++it)
        if (*it == words[0]) 
            break;
    if (it == VALID_KEYS.end())
        throw std::runtime_error("Error: Invalid key '" + words[0] + "' for " + blockType + " block");
    ErrorHandle(words, ConfNode, blockType);
}

void MaxBodySizeToBytes(std::vector<std::string>& words) {
    if (words.empty() || words.size() < 2 || words[1].empty()) {
        words[1] = "0";
        return;
    }

    std::string value = words[1];
    char unit = value[value.length() - 1];
    size_t number = 0;

    std::string num_str = value.substr(0, value.length() - 1);
    std::stringstream ss(num_str);
    ss >> number;
    if (ss.fail()) {
        words[1] = "0";
        return;
    }

    switch (unit) {
        case 'B':
            break;
        case 'K':
            number *= 1024;
            break;
        case 'M':
            number *= 1024 * 1024;
            break;
        case 'G':
            number *= 1024 * 1024 * 1024;
            break;
        default:
            words[1] = "0";
            return;
    }

    std::stringstream result;
    result << number;
    words[1] = result.str();
}
// add key-value pair to the node
void AddKV(ConfigNode &ConfNode, std::vector<std::string>& words)
{
    static bool initialized = false;

    static std::vector<std::string> SERVER_VALID_KEYS;
    static std::vector<std::string> LOCATION_VALID_KEYS;

    if (!initialized)
    {
        SERVER_VALID_KEYS.push_back("listen");
        SERVER_VALID_KEYS.push_back("server_names");
        SERVER_VALID_KEYS.push_back("error_page");
        SERVER_VALID_KEYS.push_back("client_max_body_size");
        SERVER_VALID_KEYS.push_back("root");
        SERVER_VALID_KEYS.push_back("index");
        SERVER_VALID_KEYS.push_back("autoindex");
        SERVER_VALID_KEYS.push_back("return");

        LOCATION_VALID_KEYS.push_back("autoindex");
        LOCATION_VALID_KEYS.push_back("allow_methods");
        LOCATION_VALID_KEYS.push_back("return");
        LOCATION_VALID_KEYS.push_back("root");
        LOCATION_VALID_KEYS.push_back("index");
        LOCATION_VALID_KEYS.push_back("allow_cgi");
        LOCATION_VALID_KEYS.push_back("upload_store");
        LOCATION_VALID_KEYS.push_back("alias");

        initialized = true;
    }

    if (words.size() < 2)
        throw std::runtime_error("Error: Invalid key-value pair.");

    std::string nodeName = ConfNode.getName();
    trimSpaces(nodeName);
    std::vector<std::string> locations = split(nodeName);

    if (locations[0] == "server")
        AllowedIn(SERVER_VALID_KEYS, words, ConfNode, locations[0]);
    else if (locations[0] == "location")
        AllowedIn(LOCATION_VALID_KEYS, words, ConfNode, locations[0]);
    else
        throw std::runtime_error("Error: Unknown block type in configuration.");
    if (words[0] == "client_max_body_size")
    {
        MaxBodySizeToBytes(words);
        // std::cout << "test: " << words[1] << std::endl;
    }
        
        
    for (size_t i = 1; i < words.size(); ++i)
        ConfNode.addValue(words[0], words[i]);
}

// process the closing brace
void processClosingBrace(std::vector<ConfigNode*> &nodeStack)
{
    if (!nodeStack.empty())
        nodeStack.pop_back();
    else
        throw std::runtime_error("Error: Unmatched closing brace '}'.");
}

// process the semicolon
void processSemicolon(std::string &text, std::vector<ConfigNode*> &nodeStack)
{
    if (nodeStack.empty())
        throw std::runtime_error("Error: No active node for key-value pair.");
    std::vector<std::string> words = split(text);
    if (!words.empty())
        AddKV(*nodeStack.back(), words);
}

// process the opening brace
void processOpeningBrace(std::string &text, std::vector<ConfigNode*> &nodeStack, bool &isRootNameSet, size_t pos)
{
    CheckStartServer(nodeStack, text, pos);
    trimSpaces(text);
    if (nodeStack.size() == 1 && !isRootNameSet)
    {
        nodeStack.back()->PutName(text);
        isRootNameSet = true;
    }
    else
    {
        nodeStack.back()->addChild(ConfigNode(text));
        ConfigNode* childPtr = &(nodeStack.back()->getLastChild());
        nodeStack.push_back(childPtr);
    }
}

// check the content of the buffer
void checkContent(std::string buffer, std::vector<ConfigNode> &ConfigPars)
{
    std::string delimiters = "{};";
    size_t pos = 0;
    std::string text;
    ConfigNode ConfNode;
    std::vector<ConfigNode*> nodeStack;
    // nodeStack.push_back(&ConfNode);
    bool isRootNameSet = false;
    if (buffer.size() == 0) throw std::runtime_error("Error: Empty configuration file.");
    while (pos < buffer.size())
    {
        size_t delimiterPos = buffer.find_first_of(delimiters, pos);
        if (delimiterPos == std::string::npos) throw std::runtime_error("Error: Could not find any delimiters '{};'");
        text = buffer.substr(pos, delimiterPos - pos);
        std::string delimiter = buffer.substr(delimiterPos, 1);

        if (delimiter == "{")
        {
            if (nodeStack.empty())
            {
                ConfigPars.push_back(ConfNode);
                ConfNode = ConfigNode();
                nodeStack.push_back(&ConfNode);
                buffer = buffer.substr(pos);
                pos = 0;
                delimiterPos = 0;
                isRootNameSet = false;
                continue;
            }
            processOpeningBrace(text, nodeStack, isRootNameSet, pos);
        }
        else if (delimiter == "}") processClosingBrace(nodeStack);
        else if (delimiter == ";") processSemicolon(text, nodeStack);
        pos = delimiterPos + 1;
    }
    ConfigPars.push_back(ConfNode);
    if (nodeStack.size() != 0) throw std::runtime_error("Error: Unmatched opening brace '{'.");
}

// Parse the configuration file
void StructConf(std::string ConfigFilePath, std::vector<ConfigNode> &ConfigPars)
{
	std::ifstream infile(ConfigFilePath);
	if (!infile.is_open())
		throw std::runtime_error("Error: Could not open configuration file.");
	std::stringstream buffer;
	buffer << infile.rdbuf();
	infile.close();
	checkContent(RmComments(buffer.str()), ConfigPars);
    if (!ConfigPars.empty()) ConfigPars.erase(ConfigPars.begin());
}

void ConfigNode::print() const {
    std::cout << "[++" << name << "]" << std::endl;

    for (std::map<std::string, std::vector<std::string> >::const_iterator it = values.begin(); it != values.end(); ++it)
    {
        std::cout << "    [---" << it->first << "]";
        for (size_t i = 0; i < it->second.size(); i++)
        {
            std::cout <<  " [" << it->second[i] << "]";
        }
        std::cout << ";" << std::endl;
    }

    for (size_t i = 0; i < children.size(); i++)
    {
        std::cout << "  ";
        children[i].print();
    }
}

ConfigNode GetTheServer(std::vector<ConfigNode> ConfigPars, std::string ReqPortHost, std::string PortOrHostInConfig, std::string port)
{
    for (size_t i = 0; i < ConfigPars.size(); i++)
    {
        std::vector<std::string> ValuesOfPortOrHost = ConfigNode::ConfgetValuesForKey(ConfigPars[i], PortOrHostInConfig, "NULL");
        if (!ValuesOfPortOrHost.empty())
        {
            for (std::vector<std::string>::iterator it = ValuesOfPortOrHost.begin(); it != ValuesOfPortOrHost.end(); ++it)
            {
                if (*it == ReqPortHost)
                    return ConfigPars[i];
            }
        }
    }
	if (PortOrHostInConfig == "listen")
    	return ConfigPars[0];		
    return GetTheServer(ConfigPars, port, "listen", port);
}

ConfigNode ConfigNode::GetServer(std::vector<ConfigNode> ConfigPars, _ServerDetails ServerDetails)
{
    std::vector<std::vector<std::string> > arr;
    std::string port;
    std::string host;
    host = ServerDetails.ServerHost.substr(0, ServerDetails.ServerHost.find(":"));
    if (ServerDetails.IsPortExist == true)
        port = ServerDetails.ServerPort.substr(ServerDetails.ServerPort.find(":") + 1);
    else 
        port = std::to_string(ServerDetails.RealPort);
    // std::cout << "Host: " << host << std::endl;
    // std::cout << "port: " << port << std::endl;
    return  GetTheServer(ConfigPars, host, "server_names", port);
    
}

std::vector<std::string> split_path(const std::string& path)
{
    std::vector<std::string> components;
    std::string current;
    bool in_component = false;
    if (path.size() == 1)
    {
        if (path[0] == '/')
        {
            components.push_back("/");
            return components;
        }
    }
    for (size_t i = 0; i < path.length(); ++i)
    {
        if (path[i] == '/')
        {
            if (in_component && !current.empty())
            {
                components.push_back("/" + current);
                current.clear();
                in_component = false;
            }
        } else
        {
            current += path[i];
            in_component = true;
        }
    }
    if (in_component && !current.empty())
        components.push_back("/" + current);
    return components;
}

bool startsWith(const std::string& path, const std::string& prefix)
{
    if (prefix.length() > path.length()) return false;
    return path.compare(0, prefix.length(), prefix) == 0;
}

std::string RemoveSlashs(std::string path)
{
    std::string result;
    bool lastWasSlash = false;
    
    result.reserve(path.length());
    
    for (std::string::const_iterator it = path.begin(); it != path.end(); ++it)
    {
        if (*it == '/')
        {
            if (!lastWasSlash)
            {
                result += *it;
                lastWasSlash = true;
            }
        }
        else
        {
            result += *it;
            lastWasSlash = false;
        }
    }
    return result;
}

std::string ConfigNode::GetRightLocation(std::string path)
{
    std::vector<ConfigNode> locations = getChildren();

    std::string RightLocation;
    for (std::vector<ConfigNode>::iterator Alllocations = locations.begin(); Alllocations != locations.end(); ++Alllocations)
    {
        std::vector<std::string> tokens = split(Alllocations->name.c_str());
        std::string loc = tokens[1];
        std::vector<std::string> SplitPath = split_path(RemoveSlashs(loc));
        for (std::vector<std::string>::iterator SplitLocPath = SplitPath.begin(); SplitLocPath != SplitPath.end(); ++SplitLocPath)
        {
            // std::cout << "path: [" << *SplitLocPath << "]" << std::endl;
            if (startsWith(RemoveSlashs(path), *SplitLocPath) == true)
                RightLocation = loc;
        }
    }
    return RightLocation;
}
