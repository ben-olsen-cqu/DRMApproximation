#pragma once

#include "Quadtree.h"
#include "Coordinates.h"
#include "Bit.h"
#include "CatchMath.h"

#include <vector>
#include <string>

enum class TreeType{Split, Single};

/* Create a wrapper for the quadtree class to manage dynamic loading and creation, */
template<typename T>
class QuadtreeManager
{
private:
	TreeType type;
	Quadtree<T>* quad;
	int levelreq;
	int memlevel;
	float spacing;
	T topLeft;
	T bottomRight;

public:
	QuadtreeManager() = default;
	void CreateQuadtree(const std::vector<std::string> files, const float _spacing, const int mem);
	void Insert(Quadtree<T>* q, Node<T>* n);
	Node<T>* Search(T p) const;
	bool inBoundary(Quadtree<T>* q, T p) const;
	T TopLeft() const;
	T BottomRight() const;
private:
	void CalculateTreeProps(const double xextent, const double yextent, const int mem);
	float CalculateNodes(int level);
	void CalculateTreeExtent(MinMax mm);
	void CreateSingleTree(std::vector<std::string> files);
	void CreateSplitTree(std::vector<std::string> files);
	Node<T>* Subsearch(Quadtree<T>* q, T p) const;
	void WriteToFile(Quadtree<T>* q, std::ofstream* datastream);
	void ReadFromFile(Quadtree<T>* q, std::ifstream* datastream);
	void CreatetoLevel(Quadtree<T>* q, int target);
	void GetBottomNodes(Quadtree<T>* q, std::vector<Quadtree<T>*>* bottomnodes);
};


