#include "./config/pars_config/config.hpp"
#include "./config/tree_config/conftree.hpp"
#include <cstddef>
#include <vector>


int Printall = 0;

void GetContent(ConfTree &TreeConf, ConfigNode ParsConf)
{
	(void)TreeConf;
    // Iterate over the current node values first
    const std::map<std::string, std::vector<std::string> >& currentValues = ParsConf.getValues();
    std::map<std::string, std::vector<std::string> >::const_iterator it;
    std::cout << ParsConf.getName() << std::endl;
    for (it = currentValues.begin(); it != currentValues.end(); ++it) {
        std::cout << "  " << it->first; // Print the key
        size_t i;
        for (i = 0; i < it->second.size(); i++) {
            std::cout << " " << it->second[i]; // Print the values
        }
        std::cout << ";" << std::endl;
    }

    // // Now, loop through the children nodes of ParsConf recursively
    // const std::vector<ConfigNode>& children = ParsConf.getChildren();
    // size_t i;
    // for (i = 0; i < children.size(); ++i) {
	// 	std::cout << "----------------_>" << std::endl;
    //     GetContent(TreeConf, children[i]);
    // }
}


int main(int argc, char **argv)
{

	if (argc != 2)
	{
		std::cerr << "./webserv [configuration file]" << std::endl;
		return (1);
	}
	std::string ConfigFilePath = argv[1];
	ConfigNode ParsConf;
	ConfTree TreeConf;
	try
	{
		StructConf(ParsConf, ConfigFilePath);
		if(Printall)
			ParsConf.print();
		GetContent(TreeConf, ParsConf);

	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1; 
	}
	return 0; 
}