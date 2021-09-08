#include "../Headers/CatchmentBuilder.h"
#include "../Headers/FileWriter.h"
#include "../Headers/FileReader.h"

#include <iostream>
#include <algorithm>

#define PI 3.14159265

void CatchmentBuilder::CreateCatchments(ProgamParams progp)
{
    QuadtreeManager<Coordinates> quad;
    int blurrad = 21;           //pre-processing radius
    int acctarget = 10000;      //Number of cells required to flow into a cell before it's considered a stream
    int breakdist = 200;        //Distance along the flow paths to split the catchment

    if (progp.reuselevel >= 1)
    {
        //Reuse previously computed data
        quad.ReadManagerFromFile();
        std::cout << "Existing Original Surface Loaded\n";
    }
    else
    {
        //Create new data from input files
        quad.CreateQuadtree(progp.files, progp.spacing, progp.maxMem);
        quad.WriteManagerToFile();

        std::cout << "Exporting Original Surface\n";
        FileWriter::WriteCoordTreeASC("./Exports/Surfaces/Original", quad);
    }

    QuadtreeManager<Coordinates> smooth(quad.topLeft, quad.bottomRight);
    smooth.prePath = "Temp/SmoothTree/SmoothTree";

    if (progp.reuselevel >= 2)
    {
        //Reuse previously computed data
        smooth.ReadManagerFromFile();
        std::cout << "Existing Smoothed Surface Loaded\n";
    }
    else
    {
        //Create new data
        smooth.spacing = quad.spacing;
        smooth.splitlevel = quad.splitlevel;
        smooth.SetTreeType(quad.type);

        if (quad.type == TreeType::Single)
        {
            SmoothPointsSingle(quad, smooth,blurrad);
        }
        else
        {
            SmoothPointsSplit(quad, smooth,blurrad);
        }
        smooth.WriteManagerToFile();
        std::cout << "Exporting Smoothed Surface\n";
        FileWriter::WriteCoordTreeASC("./Exports/Surfaces/Smooth", smooth);
    }

    quad.~QuadtreeManager();

    Normal tL(quad.topLeft.x + quad.spacing / 2, quad.topLeft.y - quad.spacing / 2);
    Normal bR(quad.bottomRight.x - quad.spacing / 2, quad.bottomRight.y + quad.spacing / 2);

    QuadtreeManager<Normal> normal(tL, bR);
    normal.prePath = "Temp/NormalTree/Tree";

    if (progp.reuselevel >= 3)
    {
        //Reuse previously computed data
        normal.ReadManagerFromFile();
        std::cout << "Existing Normals Loaded\n";
    }
    else
    {
        //Create new data
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
        normal.WriteManagerToFile();
        std::cout << "Writing Normals to File.\n";
        //FileWriter::WriteVecNormals3dWKT("./Exports/Vectors/SmoothNormals3dWKT", normal);
        FileWriter::WriteVecNormals2dWKT("./Exports/Vectors/SmoothNormals2dWKT", normal);
    }

    smooth.~QuadtreeManager();

    FlowDirection tL2(quad.topLeft.x + quad.spacing / 2, quad.topLeft.y - quad.spacing / 2);
    FlowDirection bR2(quad.bottomRight.x - quad.spacing / 2, quad.bottomRight.y + quad.spacing / 2);

    QuadtreeManager<FlowDirection> flowdirection(tL2, bR2);
    flowdirection.prePath = "Temp/DirectionTree/Tree";
    
    if (progp.reuselevel >= 4)
    {
        //Reuse previously computed data
        flowdirection.ReadManagerFromFile();
        std::cout << "Existing Flow Directions Loaded\n";
    }
    else
    {
        //Create new data
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

        flowdirection.WriteManagerToFile();

        std::cout << "Writing Flow Directions to File.\n";
        FileWriter::WriteFlowDirection2dWKT("./Exports/Vectors/FlowDirections2dWKT", flowdirection);
    }

    normal.~QuadtreeManager();

    FlowGeneral tL3(quad.topLeft.x + quad.spacing / 2, quad.topLeft.y - quad.spacing / 2);
    FlowGeneral bR3(quad.bottomRight.x - quad.spacing / 2, quad.bottomRight.y + quad.spacing / 2);

    QuadtreeManager<FlowGeneral> flowaccum(tL3, bR3);
    flowaccum.prePath = "Temp/AccumulationTree/Tree";

    if (progp.reuselevel >= 5)
    {
        //Reuse previously computed data
        flowaccum.ReadManagerFromFile();
        std::cout << "Existing Flow Accumulations Loaded\n";
    }
    else
    {
        //Create new data
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

        flowaccum.WriteManagerToFile();

        std::cout << "Exporting Flow Accumulation Surface\n";
        FileWriter::WriteFlowGeneralTreeASC("./Exports/Surfaces/Accum", flowaccum);
    }

    std::vector<FlowPath> flowpaths;

    if (progp.reuselevel >= 6)
    {
        //Reuse previously computed data
        FileReader::ReadStreamPathsBinary("Temp/FlowPath/FlowLines", flowpaths);
        std::cout << "Existing Flow Paths Loaded\n";
    }
    else
    {
        //Create new data
        if (quad.type == TreeType::Single)
        {
            flowpaths = StreamLinkingSingle(flowaccum, flowdirection, acctarget);
        }
        else
        {
            
        }

        FileWriter::WriteStreamPathsBinary("Temp/FlowPath/FlowLines", flowpaths);

        std::cout << "Exporting Stream Paths\n";
        FileWriter::WriteStreamPaths2dWKT("./Exports/Vectors/FlowPaths2dWKT", flowpaths);
    }

    flowaccum.~QuadtreeManager();

    std::vector<DischargePoint> dischargepoints = GenerateDischargePoints(flowpaths, breakdist);

    QuadtreeManager<FlowGeneral> catchclass(tL3, bR3);
    catchclass.prePath = "Temp/CatchmentClassification/Tree";

    if (progp.reuselevel >= 7)
    {
        //Reuse previously computed data
        catchclass.ReadManagerFromFile();
        std::cout << "Existing Catchment Classifications Loaded\n";
    }
    else
    {
        //Create new data
        catchclass.spacing = quad.spacing;
        catchclass.splitlevel = quad.splitlevel;
        catchclass.SetTreeType(quad.type);

        CatchmentClassification(catchclass, flowdirection, dischargepoints);

        catchclass.WriteManagerToFile();

        std::cout << "Exporting Classified Catchment Surface\n";
        FileWriter::WriteFlowGeneralTreeASC("./Exports/Surfaces/CatchmentClassification", catchclass);
    }

    flowdirection.~QuadtreeManager();
    flowpaths.clear();

    std::vector<Catchment> catchlist;

    PolygoniseCatchments(catchclass, dischargepoints, catchlist);

    catchclass.~QuadtreeManager();
}

