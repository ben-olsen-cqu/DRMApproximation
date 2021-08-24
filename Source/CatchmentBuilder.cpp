#include "../Headers/CatchmentBuilder.h"
#include "../Headers/FileWriter.h"

#include <iostream>

#define PI 3.14159265

void CatchmentBuilder::CreateCatchments(QuadtreeManager<Coordinates>& quad)
{
    std::cout << "Smoothing Surface\n";

    QuadtreeManager<Coordinates> smooth(quad.topLeft,quad.bottomRight);

    smooth.prePath = "Temp/SmoothTree/SmoothTree";
    smooth.spacing = quad.spacing;
    smooth.splitlevel = quad.splitlevel;
    smooth.SetTreeType(quad.type);

    if (quad.type == TreeType::Single)
    {
        SmoothPointsSingle(quad, smooth);
    }
    else
    {
        SmoothPointsSplit(quad, smooth);
    }

    //std::cout << "Exporting Original Surface\n";
    //FileWriter::WriteCoordTreeASC("./Exports/Surfaces/Original", quad);

    //std::cout << "Exporting Smoothed Surface\n";
    //FileWriter::WriteCoordTreeASC("./Exports/Surfaces/Smooth", smooth);

    quad.~QuadtreeManager();

    Normal tL(quad.topLeft.x + quad.spacing / 2, quad.topLeft.y - quad.spacing / 2);
    Normal bR(quad.bottomRight.x - quad.spacing / 2, quad.bottomRight.y + quad.spacing / 2);
    
    QuadtreeManager<Normal> normal(tL,bR);

    normal.prePath = "Temp/NormalTree/Tree";
    normal.spacing = quad.spacing;
    normal.splitlevel = quad.splitlevel;
    normal.SetTreeType(quad.type);

    if (quad.type == TreeType::Single)
    {
        CalculateNormalsSingle(smooth, normal);
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
    }
    else
    {
        //CalculateFlowDirectionSingle(smooth, normal);
    }

    FlowAccumulation tL3(quad.topLeft.x + quad.spacing / 2, quad.topLeft.y - quad.spacing / 2);
    FlowAccumulation bR3(quad.bottomRight.x - quad.spacing / 2, quad.bottomRight.y + quad.spacing / 2);

    QuadtreeManager<FlowAccumulation> flowaccum(tL3, bR3);

    flowaccum.prePath = "Temp/AccumulationTree/Tree";
    flowaccum.spacing = quad.spacing;
    flowaccum.splitlevel = quad.splitlevel;
    flowaccum.SetTreeType(quad.type);

    if (quad.type == TreeType::Single)
    {
        CalculateFlowAccumulationSingle(flowdirection, flowaccum);
    }
    else
    {
        //CalculateFlowAccumulationSplit(flowdirection, flowaccum);
    }
    
}

void CatchmentBuilder::SmoothPointsSingle(QuadtreeManager<Coordinates>& quad, QuadtreeManager<Coordinates>& smooth)
{
    double boundsx = (quad.BottomRight().x) - (quad.TopLeft().x);
    double boundsy = (quad.TopLeft().y) - (quad.BottomRight().y);
    double bottom = (quad.BottomRight().y);
    double left = (quad.TopLeft().x);

    int blurrad = 5; //odd numbers only
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
    double boundsx = (smooth.BottomRight().x) - (smooth.TopLeft().x);
    double boundsy = (smooth.TopLeft().y) - (smooth.BottomRight().y);
    double bottom = (smooth.BottomRight().y);
    double left = (smooth.TopLeft().x);

    for (int x = 0; x < boundsx; x++)
        for (int y = 0; y < boundsy; y++)
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

            normalq += translation;
            
            normal.Insert(new Node<Normal>(Normal(translation, normalq)));
        }

    //std::cout << "Writing Normals to File.\n";
    //FileWriter::WriteVecNormals3dWKT("./Exports/Normals/SmoothNormals3dWKT", normal);
    //FileWriter::WriteVecNormals2dWKT("./Exports/Normals/SmoothNormals2dWKT", normal);
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

    for (int x = 0; x < boundsx; x++)
        for (int y = 0; y < boundsy; y++)
        {
            auto n = normal.Search(Normal(x + left, y + bottom))->pos;

            Vec2 translated(n.norm.x - n.x, n.norm.y - n.y);

            float angle = std::atan2(translated.x,translated.y);

            int octant = (int)std::round(8 * angle / (2 * PI) + 8) % 8;

            Direction dir = (Direction)octant;

            flowdirection.Insert(new Node<FlowDirection>(FlowDirection(x + left, y + bottom, dir)));
        }
    std::cout << "Complete\n";
}

void CatchmentBuilder::CalculateFlowAccumulationSingle(QuadtreeManager<FlowDirection>& flowdirection, QuadtreeManager<FlowAccumulation>& flowaccum)
{
    std::cout << "Calculating Flow Accumulations\n";
}

void CatchmentBuilder::CalculateStreamLinkingSingle(QuadtreeManager<FlowAccumulation>& flowaccum)
{
}

