#pragma once

#include "Quadtree.h"
#include "Coordinates.h"

#include <vector>

class QuadtreeBuilder
{
public:
	void CreateQuadtree(const std::vector<Coordinates>& Coords);
};