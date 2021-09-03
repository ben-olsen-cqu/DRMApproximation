#include "../Headers/CatchmentBuilder.h"
#include "../Headers/FileWriter.h"

#include <iostream>
#include <algorithm>

#define PI 3.14159265

void CatchmentBuilder::CreateCatchments(QuadtreeManager<Coordinates>& quad)
{
    QuadtreeManager<Coordinates> smooth(quad.topLeft, quad.bottomRight);

    smooth.prePath = "Temp/SmoothTree/SmoothTree";
    smooth.spacing = quad.spacing;
    smooth.splitlevel = quad.splitlevel;
    smooth.SetTreeType(quad.type);

    if (quad.type == TreeType::Single)
    {
        SmoothPointsSingle(quad, smooth);

        std::cout << "Exporting Original Surface\n";
        FileWriter::WriteCoordTreeASC("./Exports/Surfaces/Original", quad);

        std::cout << "Exporting Smoothed Surface\n";
        FileWriter::WriteCoordTreeASC("./Exports/Surfaces/Smooth", smooth);
    }
    else
    {
        SmoothPointsSplit(quad, smooth);

        std::cout << "Exporting Original Surface\n";
        FileWriter::WriteCoordTreeASC("./Exports/Surfaces/Original", quad);

        std::cout << "Exporting Smoothed Surface\n";
        FileWriter::WriteCoordTreeASC("./Exports/Surfaces/Smooth", smooth);
    }

    quad.~QuadtreeManager();

    Normal tL(quad.topLeft.x + quad.spacing / 2, quad.topLeft.y - quad.spacing / 2);
    Normal bR(quad.bottomRight.x - quad.spacing / 2, quad.bottomRight.y + quad.spacing / 2);

    QuadtreeManager<Normal> normal(tL, bR);

    normal.prePath = "Temp/NormalTree/Tree";
    normal.spacing = quad.spacing;
    normal.splitlevel = quad.splitlevel;
    normal.SetTreeType(quad.type);

    if (quad.type == TreeType::Single)
    {
        CalculateNormalsSingle(smooth, normal);

        std::cout << "Writing Normals to File.\n";
        ////FileWriter::WriteVecNormals3dWKT("./Exports/Vectors/SmoothNormals3dWKT", normal);
        FileWriter::WriteVecNormals2dWKT("./Exports/Vectors/SmoothNormals2dWKT", normal);
    }
    else
    {
        CalculateNormalsSplit(smooth, normal);
    }

    smooth.~QuadtreeManager();

    FlowDirection tL2(quad.topLeft.x + quad.spacing / 2, quad.topLeft.y - quad.spacing / 2);
    FlowDirection bR2(quad.bottomRight.x - quad.spacing / 2, quad.bottomRight.y + quad.spacing / 2);

    QuadtreeManager<FlowDirection> flowdirection(tL2, bR2);

    flowdirection.prePath = "Temp/DirectionTree/Tree";
    flowdirection.spacing = quad.spacing;
    flowdirection.splitlevel = quad.splitlevel;
    flowdirection.SetTreeType(quad.type);

    if (quad.type == TreeType::Single)
    {
        CalculateFlowDirectionSingle(flowdirection, normal);

        std::cout << "Writing Flow Directions to File.\n";
        FileWriter::WriteFlowDirection2dWKT("./Exports/Vectors/FlowDirections2dWKT", flowdirection);
    }
    else
    {
        //CalculateFlowDirectionSingle(smooth, normal);
    }

    normal.~QuadtreeManager();

    FlowGeneral tL3(quad.topLeft.x + quad.spacing / 2, quad.topLeft.y - quad.spacing / 2);
    FlowGeneral bR3(quad.bottomRight.x - quad.spacing / 2, quad.bottomRight.y + quad.spacing / 2);

    QuadtreeManager<FlowGeneral> flowaccum(tL3, bR3);

    flowaccum.prePath = "Temp/AccumulationTree/Tree";
    flowaccum.spacing = quad.spacing;
    flowaccum.splitlevel = quad.splitlevel;
    flowaccum.SetTreeType(quad.type);

    if (quad.type == TreeType::Single)
    {
        CalculateFlowAccumulationSingle(flowdirection, flowaccum);

        std::cout << "Exporting Flow Accumulation Surface\n";
        FileWriter::WriteFlowGeneralTreeASC("./Exports/Surfaces/Accum", flowaccum);
    }
    else
    {
        //CalculateFlowAccumulationSplit(flowdirection, flowaccum);
    }

    std::vector<FlowPath> flowpaths;

    if (quad.type == TreeType::Single)
    {
        flowpaths = StreamLinkingSingle(flowaccum, flowdirection);

        std::cout << "Exporting Stream Paths\n";
        FileWriter::WriteStreamPaths2dWKT("./Exports/Vectors/FlowPaths2dWKT", flowpaths);
    }
    else
    {
        //CalculateFlowAccumulationSplit(flowdirection, flowaccum);
    }

    flowaccum.~QuadtreeManager();

    QuadtreeManager<FlowGeneral> catchclass(tL3, bR3);

    catchclass.prePath = "Temp/CatchmentClassification/Tree";
    catchclass.spacing = quad.spacing;
    catchclass.splitlevel = quad.splitlevel;
    catchclass.SetTreeType(quad.type);


    CatchmentClassification(catchclass, flowdirection, flowpaths);
    if (quad.type == TreeType::Single)
    {
        std::cout << "Exporting Flow Accumulation Surface\n";
        FileWriter::WriteFlowGeneralTreeASC("./Exports/Surfaces/CatchmentClassification", catchclass);
    }
    else
    {
        //CalculateFlowAccumulationSplit(flowdirection, flowaccum);
    }

    flowdirection.~QuadtreeManager();
}

