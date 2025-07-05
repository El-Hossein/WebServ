#include "Get.hpp"

Get::Get(const	Request	&_obj) : obj(_obj)
{
}

Get::~Get() {
}

void	Get::CheckPath(std::string path)
{
	struct stat fileStat;

	if (stat(path.c_str(), &fileStat) == -1)
		throw ("404 Not Found: file does not exist");

	if (!S_ISREG(fileStat.st_mode)) // Check wach path fih regular file containing Data
		throw ("403 Forbidden: resource is not a file");

	if (access(path.c_str(), R_OK | W_OK) == -1)
		throw ("403 Forbidden: file is not readable");
}