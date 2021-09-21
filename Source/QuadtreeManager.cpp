#include "../Headers/QuadtreeManager.h"
#include "../Headers/FileReader.h"

#include <iostream>
#include <cmath>
#include <fstream>
#include <chrono>
#include <filesystem>

template<typename T>
QuadtreeManager<T>::QuadtreeManager()
{
    quad = new Quadtree<T>();
    quad->level = 0;
}

template<typename T>
QuadtreeManager<T>::QuadtreeManager(T tL, T bR)
{
    quad = new Quadtree<T>(tL,bR);
    topLeft = tL;
    bottomRight = bR;
    quad->level = 0;
}

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

template<typename T>
void QuadtreeManager<T>::Insert( Node<T>* n)
{
    if (type == TreeType::Single)
    {
        SubInsert(quad, n);
    }
    else
    {
        for (int j = 0; j < bottomnodes.size(); j++)
        {
            if (inBoundary(bottomnodes[j], n->pos))
            {
                if (bottomnodes[j]->hasData == true)
                {
                    //tree already loaded
                    SubInsert(bottomnodes[j], n);
                }
                else
                {
                    //deload the previous tree
                    for (int k = 0; k < bottomnodes.size(); k++)
                    {
                        if (bottomnodes[k]->hasData == true) //deloads only if a tree is loaded
                        {
                            std::ofstream datastream;

                            datastream.open("./" + prePath + std::to_string(bottomnodes[k]->index) + ".bin", std::ios::binary);

                            WriteQuadToFile(bottomnodes[k], &datastream);

                            datastream.close();

                            bottomnodes[k]->~Quadtree();
                            bottomnodes[k]->hasData = false;
                            break;
                        }
                    }

                    //load the tree if the file exists
                    if (std::filesystem::exists("./" + prePath + std::to_string(bottomnodes[j]->index) + ".bin"))
                    {
                        std::ifstream datastream2;

                        datastream2.open("./" + prePath + std::to_string(bottomnodes[j]->index) + ".bin", std::ios::binary);

                        ReadFromFile(bottomnodes[j], &datastream2);
                        bottomnodes[j]->hasData = true;

                        datastream2.close();
                    }
                    else
                    {
                        bottomnodes[j]->hasData = true;
                    }
                    //Insert the new node to the loaded tree
                    if (inBoundary(bottomnodes[j], n->pos))
                    {
                        SubInsert(bottomnodes[j], n);
                        break;
                    }
                }
            }
        }
    }
}

template <typename T>
void QuadtreeManager<T>::SubInsert(Quadtree<T>* q, Node<T>* n)
{
    if (!inBoundary(q,n->pos))
    {
        delete n;
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
                q->topRightTree->hasData = true;
            }
            SubInsert(q->topRightTree, n);
        }

        // Indicates Bottom Right Tree 
        else
        {
            if (q->bottomRightTree == nullptr)
            {
                q->bottomRightTree = new Quadtree<T>(T((q->topLeft.x + q->bottomRight.x) / 2, (q->topLeft.y + q->bottomRight.y) / 2), T(q->bottomRight.x, q->bottomRight.y));
                q->bottomRightTree->level = q->level + 1;
                q->bottomRightTree->hasData = true;
            }
            SubInsert(q->bottomRightTree, n);
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
                q->topLeftTree->hasData = true;
            }
            SubInsert(q->topLeftTree, n);
        }

        // Indicates Bottom Left Tree 
        else
        {
            if (q->bottomLeftTree == nullptr)
            {
                q->bottomLeftTree = new Quadtree<T>(T(q->topLeft.x, (q->topLeft.y + q->bottomRight.y) / 2), T((q->topLeft.x + q->bottomRight.x) / 2, q->bottomRight.y));
                q->bottomLeftTree->level = q->level + 1;
                q->bottomLeftTree->hasData = true;
            }
            SubInsert(q->bottomLeftTree, n);
        }
    }
}