void CatchmentBuilder::SmoothPointsSingle(QuadtreeManager<Coordinates>& quad, QuadtreeManager<Coordinates>& smooth)
{
    std::cout << "Smoothing Surface\n";
    double boundsx = (quad.BottomRight().x) - (quad.TopLeft().x);
    double boundsy = (quad.TopLeft().y) - (quad.BottomRight().y);
    double bottom = (quad.BottomRight().y);
    double left = (quad.TopLeft().x);

    int blurrad = 13; //odd numbers only
    int storenum = (blurrad - 1) / 2;

    for (int y = 0; y <= boundsy; y++)
        for (int x = 0; x <= boundsx; x++)
        {
            Node<Coordinates>* node = quad.Search(Coordinates(x + left, y + bottom));

            if (node != nullptr)
            {
                Coordinates coord = node->pos;

                std::vector<Coordinates> vecCoords;

                for (int j = -storenum; j <= storenum; j++)
                    for (int i = -storenum; i <= storenum; i++)
                    {
                        Node<Coordinates>* n = quad.Search(Coordinates(double(i) + double(x) + left, double(j) + double(y) + bottom));

                        if (n != nullptr)
                        {
                            Coordinates coord = n->pos;
                            vecCoords.push_back(coord);
                        }
                    }

                float zavg = 0.0f;

                for (auto const c : vecCoords)
                {
                    zavg += c.z;
                }

                zavg /= vecCoords.size();

                coord.z = zavg;

                smooth.Insert(new Node<Coordinates>(coord));
            }
        }
}

