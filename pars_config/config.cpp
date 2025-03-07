#include "config.hpp"
#include <cstddef>
#include <string>
#include <vector>

ConfigNode::ConfigNode(){}


ConfigNode::~ConfigNode(){}

ConfigNode::ConfigNode(const std::string& name) : name(name) {}

void ConfigNode::addValue(const std::string& key, const std::string& value) {values[key].push_back(value);}

void ConfigNode::addChild(const ConfigNode& child) {children.push_back(child);}

ConfigNode& ConfigNode::getLastChild() {return children.back();}

// const std::vector<ConfigNode>& ConfigNode::getChildren() const {return children;}

const std::string& ConfigNode::getName() const {return name;}

std::map<std::string, std::vector<std::string> >& ConfigNode::getValues()  {return values;}

std::vector<std::string>* ConfigNode::getValuesForKey(ConfigNode& ConfNode, const std::string& key) 
{
    //loop through the confnode check if the key is present
    std::map<std::string, std::vector<std::string> >::iterator it = ConfNode.getValues().find(key);
    if (it != ConfNode.getValues().end())
        return &it->second;
    // std::map<std::string, std::vector<std::string> >::iterator it = values.find(key);

    return NULL;
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
std::vector<std::string> split(std::string text)
{
	std::istringstream stream(text);
    std::string word;
    std::vector<std::string> words;
    while (stream >> word)
        words.push_back(word);
	return words;
}

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
std::string RmComments( std::string buffer)
{
	std::string result;
	size_t start = 0;
	size_t end = buffer.find('\n');
	while (end != std::string::npos)
	{
		std::string line = buffer.substr(start, end - start);
		if (line.find('#') != std::string::npos) line.erase(line.find('#'));
		line = removeExtraSpaces(line);
		if (!line.empty() && line.find_first_not_of(' ') != std::string::npos) result += line + ' ';
		start = end + 1;
		end = buffer.find('\n', start);
	}
	std::string lastLine = buffer.substr(start);
	if (lastLine.find('#') != std::string::npos) lastLine.erase(lastLine.find('#'));
	lastLine = removeExtraSpaces(lastLine);
	if (!lastLine.empty() && lastLine.find_first_not_of(' ') != std::string::npos) result += lastLine;
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

void CheckAllError(const std::vector<std::string>& KV, const std::string& key, ConfigNode& ConfNode, int max, int mult) {
    int count = 0;
    (void)ConfNode;
    count = KV.size() - 1;
    if (key != KV[0]) return;
    
    
    std::vector<std::string>* helo = ConfNode.getValuesForKey(ConfNode, key);
    if (helo != NULL)
    {
        if (max != -1 && (int)(helo->size() + count) > max) throw std::runtime_error("Error: Too many values for key '" + key + "'. Maximum allowed is " + std::to_string(max) + ".");
        if (mult != -1 && (helo->size() + count) % mult != 0) throw std::runtime_error("Error: Number of values for key '" + key + "' must be a multiple of " + std::to_string(mult) + ".");
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
        CheckAllError(KV, "server_name", ConfNode, -1, 1);
        CheckAllError(KV, "error_page", ConfNode, -1, 2);
        CheckAllError(KV, "client_max_body_size", ConfNode, 1, 1);
        CheckAllError(KV, "root", ConfNode, 1, 1);
        CheckAllError(KV, "index", ConfNode, -1, -1);
        CheckAllError(KV, "autoindex", ConfNode, 1, 1);
        CheckAllError(KV, "return", ConfNode, -1, 2);
    }
    else {
        CheckAllError(KV, "allow_methods", ConfNode, 3, 1);
        CheckAllError(KV, "autoindex", ConfNode, 1, 1);
        CheckAllError(KV, "return", ConfNode, -1, 2);
        CheckAllError(KV, "php-cgi", ConfNode, 1, 1);
        CheckAllError(KV, "root", ConfNode, 1, 1);
        CheckAllError(KV, "index", ConfNode, -1, -1);
        CheckAllError(KV, "py-cgi", ConfNode, 1, 1);
        CheckAllError(KV, "upload_store", ConfNode, 1, 1);
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

// add key-value pair to the node
void AddKV(ConfigNode &ConfNode, std::vector<std::string>& words)
{
    static bool initialized = false;

    static std::vector<std::string> SERVER_VALID_KEYS;
    static std::vector<std::string> LOCATION_VALID_KEYS;

    if (!initialized)
    {
        SERVER_VALID_KEYS.push_back("listen");
        SERVER_VALID_KEYS.push_back("server_name");
        SERVER_VALID_KEYS.push_back("error_page");
        SERVER_VALID_KEYS.push_back("client_max_body_size");
        SERVER_VALID_KEYS.push_back("root");
        SERVER_VALID_KEYS.push_back("index");
        SERVER_VALID_KEYS.push_back("autoindex");
        SERVER_VALID_KEYS.push_back("return");

        LOCATION_VALID_KEYS.push_back("autoindex");
        LOCATION_VALID_KEYS.push_back("allow_methods");
        LOCATION_VALID_KEYS.push_back("return");
        LOCATION_VALID_KEYS.push_back("php-cgi");
        LOCATION_VALID_KEYS.push_back("root");
        LOCATION_VALID_KEYS.push_back("index");
        LOCATION_VALID_KEYS.push_back("py-cgi");
        LOCATION_VALID_KEYS.push_back("upload_store");

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
    nodeStack.push_back(&ConfNode);
    bool isRootNameSet = false;

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
}

void ConfigNode::print() const {
    std::cout << "[" << name << "]" << std::endl;

    for (std::map<std::string, std::vector<std::string> >::const_iterator it = values.begin(); it != values.end(); ++it)
    {
        std::cout << "    [" << it->first << "]";
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
