#include "config.hpp"

ConfigNode::ConfigNode(){}

ConfigNode::ConfigNode(const ConfigNode & other) {*this = other;}

ConfigNode::~ConfigNode(){}

ConfigNode::ConfigNode(const std::string& name) : name(name) {}

ConfigNode& ConfigNode::operator=(const ConfigNode& other) {
    if (this != &other) {
        this->name = other.name;
        this->values = other.values;
        this->children = other.children;
    }
    return *this;
}

void ConfigNode::addValue(const std::string& key, const std::string& value) {values[key].push_back(value);}

void ConfigNode::addChild(const ConfigNode& child) {children.push_back(child);}

ConfigNode& ConfigNode::getLastChild() {return children.back();}

const std::vector<ConfigNode>& ConfigNode::getChildren() const {return children;}

const std::string& ConfigNode::getName() const {return name;}

const std::map<std::string, std::vector<std::string> >& ConfigNode::getValues() const {return values;}

const std::vector<std::string>* ConfigNode::getValuesForKey(const std::string& key) const
{
    std::map<std::string, std::vector<std::string> >::const_iterator it = values.find(key);
    if (it != values.end())
        return &(it->second);  // Return pointer to the vector of strings
    return NULL;  // Use NULL instead of nullptr (C++98 doesn't have nullptr)
}

void ConfigNode::PutName(const std::string &name) {this->name = name;}

//////////////////////////////////////////

std::vector<std::string> split(std::string text)
{
	std::istringstream stream(text);
    std::string word;
    std::vector<std::string> words;
    while (stream >> word)
        words.push_back(word);
	return words;
}

std::string removeSpaces(const std::string &input) {
	std::string result = "";
	for (size_t i = 0; i < input.length(); i++) {
		if (input[i] != ' ') {
			result += input[i];
		}
	}
	return result;
}

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

std::string RmComments(const std::string &buffer)
{
	std::string result;
	size_t start = 0;
	size_t end = buffer.find('\n');
	while (end != std::string::npos)
	{
		std::string line = buffer.substr(start, end - start);
		if (line.find('#') != std::string::npos)
			line.erase(line.find('#'));
		line = removeExtraSpaces(line);
		if (!line.empty() && line.find_first_not_of(' ') != std::string::npos)
			result += line + ' ';
		start = end + 1;
		end = buffer.find('\n', start);
	}
	std::string lastLine = buffer.substr(start);
	if (lastLine.find('#') != std::string::npos)
		lastLine.erase(lastLine.find('#'));
	lastLine = removeExtraSpaces(lastLine);
	if (!lastLine.empty() && lastLine.find_first_not_of(' ') != std::string::npos)
		result += lastLine;
	size_t endPos = result.find_last_not_of(' ');
	if (endPos != std::string::npos)
		result.erase(endPos + 1);
	return result;
}

void CheckStartServer(std::string text, size_t pos)
{
    std::vector<std::string> words = split(text);

    if ((pos == 0 && (words[0] != "server" || words.size() != 1)) || 
        (pos != 0 && (words.size() != 2 || words[0] != "location")))
        throw std::runtime_error("Error: invalid syntax.");
}

void AddKV(ConfigNode &ConfTree, const std::vector<std::string>& words)
{
    if (words.size() < 2)
        throw std::runtime_error("Error: Invalid key-value pair.");
    for (size_t i = 1; i < words.size(); i++)
        ConfTree.addValue(words[0], words[i]);
}

void checkContent(ConfigNode &ConfTree, const std::string& buffer)
{
    std::string delimiters = "{};";
    size_t pos = 0;
    std::string text;
    std::vector<ConfigNode*> nodeStack;
    nodeStack.push_back(&ConfTree);
    bool isRootNameSet = false;

    while (pos < buffer.size())
    {
        size_t delimiterPos = buffer.find_first_of(delimiters, pos);
        if (delimiterPos == std::string::npos)
            throw std::runtime_error("Error: Could not find any delimiters '{};'");
        text = buffer.substr(pos, delimiterPos - pos);
        std::string delimiter = buffer.substr(delimiterPos, 1);
        
        if (delimiter == "{")
        {
            CheckStartServer(text, pos);
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
        else if (delimiter == "}")
        {
            if (!nodeStack.empty())
                nodeStack.pop_back();
            else
                throw std::runtime_error("Error: Unmatched closing brace '}'.");
        }
        else if (delimiter == ";")
        {
            if (nodeStack.empty())
                throw std::runtime_error("Error: No active node for key-value pair.");
            std::vector<std::string> words = split(text);
            if (!words.empty())
                AddKV(*nodeStack.back(), words);
        }
        pos = delimiterPos + 1;
    }
    if (nodeStack.size() != 0)
        throw std::runtime_error("Error: Unmatched opening brace '{'.");
}


void StructConf(ConfigNode &ConfTree, std::string ConfigFilePath)
{
	std::ifstream infile(ConfigFilePath);
	if (!infile.is_open())
		throw std::runtime_error("Error: Could not open configuration file.");
	std::stringstream buffer;
	buffer << infile.rdbuf();
	infile.close();
	checkContent(ConfTree, RmComments(buffer.str()));
}

void ConfigNode::print() const {
    std::cout << name << " {" << std::endl;

    // Print key-value pairs
    for (std::map<std::string, std::vector<std::string> >::const_iterator it = values.begin(); it != values.end(); ++it) {
        std::cout << "  " << it->first;
        for (size_t i = 0; i < it->second.size(); i++) {
            std::cout << " " << it->second[i];
        }
        std::cout << ";" << std::endl;
    }

    // Print child nodes
    for (size_t i = 0; i < children.size(); i++) {
        std::cout << "  ";
        children[i].print();
    }

    std::cout << "}" << std::endl;
}