void CatchmentBuilder::SmoothPointsSplit(QuadtreeManager<Coordinates>& quad, QuadtreeManager<Coordinates>& smooth)
{
    std::cout << "Smoothing Surface\n";
    double boundsx = (quad.BottomRight().x) - (quad.TopLeft().x);
    double boundsy = (quad.TopLeft().y) - (quad.BottomRight().y);
    double bottom = (quad.BottomRight().y);
    double left = (quad.TopLeft().x);

    int numquads = quad.splitlevel * 2; //quad splits the area in half in the x and y axis
    int blurrad = 5; //odd numbers only
    int storenum = (blurrad - 1) / 2;

    int boundsperquadx = std::floor(boundsx / numquads) + 1;
    int boundsperquady = std::floor(boundsy / numquads) + 1;

    int totalquads = numquads * numquads;

    ////Create quadtree for storing edge cases
    QuadtreeManager<Coordinates> gaps(quad.topLeft, quad.bottomRight);
    gaps.prePath = "Temp/SmoothTree/GapTree";
    gaps.spacing = quad.spacing;
    gaps.splitlevel = 0;
    gaps.SetTreeType(quad.type);

    //for loops for iterating through split level trees
    for (int v = 0; v < numquads; v++) //move vertically through sub trees
        for (int w = 0; w < numquads; w++) //move horizontally through sub trees
        {
            std::cout << "\rQuad " << v * numquads + (w + 1) << " of " << totalquads << " Complete";

            for (int y = 0; y < boundsperquady; y++) //move through each coord in the y direction of the subtree
                for (int x = 0; x < boundsperquadx; x++)//move through each coord in the x direction of the subtree
                {
                    if (y < storenum || (boundsperquady - y) <= storenum || x < storenum || (boundsperquadx - x) <= storenum)
                    {
                        Node<Coordinates>* node = quad.Search(Coordinates(x + w * boundsperquadx + left, y + v * boundsperquady + bottom));

                        if (node != nullptr)
                        {
                            Coordinates coord = node->pos;
                            gaps.Insert(new Node<Coordinates>(coord));
                        }
                    }
                    else
                    {
                        Node<Coordinates>* node = quad.Search(Coordinates(x + w * boundsperquadx + left, y + v * boundsperquady + bottom));

                        if (node != nullptr)
                        {
                            Coordinates coord = node->pos;

                            std::vector<Coordinates> vecCoords;

                            for (int j = -storenum; j <= storenum; j++)
                                for (int i = -storenum; i <= storenum; i++)
                                {
                                    Node<Coordinates>* n = quad.Search(Coordinates((i + x) + (w * boundsperquadx) + left, (j + y) + (v * boundsperquady) + bottom));

                                    if (n != nullptr)
                                    {
                                        Coordinates c = n->pos;
                                        vecCoords.push_back(c);
                                    }
                                }

                            float zavg = 0.0f;

                            for (auto const c : vecCoords)
                            {
                                zavg += c.z;
                            }

                            zavg /= vecCoords.size();

                            coord.z = zavg;

                            smooth.Insert(new Node<Coordinates>(coord));
                        }
                    }
                }

        }

    smooth.Cleanup();
    gaps.Cleanup();
    quad.Cleanup();

    std::cout << "\nComplete\nFilling Gaps...\n";

    for (int v = 0; v < numquads; v++) //move vertically through sub trees
    {
        for (int w = 0; w < numquads; w++) //move horizontally through sub trees
        {
            std::cout << "\rQuad " << v * numquads + (w + 1) << " of " << totalquads << " Complete";

            //Top
            for (int x = 0; x < boundsperquadx; x++)
                for (int y = boundsperquady - storenum - 1; y < boundsperquady; y++)
                {
                    Node<Coordinates>* node = gaps.Search(Coordinates(x + w * boundsperquadx + left, y + v * boundsperquady + bottom));

                    if (node != nullptr)
                    {
                        Coordinates coord = node->pos;

                        std::vector<Coordinates> vecCoords;

                        for (int j = -storenum; j <= storenum; j++)
                            for (int i = -storenum; i <= storenum; i++)
                            {
                                Node<Coordinates>* n = gaps.Search(Coordinates((i + x) + (w * boundsperquadx) + left, (j + y) + (v * boundsperquady) + bottom));

                                if (n != nullptr)
                                {
                                    Coordinates c = n->pos;
                                    vecCoords.push_back(c);
                                }
                            }

                        float zavg = 0.0f;

                        for (auto const c : vecCoords)
                        {
                            zavg += c.z;
                        }

                        zavg /= vecCoords.size();

                        coord.z = zavg;

                        smooth.Insert(new Node<Coordinates>(coord));
                    }
                }
            //Bottom
            for (int x = 0; x < boundsperquadx; x++)
                for (int y = 0; y < storenum; y++)
                {
                    Node<Coordinates>* node = gaps.Search(Coordinates(x + w * boundsperquadx + left, y + v * boundsperquady + bottom));

                    if (node != nullptr)
                    {
                        Coordinates coord = node->pos;

                        std::vector<Coordinates> vecCoords;

                        for (int j = -storenum; j <= storenum; j++)
                            for (int i = -storenum; i <= storenum; i++)
                            {
                                Node<Coordinates>* n = gaps.Search(Coordinates((i + x) + (w * boundsperquadx) + left, (j + y) + (v * boundsperquady) + bottom));

                                if (n != nullptr)
                                {
                                    Coordinates c = n->pos;
                                    vecCoords.push_back(c);
                                }
                            }

                        double zavg = 0.0f;

                        for (auto const c : vecCoords)
                        {
                            zavg += c.z;
                        }

                        zavg /= vecCoords.size();

                        coord.z = zavg;

                        smooth.Insert(new Node<Coordinates>(coord));
                    }
                }
            //Right

            for (int x = boundsperquadx - storenum - 1; x < boundsperquadx; x++)
                for (int y = storenum; y < boundsperquady - storenum - 1; y++)
                {
                    Node<Coordinates>* node = gaps.Search(Coordinates(x + w * boundsperquadx + left, y + v * boundsperquady + bottom));

                    if (node != nullptr)
                    {
                        Coordinates coord = node->pos;

                        std::vector<Coordinates> vecCoords;

                        for (int j = -storenum; j <= storenum; j++)
                            for (int i = -storenum; i <= storenum; i++)
                            {
                                Node<Coordinates>* n = gaps.Search(Coordinates((j + x) + (w * boundsperquadx) + left, (i + y) + (v * boundsperquady) + bottom));

                                if (n != nullptr)
                                {
                                    Coordinates c = n->pos;
                                    vecCoords.push_back(c);
                                }
                            }

                        float zavg = 0.0f;

                        for (auto const c : vecCoords)
                        {
                            zavg += c.z;
                        }

                        zavg /= vecCoords.size();

                        coord.z = zavg;

                        smooth.Insert(new Node<Coordinates>(coord));
                    }
                }
            //Left
            for (int x = 0; x < storenum; x++)
                for (int y = 0; y < boundsperquady; y++)
                {
                    Node<Coordinates>* node = gaps.Search(Coordinates(x + w * boundsperquadx + left, y + v * boundsperquady + bottom));

                    if (node != nullptr)
                    {
                        Coordinates coord = node->pos;

                        std::vector<Coordinates> vecCoords;

                        for (int j = -storenum; j <= storenum; j++)
                            for (int i = -storenum; i <= storenum; i++)
                            {
                                Node<Coordinates>* n = gaps.Search(Coordinates((j + x) + (w * boundsperquadx) + left, (i + y) + (v * boundsperquady) + bottom));

                                if (n != nullptr)
                                {
                                    Coordinates c = n->pos;
                                    vecCoords.push_back(c);
                                }
                            }

                        float zavg = 0.0f;

                        for (auto const c : vecCoords)
                        {
                            zavg += c.z;
                        }

                        zavg /= vecCoords.size();

                        coord.z = zavg;

                        smooth.Insert(new Node<Coordinates>(coord));
                    }
                }

        }
    }

    smooth.Cleanup();
    gaps.Cleanup();
    quad.Cleanup();

    std::cout << "\nComplete\n";
}