void CatchmentBuilder::SmoothPointsSingle(QuadtreeManager<Coordinates>& quad, QuadtreeManager<Coordinates>& smooth, int blurrad)
{
    std::cout << "Smoothing Surface\n";
    double boundsx = (quad.BottomRight().x) - (quad.TopLeft().x);
    double boundsy = (quad.TopLeft().y) - (quad.BottomRight().y);
    double bottom = (quad.BottomRight().y);
    double left = (quad.TopLeft().x);

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

void CatchmentBuilder::SmoothPointsSplit(QuadtreeManager<Coordinates>& quad, QuadtreeManager<Coordinates>& smooth,int blurrad)
{
    std::cout << "Smoothing Surface\n";
    double boundsx = (quad.BottomRight().x) - (quad.TopLeft().x);
    double boundsy = (quad.TopLeft().y) - (quad.BottomRight().y);
    double bottom = (quad.BottomRight().y);
    double left = (quad.TopLeft().x);

    int numquads = quad.splitlevel * 2; //quad splits the area in half in the x and y axis
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

                        nNIDP = NIDP.Search(FlowGeneral(i + left, j + bottom));
                    } 
                    while (nNIDP != nullptr);
                }
            }
        }

    NIDP.~QuadtreeManager();
}

std::vector<FlowPath> CatchmentBuilder::StreamLinkingSingle(QuadtreeManager<FlowGeneral>& flowaccum, QuadtreeManager<FlowDirection>& flowdirection, int acctarget)
{
    std::cout << "Stream Linking\n";

    double boundsx = (flowaccum.BottomRight().x) - (flowaccum.TopLeft().x);
    double boundsy = (flowaccum.TopLeft().y) - (flowaccum.BottomRight().y);
    double bottom = (flowaccum.BottomRight().y);
    double left = (flowaccum.TopLeft().x);

    std::vector<std::vector<Vec2>> flowpaths;

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

void CatchmentBuilder::CatchmentClassification(QuadtreeManager<FlowGeneral>& catchclass, QuadtreeManager<FlowDirection>& flowdirection, std::vector<DischargePoint> dischargepoints)
{
    std::cout << "Classifying Catchment Areas\n";

    double boundsx = (catchclass.BottomRight().x) - (catchclass.TopLeft().x);
    double boundsy = (catchclass.TopLeft().y) - (catchclass.BottomRight().y);
    double bottom = (catchclass.BottomRight().y);
    double left = (catchclass.TopLeft().x);

    //Initialise the accumulation grid at 1
    for (int x = 0; x <= boundsx; x++)
    {
        for (int y = 0; y <= boundsy; y++)
        {

            if (flowdirection.Search(FlowDirection(x + left, y + bottom)) != nullptr)
                catchclass.Insert(new Node<FlowGeneral>(FlowGeneral(x + left, y + bottom, 0)));
        }
    }

    FileWriter::WriteVec2Points("./Exports/Points/DischargePoints2dWKT", dischargepoints);

    std::reverse(dischargepoints.begin(), dischargepoints.end());

    for (int y = 0; y <= boundsy; y++)
    {
        for (int x = 0; x <= boundsx; x++)
        {
            ClassifyFlowPath(catchclass, flowdirection, dischargepoints, Vec2(x + left, y + bottom));
        }
    }
}

std::vector<DischargePoint> CatchmentBuilder::GenerateDischargePoints(std::vector<FlowPath>& fps, int breakdist)
{
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
        if (length > breakdist) //if the length is less than the breakdist then don't process any further
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
                if (insert)
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
        if (unique)
            temppoints.push_back(dischargepoints[i]);
    }
    dischargepoints = temppoints;

    //Asign point indexes
    for (int j = 1; j < dischargepoints.size() + 1; j++)
    {
        dischargepoints[j - 1].index = j;
    }

    return dischargepoints;
}

