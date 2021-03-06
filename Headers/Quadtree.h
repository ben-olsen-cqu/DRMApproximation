#pragma once

#include <limits>
#include "../Headers/MinMax.h"

template<typename T>
class Node
{
public:
	T pos;
	Node<T>(T _pos)
	{
		pos = _pos;
	}
	Node<T>() = default;
};

template<typename T>
class Quadtree
{
public:
	T topLeft;
	T bottomRight;

	bool hasData;
	int index;
	int level;

	Node<T>* n = nullptr;

	Quadtree<T>* topRightTree = nullptr;
	Quadtree<T>* bottomRightTree = nullptr;
	Quadtree<T>* bottomLeftTree = nullptr;
	Quadtree<T>* topLeftTree = nullptr;

	~Quadtree<T>()
	{
		if (n != nullptr)
		{
			delete n;
			n = nullptr;
		}
		if (topRightTree != nullptr)
		{
			delete topRightTree;
			topRightTree = nullptr;
		}
		if (bottomRightTree != nullptr)
		{
			delete bottomRightTree;
			bottomRightTree = nullptr;
		}
		if (topLeftTree != nullptr)
		{
			delete topLeftTree;
			topLeftTree = nullptr;
		}
		if (bottomLeftTree != nullptr)
		{
			delete bottomLeftTree;
			bottomLeftTree = nullptr;
		}
	};
	Quadtree<T>(T topL, T bottomR)
	{
		hasData = false;

		n = nullptr;
		topLeftTree = nullptr;
		topRightTree = nullptr;
		bottomLeftTree = nullptr;
		bottomRightTree = nullptr;

		topLeft = topL;
		bottomRight = bottomR;
	}

	Quadtree<T>()
	{
		hasData = false;

		n = nullptr;
		topLeftTree = nullptr;
		topRightTree = nullptr;
		bottomLeftTree = nullptr;
		bottomRightTree = nullptr;

		topLeft = T(0,0);
		bottomRight = T(0,0);
	}

	T TopLeft() const;
	T BottomRight() const;
};