void CatchmentBuilder::CalculateNormalsSingle(QuadtreeManager<Coordinates>& smooth, QuadtreeManager<Normal>& normal)
{
    std::cout << "Generating Normals\n";
    double boundsx = (smooth.BottomRight().x) - (smooth.TopLeft().x)-1;
    double boundsy = (smooth.TopLeft().y) - (smooth.BottomRight().y)-1;
    double bottom = (smooth.BottomRight().y);
    double left = (smooth.TopLeft().x);

    for (int y = 0; y <= boundsx; y++)
        for (int x = 0; x <= boundsy; x++)
        {

            //Get points in a quad going clockwise starting from the BL

            Vec3 p1, p2, p3, p4, vec1, vec2, vec3, vec4, translation, normal1, normal2;

            Coordinates c = smooth.Search(Coordinates(x + left, y + bottom))->pos;

            p1 = Vec3(c.x, c.y, c.z);

            c = smooth.Search(Coordinates(x + left, y + bottom + 1))->pos;

            p2 = Vec3(c.x, c.y, c.z);

            c = smooth.Search(Coordinates(x + left + 1, y + bottom + 1))->pos;

            p3 = Vec3(c.x, c.y, c.z);

            c = smooth.Search(Coordinates(x + left + 1, y + bottom))->pos;

            p4 = Vec3(c.x, c.y, c.z);
            
            double avz = (p1.z + p2.z + p3.z + p4.z) / 4;
            translation = Vec3(p1.x + 0.5f, p1.y + .5f, avz);

            //translate each vector so each side of the quad is represented
            vec1 = p2 - p1;
            vec2 = p3 - p2;
            vec3 = p4 - p3;
            vec4 = p1 - p4;

            //Cross product to calculate normal
            normal1 = vec1 % vec2;

            normal2 = vec3 % vec4;

            //average normals to get normal for quad
            Vec3 normalq = Vec3((normal1.x + normal2.x) / 2, (normal1.y + normal2.y) / 2, (normal1.z + normal2.z) / 2);
            normalq.Normalize();
            normalq.x = -normalq.x;
            normalq.y = -normalq.y;
            normalq.z = -normalq.z;

            normalq += translation;
            
            normal.Insert(new Node<Normal>(Normal(translation, normalq)));
        }
}

void CatchmentBuilder::CalculateNormalsSplit(QuadtreeManager<Coordinates>& quad, QuadtreeManager<Normal>& normal)
{
    std::cout << "Generating Normals\n";
}

void CatchmentBuilder::CalculateFlowDirectionSingle(QuadtreeManager<FlowDirection>& flowdirection, QuadtreeManager<Normal>& normal)
{
    std::cout << "Calculating Flow Directions\n";
    double boundsx = (normal.BottomRight().x) - (normal.TopLeft().x);
    double boundsy = (normal.TopLeft().y) - (normal.BottomRight().y);
    double bottom = (normal.BottomRight().y);
    double left = (normal.TopLeft().x);

    for (int x = 0; x <= boundsx; x++)
        for (int y = 0; y <= boundsy; y++)
        {
            auto f = normal.Search(Normal(x + left, y + bottom));

            if (f != nullptr)
            {
                auto n = f->pos;

                Vec2 translated(n.norm.x - n.x, n.norm.y - n.y);

                float angle = std::atan2(translated.x, translated.y);

                int octant = (int)std::round(8 * angle / (2 * PI) + 8) % 8;

                Direction dir = (Direction)octant;

                flowdirection.Insert(new Node<FlowDirection>(FlowDirection(x + left, y + bottom, dir)));
            }
        }
}

