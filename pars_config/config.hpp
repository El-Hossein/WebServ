#ifndef CONFIG_HPP
#define CONFIG_HPP


#include "../allincludes.hpp"

//  server === > "listen", "server_names", "error_page", "client_max_body_size", "root", "index", "autoindex", "return"
//  location =====> "autoindex", "allow_methods", "return", "php-cgi", "root", "index", "py-cgi", "upload_store"
class ConfigNode {
	private:
		std::vector<ConfigNode> ConfigPars;
		std::string name; // Name of the context ("http", "server", etc.)
		std::map<std::string, std::vector<std::string> > values; // Multi-value directives
		std::vector<ConfigNode> children;  // Nested contexts
	public:
		ConfigNode();
		ConfigNode(const std::string& name);
		~ConfigNode();
		
		void addValue(const std::string& key, const std::string& value);
		void addChild(const ConfigNode& child);
		ConfigNode& getLastChild();
		const std::vector<ConfigNode>& getChildren() const;
		const std::string& getName() const;
		void PutName(const std::string& name);
		
		std::map<std::string, std::vector<std::string> >& getValues() ;
		static ConfigNode GetServer(std::vector<ConfigNode> ConfigPars, _ServerDetails ServerDetails);
		static std::vector<std::string> getValuesForKey(ConfigNode& ConfNode, const std::string& key, std::string del)  ;
		static std::vector<std::string> ConfgetValuesForKey(ConfigNode& ConfNode, const std::string& key)  ;
		static std::string GetLocationValue(ConfigNode& ConfNode, size_t index);
		std::string GetRightLocation(std::string path);
		// const std::map<std::string, std::vector<std::string> >& getValues() const {return values;}
		void print() const;
};


void StructConf(std::string ConfigFilePath, std::vector<ConfigNode> &ConfigPars);


#endif