template<typename T>
Node<T>* QuadtreeManager<T>::Search(T p)
{
    if (!inBoundary(quad, p))
        return nullptr;
    
    if (type == TreeType::Single)
    {
        return Subsearch(quad, p);
    }
    else
    {
        for (int j = 0; j < bottomnodes.size(); j++)
        {
            if (inBoundary(bottomnodes[j], p))
            {
                if (bottomnodes[j]->hasData == true)
                {
                    //tree already loaded
                    return Subsearch(bottomnodes[j], p);
                }
                else
                {
                    //deload the previous tree
                    std::cout << "Point Causing Deload: " <<p.x << "," << p.y << "\n";
                    for (int k = 0; k < bottomnodes.size(); k++)
                    {
                        if (bottomnodes[k]->hasData == true) //deloads only if a tree is loaded
                        {
                            bottomnodes[k]->~Quadtree();
                            bottomnodes[k]->hasData = false;
                            break;
                        }
                    }

                    //load the tree if the file exists
                    if (std::filesystem::exists("./" + prePath + std::to_string(bottomnodes[j]->index) + ".bin"))
                    {
                        std::ifstream datastream2;

                        datastream2.open("./" + prePath + std::to_string(bottomnodes[j]->index) + ".bin", std::ios::binary);

                        ReadFromFile(bottomnodes[j], &datastream2);
                        if(bottomnodes[j]->bottomLeftTree != nullptr || bottomnodes[j]->bottomRightTree != nullptr || bottomnodes[j]->topLeftTree != nullptr || bottomnodes[j]->topRightTree != nullptr)
                            bottomnodes[j]->hasData = true;

                        datastream2.close();
                    }
                    //Insert the new node to the loaded tree
                    return Subsearch(bottomnodes[j], p);
                }
            }
        }
    }
}

template<typename T>
Node<T>* QuadtreeManager<T>::SearchW(T p)
{
    if (!inBoundary(quad, p))
        return nullptr;

    if (type == TreeType::Single)
    {
        return Subsearch(quad, p);
    }
    else
    {
        for (int j = 0; j < bottomnodes.size(); j++)
        {
            if (inBoundary(bottomnodes[j], p))
            {
                if (bottomnodes[j]->hasData == true)
                {
                    //tree already loaded
                    return Subsearch(bottomnodes[j], p);
                }
                else
                {
                    //deload the previous tree
                    for (int k = 0; k < bottomnodes.size(); k++)
                    {
                        if (bottomnodes[k]->hasData == true) //deloads only if a tree is loaded
                        {
                            std::ofstream datastream;

                            datastream.open("./" + prePath + std::to_string(bottomnodes[k]->index) + ".bin", std::ios::binary);

                            WriteQuadToFile(bottomnodes[k], &datastream);

                            datastream.close();

                            bottomnodes[k]->~Quadtree();
                            bottomnodes[k]->hasData = false;
                            break;
                        }
                    }

                    //load the tree if the file exists
                    if (std::filesystem::exists("./" + prePath + std::to_string(bottomnodes[j]->index) + ".bin"))
                    {
                        std::ifstream datastream2;

                        datastream2.open("./" + prePath + std::to_string(bottomnodes[j]->index) + ".bin", std::ios::binary);

                        ReadFromFile(bottomnodes[j], &datastream2);
                        bottomnodes[j]->hasData = true;

                        datastream2.close();
                    }
                    //Insert the new node to the loaded tree
                    return Subsearch(bottomnodes[j], p);
                }
            }
        }
    }
}

template<typename T>
bool QuadtreeManager<T>::inBoundary(Quadtree<T>* q, T p) const
{
    //greater than left
    bool left = q->topLeft.x - p.x < 0.0001;
    //less than right
    bool right = p.x - q->bottomRight.x < 0.0001;
    //greater than bottom
    bool top = q->bottomRight.y - p.y < 0.0001;
    //less than top
    bool bottom = p.y - q->topLeft.y < 0.0001;
    return left && right && top && bottom;
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
void QuadtreeManager<T>::CreateTreestoLevel()
{
    CreatetoLevel(quad, splitlevel);
}

template<typename T>
void QuadtreeManager<T>::SetTreeType(TreeType t)
{
    type = t;

    if (type == TreeType::Split)
    {
        CreateTreestoLevel();
        GetBottomNodes(quad);

        for (int j = 0; j < bottomnodes.size(); j++)
        {
            //Index the trees in the list for reference
            (bottomnodes[j])->index = j;
        }
    }
}

template<typename T>
void QuadtreeManager<T>::Cleanup()
{
    for (int k = 0; k < bottomnodes.size(); k++)
    {
        if (bottomnodes[k]->hasData == true) //deloads only if a tree is loaded
        {
            std::ofstream datastream;

            datastream.open("./" + prePath + std::to_string(bottomnodes[k]->index) + ".bin", std::ios::binary);

            WriteQuadToFile(bottomnodes[k], &datastream);

            datastream.close();

            bottomnodes[k]->~Quadtree();
            bottomnodes[k]->hasData = false;
            break;
        }
    }

}

template<typename T>
void QuadtreeManager<T>::WriteManagerToFile()
{    
    std::ofstream datastream;

    datastream.open("./" + prePath + "_def.bin", std::ios::binary);

    datastream.write((char*)&levelreq, sizeof(int));
    datastream.write((char*)&memlevel, sizeof(int));
    datastream.write((char*)&splitlevel, sizeof(int));
    datastream.write((char*)&spacing, sizeof(float));
    datastream.write((char*)&topLeft, sizeof(T));
    datastream.write((char*)&bottomRight, sizeof(T));
    datastream.write((char*)&type, sizeof(TreeType));

    datastream.close();

    if (type == TreeType::Single)
    {
        datastream.open("./" + prePath + ".bin", std::ios::binary);
        WriteQuadToFile(quad, &datastream);
        datastream.close();
    }
}

template<typename T>
void QuadtreeManager<T>::ReadManagerFromFile()
{
    std::ifstream datastream;

    datastream.open("./" + prePath + "_def.bin", std::ios::binary);

    datastream.read((char*)&levelreq, sizeof(int));
    datastream.read((char*)&memlevel, sizeof(int));
    datastream.read((char*)&splitlevel, sizeof(int));
    datastream.read((char*)&spacing, sizeof(float));
    datastream.read((char*)&topLeft, sizeof(T));
    datastream.read((char*)&bottomRight, sizeof(T));
    datastream.read((char*)&type, sizeof(TreeType));

    quad = new Quadtree<T>(topLeft,bottomRight);
    quad->level = 0;
    SetTreeType(type);
    
    
    datastream.close();

    if (type == TreeType::Single)
    {
        datastream.open("./" + prePath + ".bin", std::ios::binary);
        ReadFromFile(quad, &datastream);
        datastream.close();
    }
}

template<typename T>
QuadtreeManager<T>::~QuadtreeManager()
{
    if (quad != nullptr)
    {
        quad->~Quadtree();
        delete quad;
        quad = nullptr;
    }
}

template<typename T>
void QuadtreeManager<T>::CalculateTreeProps(const double xextent, const double yextent, const int mem)
{
	levelreq = 0;

    float area = (float)std::pow(std::max(xextent, yextent),2); //set area as square using the largest dimension
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

    splitlevel = levelreq - memlevel;

	std::cout << std::endl << "Memory Split Level: " << splitlevel << std::endl << std::endl;

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
		nodecount += (float)std::pow(4, i);

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

        topLeft = T(mm.minx, mm.maxy);
        bottomRight = T(mm.maxx, mm.miny);
    }
}

