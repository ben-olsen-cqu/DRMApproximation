#include "../Headers/FileReader.h"

#include <fstream>
#include <filesystem>
#include <chrono>
#include <iostream>

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

void FileReader::GetMinMaxCSV(std::vector<std::string> files, MinMax& mm)
{
    auto time_start = std::chrono::steady_clock::now();

    for (int i = 0; i < files.size(); i++)
    {
        std::string infileName = files[i];

        std::ifstream infile(infileName);
        
        Coordinates compare(0.0f, 0.0f, 0.0f);
        
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

            if ((compare != c))
            {
                if (c.x > mm.maxx)
                    mm.maxx = c.x;

                if (c.y > mm.maxy)
                    mm.maxy = c.y;

                if (c.z > mm.maxz)
                    mm.maxz = c.z;

                if (c.x < mm.minx)
                    mm.minx = c.x;

                if (c.y < mm.miny)
                    mm.miny = c.y;

                if (c.z < mm.minz)
                    mm.minz = c.z;
            }
        }
    }

    auto time_complete = std::chrono::steady_clock::now();

    auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(time_complete - time_start);

    std::cout << std::fixed << "Min and Max Coords: " << mm.minx << "," << mm.miny << "," << mm.minz << "\t" << mm.maxx << "," << mm.maxy << "," << mm.maxz << std::endl;

    std::cout << "Min Max found in: " << time_diff.count() << "ms\n" << std::endl << std::endl;
}

void FileReader::ReadLine(std::ifstream* fs, double& x, double& y, double& z)
{
    std::string substr;

    std::getline(*fs, substr, ',');
    if (!(substr.empty() || substr == "\n"))
        x = std::stod(substr);

    std::getline(*fs, substr, ',');
    if (!(substr.empty() || substr == "\n"))
        y = std::stod(substr);

    std::getline(*fs, substr, ',');
    if (!(substr.empty() || substr == "\n"))
        z = std::stod(substr);
}

void FileReader::ReadStreamPathsBinary(std::string filepath, std::vector<FlowPath>& flowpaths)
{
    std::ifstream datastream;

    datastream.open("./" + filepath + ".bin", std::ios::binary);

    while (!datastream.eof())
    {
        FlowPath fp;
        int size;

        datastream.read((char*)&fp.id, sizeof(int));
        datastream.read((char*)&size, sizeof(int));

        for (int i = 0; i < size; i++)
        {
            Vec2 p;
            datastream.read((char*)&p, sizeof(Vec2));
            fp.path.push_back(p);
        }

        flowpaths.push_back(fp);
    }

    flowpaths.erase(std::end(flowpaths)-1);
    datastream.close();
}

void FileReader::ReadCatchmentsBinary(std::string filepath, std::vector<Catchment>& catchlist)
{
    std::ifstream datastream;

    datastream.open("./" + filepath + ".bin", std::ios::binary);

    while (!datastream.eof())
    {
        Catchment var;

        datastream.read((char*)&var.id, sizeof(int));
        datastream.read((char*)&var.area, sizeof(int));
        datastream.read((char*)&var.mannings, sizeof(float));
        datastream.read((char*)&var.IL, sizeof(float));
        datastream.read((char*)&var.CL, sizeof(float));
        datastream.read((char*)&var.avgslope, sizeof(float));
        datastream.read((char*)&var.flowdistance, sizeof(float));
        datastream.read((char*)&var.highestpt, sizeof(float));
        datastream.read((char*)&var.lowestpt, sizeof(float));
        datastream.read((char*)&var.longestfplength, sizeof(float));
        datastream.read((char*)&var.dp, sizeof(DischargePoint));
        datastream.read((char*)&var.bounds, sizeof(MinMax));

        //for (int i = 0; i < size; i++)
        //{
        //    Vec2 p;
        //    datastream.read((char*)&p, sizeof(Vec2));
        //    fp.path.push_back(p);
        //}
        //Longest FP
        datastream.read((char*)&var.longest.id, sizeof(int));

        int size;
        datastream.read((char*)&size, sizeof(int));
        for (int i = 0; i < size; i++)
        {
            Vec2 p;
            datastream.read((char*)&p, sizeof(Vec2));
            var.longest.path.push_back(p);
        }
        
        //Catchment BDY
        datastream.read((char*)&size, sizeof(int));
        for (int i = 0; i < size; i++)
        {
            Vec2 p;
            datastream.read((char*)&p, sizeof(Vec2));
            var.points.push_back(p);
        }
        
        //Isochrone Areas
        datastream.read((char*)&size, sizeof(int));
        for (int i = 0; i < size; i++)
        {
            int p;
            datastream.read((char*)&p, sizeof(int));
            var.isochroneareas.push_back(p);
        }

        catchlist.push_back(var);
    }

    catchlist.erase(std::end(catchlist) - 1);
    datastream.close();
}

RainfallSeries FileReader::ReadRainfallData(std::string filepath)
{
    RainfallSeries rs;
    std::ifstream infile(filepath);

    std::string line;

    std::getline(infile, line);

    while (!infile.eof())
    {
        std::string substr;
        float time;
        float depth;

        std::getline(infile, substr, ',');
        if (!(substr.empty() || substr == "\n"))
            time = std::stof(substr);

        std::getline(infile, substr, '\n');
        if (!(substr.empty() || substr == "\n"))
            depth = std::stof(substr);

        rs.series.push_back(Rainfall(int(time*60),depth));
    }

    return rs;
}
