#include "Delete.hpp"

Delete::Delete(const	Request	&_obj) : obj(_obj)
{
}

Delete::~Delete() {
}

void	Delete::DoDelete(std::string path)
{
	struct stat fileStat;
	
	// std::cout << "Path recieved:{" << path << "" << std::endl;
	// if (stat(path.c_str(), &fileStat) == -1)
	// 	throw ("404 Not Found: file does not exist");
	
	// if (!S_ISREG(fileStat.st_mode)) // Check wach path fih regular file containing Data
	// 	throw ("403 Forbidden: resource is not a file");
	
	// if (access(path.c_str(), R_OK | W_OK) == -1)
	// 	throw ("403 Forbidden: file is not readable");

	// int ret = remove(path.c_str());
	// if (ret == -1) // Error occures while fullifing the request
	// 	throw "500 Internal Server Error";
	// if (ret == 0) // fulfilled the request + no need to return an entity-body
	// 	throw "204 No Content";
}