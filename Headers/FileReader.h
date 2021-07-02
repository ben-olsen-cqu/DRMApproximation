#pragma once

#include "os.h"
#include "Coordinates.h"

#include <filesystem>
#include <vector>
#include <string>

class FileReader
{
public:
	static std::vector<std::string> GetFileList(std::string path);
	static std::vector<Coordinates> ReadFiles(std::vector<std::string> files);

};

