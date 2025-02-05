
#include "./pars_config/config.hpp"
#include <string>

int Printall = 0;
int main(int argc, char **argv)
{

	if (argc != 2)
	{
		std::cerr << "./webserv [configuration file]" << std::endl;
		return (1);
	}
	std::string ConfigFilePath = argv[1];
	ConfigNode ParsConf;
	try
	{
		StructConf(ParsConf, ConfigFilePath);
		if(Printall)
			ParsConf.print();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1; 
	}
	return 0; 
}