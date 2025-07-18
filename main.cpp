#include "./pars_config/config.hpp"
#include "./AllServer/HttpServer.hpp"



int Printall = 0;


int ConfigeFileFunc(std::string ConfigFilePath, std::vector<ConfigNode> &ConfigPars)
{
	try
	{
		if (ConfigFilePath.substr(ConfigFilePath.length() - 5) != ".conf")
	        throw std::runtime_error("Error: Config file does not have the correct extension. {.conf}");
		StructConf(ConfigFilePath, ConfigPars);
		if(Printall == 1)
		{
			std::cout << "--------------------------------------------------------------------|" << std::endl;
			for (size_t i = 0; i < ConfigPars.size(); i++)
			{
				ConfigPars[i].print();
				std::cout << "--------------------------------------------------------------------|" << std::endl;
			}
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0; 
}

int StartServerFunc(std::vector<ConfigNode> ConfigPars)
{
	(void)ConfigPars;
	try
	{
		HttpServer server;
		server.setup_server(ConfigPars);
		server.run(ConfigPars);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "./webserv [configuration file]" << std::endl;
		return (1);
	}
	signal(SIGPIPE, SIG_IGN);
	std::string ConfigFilePath = argv[1];
	std::vector<ConfigNode> ConfigPars;
	ConfigNode obj;
	if (ConfigeFileFunc(ConfigFilePath, ConfigPars) == 1)
		return 1;
	if (StartServerFunc(ConfigPars) == 1)
		return 1;
	return 0;
}


