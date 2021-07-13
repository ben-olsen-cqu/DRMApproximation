#include "../Headers/QuadtreeManager.h"
#include "../Headers/FileReader.h"

#include <iostream>
#include <cmath>
#include <fstream>
#include <chrono>

template<typename T>
void QuadtreeManager<T>::CreateQuadtree(const std::vector<std::string> files, const float _spacing, const int mem)
{
    auto time_start = std::chrono::steady_clock::now();

    spacing = _spacing;
    MinMax mm;
    FileReader::GetMinMaxCSV(files, mm);

	CalculateTreeProps((mm.maxx-mm.minx), (mm.maxy-mm.miny), mem/2);

    CalculateTreeExtent(mm);

	if (type == TreeType::Single)
	{
        CreateSingleTree(files);
	}
	else
	{
        CreateSplitTree(files);
	}

    auto time_complete = std::chrono::steady_clock::now();

    auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(time_complete - time_start);
    
    std::cout << std::endl << "Quadtree setup complete!" << std::endl << "Setup completed in: " << time_diff.count() << "ms\n" << std::endl << std::endl;
}

template <typename T>
void QuadtreeManager<T>::Insert(Quadtree<T>* q, Node<T>* n)
{
    if (!inBoundary(q,n->pos))
    {
        return;
    }

    double checkx = std::abs(q->topLeft.x - q->bottomRight.x);
    double checky = std::abs(q->topLeft.y - q->bottomRight.y);

    if (checkx <= spacing && checky <= spacing)
    {
        if (q->n == nullptr)
            q->n = n;
        return;
    }

    if (((q->topLeft.x + q->bottomRight.x) / 2) <= n->pos.x)
    {
        // Indicates Top Right Tree 
        if (((q->topLeft.y + q->bottomRight.y) / 2) <= n->pos.y)
        {
            if (q->topRightTree == nullptr)
            {
                q->topRightTree = new Quadtree<T>(T((q->topLeft.x + q->bottomRight.x) / 2, q->topLeft.y), T(q->bottomRight.x, (q->topLeft.y + q->bottomRight.y) / 2));
                q->topRightTree->level = q->level + 1;
            }
            Insert(q->topRightTree, n);
        }

        // Indicates Bottom Right Tree 
        else
        {
            if (q->bottomRightTree == nullptr)
            {
                q->bottomRightTree = new Quadtree<T>(T((q->topLeft.x + q->bottomRight.x) / 2, (q->topLeft.y + q->bottomRight.y) / 2), T(q->bottomRight.x, q->bottomRight.y));
                q->bottomRightTree->level = q->level + 1;
            }
            Insert(q->bottomRightTree, n);
        }
    }
    else
    {
        // Indicates Top Left Tree 
        if (((q->topLeft.y + q->bottomRight.y) / 2) <= n->pos.y)
        {
            if (q->topLeftTree == nullptr)
            {
                q->topLeftTree = new Quadtree<T>(T(q->topLeft.x, q->topLeft.y), T((q->topLeft.x + q->bottomRight.x) / 2, (q->topLeft.y + q->bottomRight.y) / 2));
                q->topLeftTree->level = q->level + 1;
            }
            Insert(q->topLeftTree, n);
        }

        // Indicates Bottom Left Tree 
        else
        {
            if (q->bottomLeftTree == nullptr)
            {
                q->bottomLeftTree = new Quadtree<T>(T(q->topLeft.x, (q->topLeft.y + q->bottomRight.y) / 2), T((q->topLeft.x + q->bottomRight.x) / 2, q->bottomRight.y));
                q->bottomLeftTree->level = q->level + 1;
            }
            Insert(q->bottomLeftTree, n);
        }
    }
}

template<typename T>
Node<T>* QuadtreeManager<T>::Search(T p) const
{
    return Subsearch(quad, p);
}

template<typename T>
bool QuadtreeManager<T>::inBoundary(Quadtree<T>* q, T p) const
{
    return (p.x >= q->topLeft.x && p.x <= q->bottomRight.x && p.y <= q->topLeft.y && p.y >= q->bottomRight.y);
}

template<typename T>
T QuadtreeManager<T>::TopLeft() const
{
    return topLeft;
}

template<typename T>
T QuadtreeManager<T>::BottomRight() const
{
    return bottomRight;
}