void CatchmentBuilder::CalculateFlowAccumulationSingle(QuadtreeManager<FlowDirection>& flowdirection, QuadtreeManager<FlowGeneral>& flowaccum)
{
    std::cout << "Calculating Flow Accumulations\n";

    QuadtreeManager<FlowGeneral> NIDP(flowaccum.TopLeft(), flowaccum.BottomRight());

    //  NIDP quadtree for storing the number of cells that flow into a given cell
    //  If the NIDP value is 0 then the cell is a source cell and is the top of the flow path
    //  If the NIDP value is 1 then the cell is an interior cell and simply take the flow and passes it to the next cell
    //  If the NIDP value is >=2 then the cell is an intersectionm of flow paths
    

    NIDP.prePath = "Temp/AccumulationTree/Tree";
    NIDP.spacing = flowaccum.spacing;
    NIDP.splitlevel = flowaccum.splitlevel;
    NIDP.SetTreeType(flowaccum.type);

    double boundsx = (flowaccum.BottomRight().x) - (flowaccum.TopLeft().x);
    double boundsy = (flowaccum.TopLeft().y) - (flowaccum.BottomRight().y);
    double bottom = (flowaccum.BottomRight().y);
    double left = (flowaccum.TopLeft().x);

    //Initialise the accumulation grid at 1
    for (int x = 0; x <= boundsx; x++)
        for (int y = 0; y <= boundsy; y++)
        {
            flowaccum.Insert(new Node<FlowGeneral>(FlowGeneral(x + left, y + bottom, 1)));
        }

    //Calculate the NIDP grid
    for (int x = 0; x <= boundsx; x++)
        for (int y = 0; y <= boundsy; y++)
        {
            Node<FlowDirection>* node = flowdirection.Search(FlowDirection(x + left, y + bottom));

            if (node != nullptr)
            {
                int NIDPval = 0;

                node = flowdirection.Search(FlowDirection(x-1 + left, y-1 + bottom));
                if (node != nullptr)
                {
                    if (node->pos.direction == Direction::NE)
                        NIDPval++;
                }

                node = flowdirection.Search(FlowDirection(x + left, y - 1 + bottom));
                if (node != nullptr)
                {
                    if (node->pos.direction == Direction::N)
                        NIDPval++;
                }

                node = flowdirection.Search(FlowDirection(x + 1 + left, y - 1 + bottom));
                if (node != nullptr)
                {
                    if (node->pos.direction == Direction::NW)
                        NIDPval++;
                }

                node = flowdirection.Search(FlowDirection(x - 1 + left, y + bottom));
                if (node != nullptr)
                {
                    if (node->pos.direction == Direction::E)
                        NIDPval++;
                }

                 node = flowdirection.Search(FlowDirection(x + 1 + left, y + bottom));
                if (node != nullptr)
                {
                    if (node->pos.direction == Direction::W)
                        NIDPval++;
                }

                node = flowdirection.Search(FlowDirection(x - 1 + left, y + 1 + bottom));
                if (node != nullptr)
                {
                    if (node->pos.direction == Direction::SE)
                        NIDPval++;
                }

                node = flowdirection.Search(FlowDirection(x + left, y + 1 + bottom));
                if (node != nullptr)
                {
                    if (node->pos.direction == Direction::S)
                        NIDPval++;
                }

                node = flowdirection.Search(FlowDirection(x + 1 + left, y + 1 + bottom));
                if (node != nullptr)
                {
                    if (node->pos.direction == Direction::SW)
                        NIDPval++;
                }

                NIDP.Insert(new Node<FlowGeneral>(FlowGeneral(x + left, y + bottom, NIDPval)));
            }
        }

    std::cout << "Exporting NIDP Surface\n";
    FileWriter::WriteFlowGeneralTreeASC("./Exports/Surfaces/NIDP", NIDP);

    for (int x = 0; x <= boundsx; x++)
        for (int y = 0; y <= boundsy; y++)
        {
            Node<FlowGeneral>* nNIDP = NIDP.Search(FlowGeneral(x + left, y + bottom));

            if (nNIDP != nullptr)
            {
                if (nNIDP->pos.iValue == 0)
                {
                    //Source node found, trace to the downstream intersection node or boundary of the data
                    int Accum = 0;
                    int i = x;
                    int j = y;

                    do
                    {
                        auto nAcc = flowaccum.Search(FlowGeneral(i + left, j + bottom));

                        nAcc->pos.iValue += Accum;
                        Accum = nAcc->pos.iValue;

                        Direction d = flowdirection.Search(FlowDirection(i + left, j + bottom))->pos.direction;

                        //Increment i or j based on the flow direction to get the next cell
                        switch (d)
                        {
                        case Direction::N:
                        {
                            j++;
                            break;
                        };
                        case Direction::NE:
                        {
                            j++;
                            i++;
                            break;
                        };
                        case Direction::E:
                        {
                            i++;
                            break;
                        };
                        case Direction::SE:
                        {
                            j--;
                            i++;
                            break;
                        };
                        case Direction::S:
                        {
                            j--;
                            break;
                        };
                        case Direction::SW:
                        {
                            j--;
                            i--;
                            break;
                        };
                        case Direction::W:
                        {
                            i--;
                            break;
                        };
                        case Direction::NW:
                        {
                            j++;
                            i--;
                            break;
                        };
                        } 

                        if (nNIDP->pos.iValue >= 2)
                        {
                            nNIDP->pos.iValue--;
                            break;
                        }

                        //for each (Vec2 fp in flowpath)
                        //{
                        //    if (fp == Vec2(i, j))
                        //    {
                        //        //std::cout << "Circular flow path found\n";
                        //        break;
                        //    }
                        //}

                        nNIDP = NIDP.Search(FlowGeneral(i + left, j + bottom));
                    } 
                    while (nNIDP != nullptr);
                }
            }
        }

    NIDP.~QuadtreeManager();
}

