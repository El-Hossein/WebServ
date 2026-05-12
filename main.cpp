#include "./ConfigParsing/config.hpp"
#include "./AllServer/HttpServer.hpp"

int ConfigeFileFunc(std::string ConfigFilePath, std::vector<ConfigNode> &ConfigPars)
{
	try
	{
		if (ConfigFilePath.size() < 6 ||  ConfigFilePath.substr(ConfigFilePath.length() - 5) != ".conf")
	        throw ("Error: Config file does not have the correct extension. {.conf}");
		StructConf(ConfigFilePath, ConfigPars);
	}
	catch (char const *e)
	{
		std::cerr << e << std::endl;
		return 1;
	}
	catch (const  std::string e)
	{
		std::cerr << e << std::endl;
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
	catch (char const *e)
	{
		std::cerr << e << std::endl;
		return 1;
	}
	catch (const  std::string e)
	{
		std::cerr << e << std::endl;
		return 1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	if (argc > 2)
		return std::cerr << "./webserv [configuration file]" << std::endl, 1;

	std::string ConfigFilePath;
	signal(SIGPIPE, SIG_IGN);

	ConfigFilePath = (argc == 2) ? argv[1] : "./DefaultConfig/default.conf"; // use default config if not configuration provided

	std::vector<ConfigNode> ConfigPars;
	ConfigNode obj;
	if (ConfigeFileFunc(ConfigFilePath, ConfigPars) == 1)
		return 1;
	if (StartServerFunc(ConfigPars) == 1)
		return 1;

	return 0;
}