#include "../Headers/QuadtreeManager.h"

#include <iostream>
#include <cmath>

template<typename T>
void QuadtreeManager<T>::CreateQuadtree(const std::vector<std::string> files, const float spacing, const int mem)
{
	CalculateTreeProps(10000, 10000, spacing, mem);

	if (type == TreeType::Single)
	{
		//quad = CreateSingleTree()
	}
	else
	{
		//quad = CreateSplitTree()
	}

}

template<typename T>
void QuadtreeManager<T>::CalculateTreeProps(double xextent, double yextent, float spacing, int mem)
{
	levelreq = 0;
	float area = xextent * yextent;

	for (int level = 0; level <= 18; level++)
	{
		float nodes = CalculateNodes(level);
#if defined(DEBUG)
		if(nodes < 1000000 )
			std::cout << "Nodes at Level " << level << " :\t\t" << nodes << " Size of Nodes \t\t- " << (nodes*sizeof(Quadtree<Coordinates>))/1000000 << "MB" << std::endl;
		if(nodes > 1000000)
			std::cout << "Nodes at Level " << level << " :\t\t" << nodes << " Size of Nodes \t- " << (nodes * sizeof(Quadtree<Coordinates>)) / 1000000 << "MB" << std::endl;
#endif

		float spacingsq = spacing * spacing;

		if (area / nodes < spacingsq)
		{
			levelreq = level;
			break;
		}
	}

	std::cout << std::endl << "Level required: " << levelreq << std::endl;

	//Calculate largest relative level for sub trees if a split has to occur. Checked against memory allowance.

	memlevel = 0;

	for (int level = 0; level <= 18; level++)
	{
		float nodes = CalculateNodes(level);

		if (((nodes * sizeof(Quadtree<Coordinates>)) / 1000000) < (mem/2))
		{
			memlevel = level;
		}
	}

	std::cout << std::endl << "Memory Split Level: " << memlevel << std::endl;

	if (levelreq <= memlevel)
	{
		//No need to split the tree for memory management
		std::cout << "Single Tree Used\n";
		type = TreeType::Single;
	}
	else
	{
		int splitlevel = levelreq - memlevel; //This gives sub trees that are always a consistent size and the largest possible
		std::cout << "Split Tree Used\n";
		type = TreeType::Split;
	}

}

template<typename T>
float QuadtreeManager<T>::CalculateNodes(int level)
{
	float nodecount = 0;

	for (int i = 0; i < level+1; i++)
		nodecount += std::pow(4, i);

	return nodecount;
}

template<typename T>
Quadtree<T>* QuadtreeManager<T>::CreateSingleTree(T topL, T bottomR, float spacing)
{
	return nullptr;
}

template<typename T>
Quadtree<T>* QuadtreeManager<T>::CreateSplitTree(T topL, T bottomR, float spacing)
{
	return nullptr;
}
