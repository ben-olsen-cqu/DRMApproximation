#pragma once

#include "Quadtree.h"
#include "Coordinates.h"
#include "Bit.h"
#include "CatchMath.h"

#include <vector>
#include <string>

enum class TreeType{Split, Single};

template<typename T>
class QuadtreeManager
{
private:
	TreeType type;
	Quadtree<T>* quad;
	int levelreq;
	int memlevel;

public:
	QuadtreeManager() = default;
	void CreateQuadtree(const std::vector<std::string> files, const float spacing, const int mem);
private:
	void CalculateTreeProps(double xextent, double yextent, float spacing, int mem);
	float CalculateNodes(int level);
	Quadtree<T>* CreateSingleTree(T topL, T bottomR, float spacing);
	Quadtree<T>* CreateSplitTree(T topL, T bottomR, float spacing);

};

template class QuadtreeManager<Coordinates>;
template class QuadtreeManager<Bit>;
template class QuadtreeManager<Vec3>;
template class QuadtreeManager<Normal>;