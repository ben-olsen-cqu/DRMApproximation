#pragma once

#include <vector>
#include <string>

struct ProgamParams
{
    std::vector<std::string> files; //list of strings for input file paths

    int maxMem = 1000; //Memory limit for trees, 2 trees max will be active at a time so half for each tree 
    float spacing = 1.0f; //Spacing between the points in the x and y directions, only square grids are supported
    int reuselevel = 0;
};