std::vector<FlowPath> CatchmentBuilder::StreamLinkingSingle(QuadtreeManager<FlowGeneral>& flowaccum, QuadtreeManager<FlowDirection>& flowdirection)
{
    std::cout << "Stream Linking\n";

    double boundsx = (flowaccum.BottomRight().x) - (flowaccum.TopLeft().x);
    double boundsy = (flowaccum.TopLeft().y) - (flowaccum.BottomRight().y);
    double bottom = (flowaccum.BottomRight().y);
    double left = (flowaccum.TopLeft().x);

    std::vector<std::vector<Vec2>> flowpaths;
    int acctarget = 2250; //2250 for Test Data 1, 200 for TD 4

    for (int y = 0; y < boundsy; y++)
        for (int x = 0; x < boundsx; x++)
        {
            Node<FlowGeneral>* flowacc = flowaccum.Search(FlowGeneral(x + left, y + bottom));

            if (flowacc->pos.iValue > acctarget)
                TraceFlowPath(flowdirection, &flowpaths, x, y);
        }
    std::vector<std::vector<Vec2>> joinedflowpaths;

    joinedflowpaths.push_back(flowpaths[0]);
    flowpaths.erase(std::begin(flowpaths));


    while (flowpaths.size() != 0)
    {
        bool added;
        do //repeat until no more segments are added
        {
            added = false;
            for (int a = 0; a < flowpaths.size(); a++) //for loop for searching through all remaining 
            {
                bool reset = false;

                if (flowpaths[a][1] == joinedflowpaths[joinedflowpaths.size()-1][0])
                {
                    //If true then add the flowpaths line before the joined paths line in joined paths
                    joinedflowpaths[joinedflowpaths.size() - 1].insert(joinedflowpaths[joinedflowpaths.size() - 1].begin(), flowpaths[a][0]);
                    flowpaths.erase(flowpaths.begin() + a);
                    added = true;
                    reset = true;
                }
                else if (flowpaths[a][0] == joinedflowpaths[joinedflowpaths.size() - 1][(joinedflowpaths[joinedflowpaths.size() - 1].size() - 1)])
                {
                    joinedflowpaths[joinedflowpaths.size() - 1].push_back(flowpaths[a][1]);
                    flowpaths.erase(flowpaths.begin() + a);
                    added = true;
                    reset = true;
                }

                if (reset)
                    a = 0;
            }
        } while (added);

        if (flowpaths.size() != 0)
        {
            joinedflowpaths.push_back(flowpaths[0]);
            flowpaths.erase(std::begin(flowpaths));
        }
    }

    std::vector<FlowPath> copypaths;

    for (int i = 0; i < joinedflowpaths.size(); i++)
    {
        FlowPath fp;
        for (int j = 0; j < joinedflowpaths[i].size(); j++)
        {
            fp.path.push_back(joinedflowpaths[i][j]);
        }
        copypaths.push_back(fp);
    }

    std::sort(copypaths.begin(),copypaths.end());
    std::reverse(copypaths.begin(), copypaths.end());

    for (int i = 0; i < copypaths.size(); i++)
    {
        copypaths[i].id = i;
    }

    return copypaths;
}

