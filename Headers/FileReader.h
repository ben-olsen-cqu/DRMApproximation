#pragma once

#include "os.h"
#include "Coordinates.h"
#include "Quadtree.h"

#include <filesystem>
#include <vector>
#include <string>

class FileReader
{
public:
	static std::vector<std::string> GetFileList(std::string path);
	static std::vector<Coordinates> ReadFiles(std::vector<std::string> files);
	static void GetMinMaxCSV(std::vector<std::string> files, MinMax& mm);
	static void ReadLine(std::ifstream* fs, double& x, double& y, double& z);
};

