#pragma once

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
private:
	T topLeft;
	T bottomRight;

	bool loaded;
	bool hasData;
	int level;

	Node<T>* n = nullptr;

	float unit = 1.0f;

	Quadtree<T>* topRightTree = nullptr;
	Quadtree<T>* bottomRightTree = nullptr;
	Quadtree<T>* bottomLeftTree = nullptr;
	Quadtree<T>* topLeftTree = nullptr;

public:
	~Quadtree<T>()
	{
		if (n != nullptr)
		{;
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
		n = nullptr;
		topLeftTree = nullptr;
		topRightTree = nullptr;
		bottomLeftTree = nullptr;
		bottomRightTree = nullptr;

		topLeft = topL;
		bottomRight = bottomR;
	}
public:
    void insert(Node<T>* node);
    Node<T>* search(T p) const;
    bool inBoundary(T p) const;
	T TopLeft() const;
	T BottomRight() const;
};