void CatchmentBuilder::ClassifyFlowPath(QuadtreeManager<FlowGeneral>& catchclass, QuadtreeManager<FlowDirection>& flowdirection, std::vector<DischargePoint> dischargepoints, Vec2 point)
{
    //Trace for each x y coord
    //store catchclass nodes in vector
    //when discharge point is reached
    //get the id and apply to all nodes in the vector
    //if node already has id then skip it

    int exitcond = 0; //if 0 not reached exit, if 1 discharge point found, if 2 nullptr found, if 3 circular flowpath found
    int flowpathid = 0;
    std::vector<Node<FlowGeneral>*> path;
    
    double i = point.x;
    double j = point.y;

    auto d = flowdirection.Search(FlowDirection(i, j));
    auto c = catchclass.Search(FlowGeneral(i, j));

    while (exitcond == 0)
    {
        path.push_back(c);

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

        d = flowdirection.Search(FlowDirection(i, j));
        c = catchclass.Search(FlowGeneral(i, j));

        if (d == nullptr || c == nullptr)
        {
            exitcond = 2;
            break;
        }

        if (c->pos.iValue != 0)
        {
            exitcond = 1;
            flowpathid = c->pos.iValue;
        }


        for (size_t a = 0; a < dischargepoints.size(); a++)
        {
            if (dischargepoints[a].location == Vec2(i, j))
            {
                exitcond = 1;
                flowpathid = dischargepoints[a].index;
                break;
            }
        }

        for (size_t a = 0; a < path.size(); a++)
        {
            double comp1 = std::abs(path[a]->pos.x - i);
            double comp2 = std::abs(path[a]->pos.y - j);
            if ( comp1 < 0.0001 &&  comp2 < 0.0001)
            {
                exitcond = 3;
            }
        }
    }

    if (exitcond == 1)
    {
        for (size_t i = 0; i < path.size(); i++)
        {
            path[i]->pos.iValue = flowpathid;
        }
    }

    for (size_t i = 0; i < path.size(); i++)
    {
        path[i] = nullptr;
    }

    path.clear();

}

void CatchmentBuilder::PolygoniseCatchments(QuadtreeManager<FlowGeneral>& catchclass, std::vector<DischargePoint> dischargepoints, std::vector<Catchment>& catchlist)
{
    std::cout << "Polygonising Catchment Areas\n";

    for each (DischargePoint dispoint in dischargepoints)
    {
        double boundsx = (catchclass.BottomRight().x) - (catchclass.TopLeft().x);
        double boundsy = (catchclass.TopLeft().y) - (catchclass.BottomRight().y);
        double bottom = (catchclass.BottomRight().y);
        double left = (catchclass.TopLeft().x);
        //narrow down the bounds to the catchment to avoid 

        MinMax catchmentMM;

        for (int x = 0; x <= boundsx; x++)
            for (int y = 0; y <= boundsy; y++)
            {
                auto node = catchclass.Search(FlowGeneral(x + left, y + bottom));
                if (node != nullptr && node->pos.iValue == dispoint.index)
                {
                    if (x > catchmentMM.maxx)
                        catchmentMM.maxx = x;

                    if (y > catchmentMM.maxy)
                        catchmentMM.maxy = y;

                    if (x < catchmentMM.minx)
                        catchmentMM.minx = x;

                    if (y < catchmentMM.miny)
                        catchmentMM.miny = y;
                }
            }

        auto topLeft = FlowGeneral(catchmentMM.minx, catchmentMM.maxy);
        auto bottomRight = FlowGeneral(catchmentMM.maxx, catchmentMM.miny);

        bottom = catchmentMM.miny;
        left = catchmentMM.minx;
        boundsx = catchmentMM.maxx - catchmentMM.minx;
        boundsy = catchmentMM.maxy - catchmentMM.miny;
        
        QuadtreeManager<FlowGeneral> catchment(topLeft, bottomRight);

        catchment.prePath = "Temp/Catchment/Tree";
        catchment.spacing = catchclass.spacing;
        catchment.splitlevel = 0;
        catchment.SetTreeType(TreeType::Single);

        //Copy all nodes with the matching catchment ID to a new tree for faster read write
        for (int x = 0; x <= boundsx; x++)
            for (int y = 0; y <= boundsy; y++)
            {
                auto node = catchclass.Search(FlowGeneral(x + left, y + bottom));
                if(node != nullptr && node->pos.iValue == dispoint.index)
                    catchment.Insert(new Node<FlowGeneral>(node->pos));
            }

        

        catchment.~QuadtreeManager();
    }
}


