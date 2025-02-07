#include "./config/pars_config/config.hpp"
#include "./config/tree_config/conftree.hpp"
#include <vector>


int Printall = 1;

void GetContent(ConfTree &TreeConf, ConfigNode ParsConf)
{
    (void)TreeConf;
    (void)ParsConf;
}


int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "./webserv [configuration file]" << std::endl;
		return (1);
	}
	std::string ConfigFilePath = argv[1];
	std::vector<ConfigNode> ConfigPars;
	ConfTree TreeConf;
	try
	{
		if (ConfigFilePath.substr(ConfigFilePath.length() - 5) != ".conf")
	        throw std::runtime_error("Error: Config file does not have the correct extension.");
		StructConf(ConfigFilePath, ConfigPars);
		if(Printall)
			for (size_t i = 0; i < ConfigPars.size(); i++)
			{
				ConfigPars[i].print();
				std::cout << "--------------------------------------------------------------------|" << std::endl;
				std::cout << "--------------------------------------------------------------------|" << std::endl;
			}
		// GetContent(TreeConf, ParsConf);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1; 
	}
	return 0; 
}