template<typename T>
void QuadtreeManager<T>::CreateSingleTree(std::vector<std::string> files)
{
    quad->topLeft = topLeft;
    quad->bottomRight = bottomRight;
    quad->level = 0;
    quad->hasData = true;

    for (int i = 0; i < files.size(); i++)
    {
        std::ifstream fs(files[i]);

        double x,y,z;
        while (!fs.eof())
        {
            FileReader::ReadLine(&fs, x, y, z);

            T n(x, y, z);
            Node<T>* node = new Node<T>(n);

            SubInsert(quad, node);
        }
    }
}

template<typename T>
void QuadtreeManager<T>::CreateSplitTree(std::vector<std::string> files)
{
    quad->topLeft = topLeft;
    quad->bottomRight = bottomRight;
    quad->level = 0;
    quad->hasData = true;

    CreatetoLevel(quad, splitlevel);

    GetBottomNodes(quad);

    for (int j = 0; j < bottomnodes.size(); j++)
    {
        //Index the trees in the list for reference
        (bottomnodes[j])->index = j;
    }

    std::cout << "List of bottom nodes generated" << std::endl;

    std::cout << "Creating Output Streams" << std::endl;

    std::vector<std::ofstream*> outstreams;

    for (int j = 0; j < bottomnodes.size(); j++)
    {
        std::ofstream* ofstream = new std::ofstream;
        ofstream->open("./" + prePath + std::to_string(bottomnodes[j]->index) + ".bin", std::ios_base::app | std::ios::binary);
        outstreams.push_back(ofstream);
    }

    std::cout << "Output Streams Created" << std::endl;

    for (int i = 0; i < files.size(); i++)
    {
        std::ifstream fs(files[i]);

        double x, y, z;
        while (!fs.eof())
        {
            FileReader::ReadLine(&fs, x, y, z);

            T n(x, y, z);
            Node<T>* node = new Node<T>(n);

            for (int j = 0; j < bottomnodes.size(); j++)
            {
                if (inBoundary(bottomnodes[j], n))
                {
                    WriteTToFile(&n, outstreams[j]);
                    break;
                }
            }
            delete node;
        }
    }

    std::cout << "Tree Structures Written" << std::endl;

    for (int j = 0; j < bottomnodes.size(); j++) //Clean up data streams
    {
        outstreams[j]->close();
        delete outstreams[j];
        outstreams[j] = nullptr;
    }
}

