#include "./config/pars_config/config.hpp"
#include "./config/tree_config/conftree.hpp"
#include <vector>


int Printall = 0;

void GetContent(ConfTree &TreeConf, std::vector<ConfigNode> ConfigPars)
{
    (void)TreeConf;
    (void)ConfigPars;
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
	        throw std::runtime_error("Error: Config file does not have the correct extension. {.conf}");
		StructConf(ConfigFilePath, ConfigPars);
		if(Printall == 0)
		{
			std::cout << "--------------------------------------------------------------------|" << std::endl;
			std::cout << "--------------------------------------------------------------------|" << std::endl;
			for (size_t i = 0; i < ConfigPars.size(); i++)
			{
				ConfigPars[i].print();
				std::cout << "--------------------------------------------------------------------|" << std::endl;
				std::cout << "--------------------------------------------------------------------|" << std::endl;
			}
		}
		GetContent(TreeConf, ConfigPars);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1; 
	}
	return 0; 
}