void CatchmentBuilder::TraceFlowPath(QuadtreeManager<FlowDirection>& flowdirection, std::vector<std::vector<Vec2>>* flowpaths, int x, int y)
{
    double bottom = (flowdirection.BottomRight().y);
    double left = (flowdirection.TopLeft().x);

    int i = x;
    int j = y;
    std::vector<Vec2> path;
    bool end = false;

    auto d = flowdirection.Search(FlowDirection(i + left, j + bottom));

    while (d != nullptr)
    {
        path.push_back(Vec2(i + left, j + bottom));

        //Increment i or j based on the flow direction to get the next cell
        switch (d->pos.direction)
        {
        case Direction::N:
        {
            j++;
            break;
        };
        case Direction::NE:
        {
            j++;
            i++;
            break;
        };
        case Direction::E:
        {
            i++;
            break;
        };
        case Direction::SE:
        {
            j--;
            i++;
            break;
        };
        case Direction::S:
        {
            j--;
            break;
        };
        case Direction::SW:
        {
            j--;
            i--;
            break;
        };
        case Direction::W:
        {
            i--;
            break;
        };
        case Direction::NW:
        {
            j++;
            i--;
            break;
        };
        }

        bool match = false; //check if a coord matches in the already defined paths
        for (int a = 0; a < flowpaths->size(); a++)
        {
            for each (Vec2 vec in (*flowpaths)[a])
            {
                if (vec == Vec2(i, j))
                {
                    match = true;
                }
            }
        }

        if (!match)
        {
            path.push_back(Vec2(i + left, j + bottom));
            break;
        }

        d = flowdirection.Search(FlowDirection(i + left, j + bottom));
    } 


    flowpaths->push_back(path);
}

void CatchmentBuilder::CatchmentClassification(QuadtreeManager<FlowGeneral>& catchclass, QuadtreeManager<FlowDirection>& flowdirection, std::vector<FlowPath>& fps)
{
    std::cout << "Classifying Catchment Areas\n";

    int breakdist = 100; //Distance along the flow paths to split the catchment

    double boundsx = (catchclass.BottomRight().x) - (catchclass.TopLeft().x);
    double boundsy = (catchclass.TopLeft().y) - (catchclass.BottomRight().y);
    double bottom = (catchclass.BottomRight().y);
    double left = (catchclass.TopLeft().x);

    //Initialise the accumulation grid at 1
    for (int x = 0; x <= boundsx; x++)
        for (int y = 0; y <= boundsy; y++)
        {
            if (flowdirection.Search(FlowDirection(x + left, y + bottom)) != nullptr)
                catchclass.Insert(new Node<FlowGeneral>(FlowGeneral(x + left, y + bottom, 0)));
        }

    std::vector<DischargePoint> dischargepoints;
    
    //*****************************************************************************************************************************************************
    //  2 part process
    // 1. Add all end of flow paths that aren't already defined by the intersection points
    // 2. Add points on segments longer than the breakdist

    //End points
    for (int i = 0; i < fps.size(); i++)
    {
        Vec2 end = fps[i].GetPointAtDist(0); //Get end point of each flow path

        for (int j = 0; j < fps.size(); j++)
        {
            if (fps[j].PointonPath(end)) //check if the end point matches any other points in any other flow path
            {
                if (dischargepoints.size() > 0) {

                    bool added = false;

                    for (int j = 0; j < dischargepoints.size(); j++) //check if the intersection has already been added
                    {
                        if (dischargepoints[j].location == end)
                        {
                            bool added = true;
                        }
                    }
                    if (!added)
                        dischargepoints.push_back(DischargePoint(end));

                }
                else
                {
                    dischargepoints.push_back(DischargePoint(end));
                }
            }

        }
    }

    //Add inter-points to long segments
    for (int i = 0; i < fps.size(); i++)
    {
        float length = fps[i].Length();//get the length of the drainage path
        if(length > breakdist) //if the length is less than the breakdist then don't process any further
            for (int dist = breakdist; dist < length; dist += breakdist) 
            {
                Vec2 point = fps[i].GetPointAtDist(dist);//get a point on the flow path at breakdist intervals

                //check if the generated point is within the breakdist from any other already added point
                bool insert = true;
                for (int j = 0; j < dischargepoints.size(); j++) //iterate through all discharge points
                {
                    if (fps[i].PointonPath(dischargepoints[j].location)) //check if the discharge point is on the current flowpath
                    {
                        double dist = fps[i].FlowLengthBetween(point, dischargepoints[j].location);
                        if (dist < breakdist) //if it is on the flow path, check if it's within breakdist from the point we want to insert
                        {
                            insert = false;
                        }
                    }
                }
                if(insert)
                    dischargepoints.push_back(DischargePoint(point)); //insert the point
            }
    }

    //Clean up duplicates
    std::vector<DischargePoint> temppoints;
    for (int i = 0; i < dischargepoints.size(); i++)
    {
        bool unique = true;
        for (int j = 0; j < temppoints.size(); j++)
        {
            if (dischargepoints[i].location == temppoints[j].location)
                unique = false;
        }
        if(unique)
            temppoints.push_back(dischargepoints[i]);
    }
    dischargepoints = temppoints;

    //Asign point indexes
    for (int j = 1; j < dischargepoints.size()+1; j++)
    {
        dischargepoints[j-1].index = j;
    }

    FileWriter::WriteVec2Points("./Exports/Points/DischargePoints2dWKT", dischargepoints);

    std::reverse(dischargepoints.begin(), dischargepoints.end());

    for (int i = 0; i < dischargepoints.size(); i++)
    {
        ClassifySubCatchment(catchclass, flowdirection, dischargepoints[i].index, dischargepoints[i].location);
        std::cout << "\rProcessing Catchment " << i+1 << " of " << dischargepoints.size();
    }
    std::cout << "\n";
}