template<typename T>
Node<T>* QuadtreeManager<T>::Subsearch(Quadtree<T>* q, T p) const
{
    // Current quad cannot contain it 
    if (!inBoundary(q, p))
        return nullptr;

    // We are at a quad of unit length 
    // We cannot subdivide this quad further 
    if (q->n != nullptr && ((p.x - q->n->pos.x) < (spacing)+.0001 && (p.y - q->n->pos.y) < (spacing)+.0001))
        return q->n;

    if (q->topRightTree == nullptr)
        return nullptr;

    if (q->bottomRightTree == nullptr)
        return nullptr;

    if (q->bottomLeftTree == nullptr)
        return nullptr;

    if (q->topLeftTree == nullptr)
        return nullptr;

    if (((q->topLeft.x + q->bottomRight.x) / 2) <= p.x)
    {
        // Indicates topRightTree 
        if ((q->topLeft.y + q->bottomRight.y) / 2 <= p.y)
        {
            if (q->topRightTree == nullptr)
                return nullptr;
            return Subsearch(q->topRightTree, p);
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

template<typename T>
void QuadtreeManager<T>::WriteQuadToFile(Quadtree<T>* q, std::ofstream* datastream)
{
    if (q->topRightTree != nullptr)
        WriteQuadToFile(q->topRightTree,datastream);
    if (q->bottomRightTree != nullptr)
        WriteQuadToFile(q->bottomRightTree,datastream);
    if (q->bottomLeftTree != nullptr)
        WriteQuadToFile(q->bottomLeftTree,datastream);
    if (q->topLeftTree != nullptr)
        WriteQuadToFile(q->topLeftTree,datastream);

    if (q->n != nullptr)
    {
        datastream->write((char*)&(q->n->pos), sizeof(T));
    }
}

template<typename T>
void QuadtreeManager<T>::WriteTToFile(T* t, std::ofstream* datastream)
{
    if (t != nullptr)
    {
        datastream->write((char*)t, sizeof(T));
    }
}

template<typename T>
void QuadtreeManager<T>::ReadFromFile(Quadtree<T>* q, std::ifstream* datastream)
{
    while (!datastream->eof())
    {
        T temp;

        datastream->read((char*)&temp, sizeof(T));

        Node<T>* node = new Node<T>(temp);

        SubInsert(quad, node);
    }
}

template<typename T>
void QuadtreeManager<T>::CreatetoLevel(Quadtree<T>* q, int target)
{
    if (q->level >= target)
        return;

    q->topRightTree = new Quadtree<T>(T((q->topLeft.x + q->bottomRight.x) / 2, q->topLeft.y), T(q->bottomRight.x, (q->topLeft.y + q->bottomRight.y) / 2));
    q->topRightTree->level = q->level + 1;
    if(q->topRightTree->level == target)
        q->topRightTree->hasData = false;
    else
        q->topRightTree->hasData = true;
    CreatetoLevel(q->topRightTree, target);

    q->bottomRightTree = new Quadtree<T>(T((q->topLeft.x + q->bottomRight.x) / 2, (q->topLeft.y + q->bottomRight.y) / 2), T(q->bottomRight.x, q->bottomRight.y));
    q->bottomRightTree->level = q->level + 1;
    if (q->topRightTree->level == target)
        q->bottomRightTree->hasData = false;
    else
        q->bottomRightTree->hasData = true;
    CreatetoLevel(q->bottomRightTree, target);

    q->topLeftTree = new Quadtree<T>(T(q->topLeft.x, q->topLeft.y), T((q->topLeft.x + q->bottomRight.x) / 2, (q->topLeft.y + q->bottomRight.y) / 2));
    q->topLeftTree->level = q->level + 1;
    if (q->topRightTree->level == target)
        q->topLeftTree->hasData = false;
    else
        q->topLeftTree->hasData = true;
    CreatetoLevel(q->topLeftTree, target);

    q->bottomLeftTree = new Quadtree<T>(T(q->topLeft.x, (q->topLeft.y + q->bottomRight.y) / 2), T((q->topLeft.x + q->bottomRight.x) / 2, q->bottomRight.y));
    q->bottomLeftTree->level = q->level + 1;
    if (q->topRightTree->level == target)
        q->bottomLeftTree->hasData = false;
    else
        q->bottomLeftTree->hasData = true;
    CreatetoLevel(q->bottomLeftTree, target);
}

template<typename T>
void QuadtreeManager<T>::GetBottomNodes(Quadtree<T>* q)
{
    if (q->level == splitlevel)
        bottomnodes.push_back(q);

    if (q->topRightTree != nullptr)
        GetBottomNodes(q->topRightTree);
    if (q->bottomRightTree != nullptr)
        GetBottomNodes(q->bottomRightTree);
    if (q->bottomLeftTree != nullptr)
        GetBottomNodes(q->bottomLeftTree);
    if (q->topLeftTree != nullptr)
        GetBottomNodes(q->topLeftTree);
}


template class QuadtreeManager<Coordinates>;
template class QuadtreeManager<Bit>;
template class QuadtreeManager<Vec3>;
template class QuadtreeManager<Normal>;
template class QuadtreeManager<FlowDirection>;
template class QuadtreeManager<FlowGeneral>;