template<typename T>
void QuadtreeManager<T>::CalculateTreeProps(const double xextent, const double yextent, const int mem)
{
	levelreq = 0;

    float area = std::pow(std::max(xextent, yextent),2); //set area as square using the largest dimension
    double spacingsq = std::pow(spacing, 2);
    double nodesreq = (area / spacingsq);

#if defined(DEBUG)
    std::cout << "Spacing: " << spacing << std::endl;
    std::cout << "x-axis extent = " << xextent << "\ty-axis extent = " << yextent << std::endl;
    std::cout << "Total Area: " << area << std::endl;
    std::cout << "Area per node Ideal: " << spacingsq << std::endl;
    std::cout << "Number of nodes required at bottom level: " << nodesreq << std::endl << std::endl;
#endif

	for (int level = 0; level <= 20; level++)
	{
		float nodes = CalculateNodes(level);

#if defined(DEBUG)
		if(nodes < 999999 )
			std::cout << "Nodes at Level " << level << "\t:" << nodes << "\t\t\tSize of Nodes \t- " << (nodes*sizeof(Quadtree<Coordinates>))/1000000 << "MB" << std::endl;
		if(nodes > 999999)
			std::cout << "Nodes at Level " << level << "\t:" << nodes << " \t\tSize of Nodes \t- " << (nodes * sizeof(Quadtree<Coordinates>)) / 1000000 << "MB" << std::endl;
#endif

		if ((area / nodes) < spacingsq)
		{
			levelreq = level;
			break;
		}
	}

	std::cout << std::endl << "Level required: " << levelreq << std::endl;

	//Calculate largest relative level for sub trees if a split has to occur. Checked against memory allowance.

	memlevel = 0;

	for (int level = 0; level <= 20; level++)
	{
		float nodes = CalculateNodes(level);

		if (((nodes * sizeof(Quadtree<Coordinates>)) / 1000000) < mem)
		{
			memlevel = level;
		}
	}

    //if (CalculateNodes(memlevel) / nodesreq > 1)
    //    memlevel++;

	std::cout << std::endl << "Memory Split Level: " << memlevel << std::endl << std::endl;

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
void QuadtreeManager<T>::CalculateTreeExtent(MinMax mm)
{
    if (((mm.maxx - mm.minx) - (mm.maxy - mm.miny)) < .0001f) //Input data is square
    {
#if defined(DEBUG)
        std::cout << "Square Data\n";
#endif
        topLeft = T(mm.minx, mm.maxy);
        bottomRight = T(mm.maxx, mm.miny);
    }
    else //Input data has rectangular extents, tree must be square so increase the smaller dimension to the larger one
    {
#if defined(DEBUG)
        std::cout << "Rectangular Data\n";
#endif
        double extentx = mm.maxx - mm.minx;
        double extenty = mm.maxy - mm.miny;

        if (extentx < extenty) //Tall Data match xextent to yextent
        {
            mm.minx = mm.minx + (extentx / 2) - (extenty / 2);
            mm.maxx = mm.maxx - (extentx / 2) + (extenty / 2);
        }
        else //Wide Data
        {
            mm.miny = mm.miny + (extenty / 2) - (extentx / 2);
            mm.maxy = mm.maxy - (extenty / 2) + (extentx / 2);
        }

        std::cout << std::fixed << "Min and Max Coords: " << mm.minx << "," << mm.miny << "," << mm.minz << "\t" << mm.maxx << "," << mm.maxy << "," << mm.maxz << std::endl;

        topLeft = T(mm.minx, mm.maxy);
        bottomRight = T(mm.maxx, mm.miny);
    }
}

template<typename T>
void QuadtreeManager<T>::CreateSingleTree(std::vector<std::string> files)
{
    quad = new Quadtree<T>(topLeft,bottomRight);
    quad->level = 0;

    for (int i = 0; i < files.size(); i++)
    {
        std::ifstream fs(files[i]);

        double x,y,z;
        while (!fs.eof())
        {
            FileReader::ReadLine(&fs, x, y, z);

            T n(x, y, z);
            Node<T>* node = new Node<T>(n);

            Insert(quad, node);
        }
    }
}

template<typename T>
void QuadtreeManager<T>::CreateSplitTree(std::vector<std::string> files)
{
}

template<typename T>
Node<T>* QuadtreeManager<T>::Subsearch(Quadtree<T>* q, T p) const
{
    // Current quad cannot contain it 
    if (!inBoundary(q, p))
        return nullptr;

    // We are at a quad of unit length 
    // We cannot subdivide this quad further 
    if (q->n != nullptr && (p.x == q->n->pos.x && p.y == q->n->pos.y))
        return q->n;

    if (((q->topLeft.x + q->bottomRight.x) / 2) <= p.x)
    {
        // Indicates topRightTree 
        if ((q->topLeft.y + q->bottomRight.y) / 2 <= p.y)
        {
            if (q->topRightTree == nullptr)
                return nullptr;
            return Subsearch(q->topRightTree,p);
        }

        // Indicates botRightTree 
        else
        {
            if (q->bottomRightTree == nullptr)
                return nullptr;
            return Subsearch(q->bottomRightTree, p);
        }
    }
    else
    {

        // Indicates topLeftTree 
        if ((q->topLeft.y + q->bottomRight.y) / 2 <= p.y)
        {
            if (q->topLeftTree == nullptr)
                return nullptr;
            return Subsearch(q->topLeftTree, p);
        }

        // Indicates botLeftTree 
        else
        {
            if (q->bottomLeftTree == nullptr)
                return nullptr;
            return Subsearch(q->bottomLeftTree, p);
        }
    }

    return nullptr;
}
