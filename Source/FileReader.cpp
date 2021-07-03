#include "../Headers/FileReader.h"
#include <fstream>
#include <filesystem>

#if defined(WINDOWS)  
using namespace std::filesystem;
#elif defined(MAC) || defined(LINUX)
using namespace std::__fs::filesystem;
#endif

std::vector<std::string> FileReader::GetFileList(std::string path)
{
    std::vector<std::string> files;

    for (const auto& entry : directory_iterator(path))
    {
        if (entry.is_regular_file())
        {
            std::string s = entry.path().string();

            if (s.substr(s.length() - 3, 3) == "csv")
            {
                files.push_back(entry.path().string());
            }
        }
    }

    return files;
}

std::vector<Coordinates> FileReader::ReadFiles(std::vector<std::string> files)
{
    std::vector<Coordinates> Coords;
    Coords.reserve(1000000);

    for (int i = 0; i < files.size(); i++)
    {
        std::string infileName = files[i];

        std::ifstream infile(infileName);

        std::string line;

        while (!infile.eof())
        {
            std::string substr;
            Coordinates c;

            std::getline(infile, substr, ',');
            if (!(substr.empty() || substr == "\n"))
                c.x = std::stof(substr);

            std::getline(infile, substr, ',');
            if (!(substr.empty() || substr == "\n"))
                c.y = std::stof(substr);

            std::getline(infile, substr, ',');
            if (!(substr.empty() || substr == "\n"))
                c.z = std::stof(substr);

            Coordinates compare(0.0f, 0.0f, 0.0f);

            if ((compare != c ))
                Coords.push_back(c);
        }
    }
    Coords.shrink_to_fit();
    return Coords;
}

