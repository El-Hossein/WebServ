#ifndef CONFIG_HPP
#define CONFIG_HPP


#include "../allincludes.hpp"


class ConfigNode {
	private:
		std::string name; // Name of the context (e.g., "http", "server", etc.)
		std::map<std::string, std::vector<std::string> > values; // Multi-value directives
		std::vector<ConfigNode> children;  // Nested contexts
	public:
		ConfigNode();
		ConfigNode(const std::string& name);
		~ConfigNode();
		
		void addValue(const std::string& key, const std::string& value);
		void addChild(const ConfigNode& child);
		ConfigNode& getLastChild();
		// const std::vector<ConfigNode>& getChildren() const;
		const std::string& getName() const;
		void PutName(const std::string& name);
		
		std::map<std::string, std::vector<std::string> >& getValues() ;

		std::vector<std::string>* getValuesForKey(const std::string& key)  ;
		// const std::map<std::string, std::vector<std::string> >& getValues() const {return values;}
		void print() const;
};


void StructConf(std::string ConfigFilePath, std::vector<ConfigNode> &ConfigPars);


#endif