void CatchmentBuilder::ClassifySubCatchment(QuadtreeManager<FlowGeneral>& catchclass, QuadtreeManager<FlowDirection>& flowdirection, int id, Vec2 point)
{
    //Start with the discharge coord
    //Add the coord to a vec(1)
    //do while loop for finding number of cells that flow into the current cell and storing coords in a second vec(2) in the scope of the loop
    //if just one cell flows into the current cell set the current cell id and iterate to the next one
        //once entire path is traced, remove the source point from (1)
    //else if two or more store the additional coords into (1)
        //once the first coord has been traced remove the source coord from (1)
        //load the 0th element from 1 and repeat the do while loop
    //terminate the do while loop when either no cells flow into the current or the cell already has an id

    std::vector<Vec2> missed;
    missed.push_back(point);

    while (missed.size() > 0)
    {
        std::vector<Vec2> inflowcells;

        double x = missed[0].x;
        double y = missed[0].y;

        Node<FlowGeneral>* nCatchPrev;
        Node<FlowGeneral>* nCatch = catchclass.Search(FlowGeneral(x, y));
        
        do
        {
            inflowcells.clear();

            if (nCatch != nullptr)
            {
                if (nCatch == nCatchPrev)
                    break;


                    auto node = flowdirection.Search(FlowDirection(x - 1, y - 1));
                    if (node != nullptr)
                    {
                        if (node->pos.direction == Direction::NE)
                            inflowcells.push_back(Vec2(x - 1, y - 1));
                    }

                    node = flowdirection.Search(FlowDirection(x, y - 1));
                    if (node != nullptr)
                    {
                        if (node->pos.direction == Direction::N)
                            inflowcells.push_back(Vec2(x, y - 1));
                    }

                    node = flowdirection.Search(FlowDirection(x + 1, y - 1));
                    if (node != nullptr)
                    {
                        if (node->pos.direction == Direction::NW)
                            inflowcells.push_back(Vec2(x + 1, y - 1));
                    }

                    node = flowdirection.Search(FlowDirection(x - 1, y));
                    if (node != nullptr)
                    {
                        if (node->pos.direction == Direction::E)
                            inflowcells.push_back(Vec2(x + 1, y - 1));
                    }

                    node = flowdirection.Search(FlowDirection(x + 1, y));
                    if (node != nullptr)
                    {
                        if (node->pos.direction == Direction::W)
                            inflowcells.push_back(Vec2(x + 1, y));
                    }

                    node = flowdirection.Search(FlowDirection(x - 1, y + 1));
                    if (node != nullptr)
                    {
                        if (node->pos.direction == Direction::SE)
                            inflowcells.push_back(Vec2(x - 1, y + 1));
                    }

                    node = flowdirection.Search(FlowDirection(x, y + 1));
                    if (node != nullptr)
                    {
                        if (node->pos.direction == Direction::S)
                            inflowcells.push_back(Vec2(x, y + 1));
                    }

                    node = flowdirection.Search(FlowDirection(x + 1, y + 1));
                    if (node != nullptr)
                    {
                        if (node->pos.direction == Direction::SW)
                            inflowcells.push_back(Vec2(x + 1, y + 1));
                    }


                    if (inflowcells.size() > 1)
                    {
                        for (int i = 1; i < inflowcells.size(); i++)
                        {
                            missed.push_back(inflowcells[i]);
                            inflowcells.erase(std::begin(inflowcells) + i);
                        }
                    }

                    if (nCatch->pos.iValue == 0)
                        nCatch->pos.iValue = id;

                    if (inflowcells.size() > 0)
                    {
                        x = inflowcells[0].x;
                        y = inflowcells[0].y;
                        nCatchPrev = nCatch;
                        nCatch = catchclass.Search(FlowGeneral(x, y));
                    }
                }
        }
        while (inflowcells.size() != 0);

        missed.erase(std::begin(missed));
    } 
}


