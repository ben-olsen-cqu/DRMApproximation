#include "../Headers/CatchmentBuilder.h"
#include "../Headers/FileWriter.h"
#include "../Headers/FileReader.h"

#include <iostream>
#include <iomanip>
#include <algorithm>

#define PI 3.14159265

std::vector<Catchment> CatchmentBuilder::CreateCatchments(ProgamParams progp)
{
    QuadtreeManager<Coordinates> quad;
    int blurrad = 3;           //pre-processing radius always odd no.
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
        if (quad.type == TreeType::Single)
        {
            std::cout << "Exporting Original Surface\n";
            FileWriter::WriteCoordTreeASC("./Exports/Surfaces/Original", quad);
        }
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
            SmoothPointsSingle(quad, smooth, blurrad);
            std::cout << "Exporting Smoothed Surface\n";
            FileWriter::WriteCoordTreeASC("./Exports/Surfaces/Smooth", smooth);
        }
        else
        {
            SmoothPointsSplit(quad, smooth, blurrad);
            std::cout << "Exporting Smoothed Surface\n";
            FileWriter::WriteCoordTreeASC("./Exports/Surfaces/Smooth", smooth);
        }
        smooth.WriteManagerToFile();

    }

    quad.~QuadtreeManager();

    Normal tL(smooth.topLeft.x + smooth.spacing / 2, smooth.topLeft.y - smooth.spacing / 2);
    Normal bR(smooth.bottomRight.x - smooth.spacing / 2, smooth.bottomRight.y + smooth.spacing / 2);

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
        normal.spacing = smooth.spacing;
        normal.splitlevel = smooth.splitlevel;
        normal.SetTreeType(smooth.type);

        if (quad.type == TreeType::Single)
        {
            CalculateNormalsSingle(smooth, normal);
            std::cout << "Writing Normals to File.\n";
            //FileWriter::WriteVecNormals3dWKT("./Exports/Vectors/SmoothNormals3dWKT", normal);
            FileWriter::WriteVecNormals2dWKT("./Exports/Vectors/SmoothNormals2dWKT", normal);
        }
        else
        {
            CalculateNormalsSplit(smooth, normal);
        }
        normal.WriteManagerToFile();
    }

    smooth.~QuadtreeManager();

    FlowDirection tL2(normal.topLeft.x, normal.topLeft.y);
    FlowDirection bR2(normal.bottomRight.x, normal.bottomRight.y);

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
            std::cout << "Writing Flow Directions to File.\n";
            FileWriter::WriteFlowDirection2dWKT("./Exports/Vectors/FlowDirections2dWKT", flowdirection);
        }
        else
        {
            CalculateFlowDirectionSplit(flowdirection, normal);
        }

        flowdirection.WriteManagerToFile();
    }

    normal.~QuadtreeManager();

    FlowGeneral tL3(flowdirection.topLeft.x, flowdirection.topLeft.y);
    FlowGeneral bR3(flowdirection.bottomRight.x, flowdirection.bottomRight.y);

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
            std::cout << "Exporting Flow Accumulation Surface\n";
            FileWriter::WriteFlowGeneralTreeASC("./Exports/Surfaces/Accum", flowaccum);
        }
        else
        {
            CalculateFlowAccumulationSplit(flowdirection, flowaccum);
        }

        flowaccum.WriteManagerToFile();
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
            flowpaths = StreamLinkingSplit(flowaccum, flowdirection, acctarget);
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

        //std::cout << "Exporting Classified Catchment Surface\n";
        //FileWriter::WriteFlowGeneralTreeASC("./Exports/Surfaces/CatchmentClassification", catchclass);
    }

    flowpaths.clear();

    std::vector<Catchment> catchlist;

    //Combine the polygonisation, longest fps, catchment params and isochrone generation into 1 function and iterate per catchment

    CatchmentProperties(catchlist, dischargepoints, catchclass, flowdirection);
    std::cout << "Exporting Catchment Polygons\n";
    FileWriter::WriteCatchmentPolysWKT("./Exports/Polygons/Catchments", catchlist);

    std::cout << "Exporting Longest Flow Paths\n";
    FileWriter::WriteStreamPaths2dWKT("./Exports/Vectors/LongestFlowPaths2dWKT", catchlist);

    catchclass.~QuadtreeManager();
    flowdirection.~QuadtreeManager();

    FileWriter::WriteCatchmentstoBinary("Temp/Catchments/Catchments",catchlist);

    return catchlist;
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
    {
        std::cout << std::fixed << std::setprecision(2) << "\r" << y / boundsy * 100 << "% Complete";
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
    std::cout << "\n";
}

void CatchmentBuilder::SmoothPointsSplit(QuadtreeManager<Coordinates>& quad, QuadtreeManager<Coordinates>& smooth,int blurrad)
{
    std::cout << "Smoothing Surface\n";

    int storenum = (blurrad - 1) / 2;
    
    ////Create quadtree for storing edge cases
    QuadtreeManager<Coordinates> gaps(quad.topLeft, quad.bottomRight);
    gaps.prePath = "Temp/SmoothTree/GapTree";
    gaps.spacing = quad.spacing;
    gaps.splitlevel = quad.splitlevel - 2;
    gaps.SetTreeType(quad.type);

    double boundsx = (quad.BottomRight().x) - (quad.TopLeft().x);
    double boundsy = (quad.TopLeft().y) - (quad.BottomRight().y);
    double bottom = (quad.BottomRight().y);
    double left = (quad.TopLeft().x);

    int numquads = std::pow(2, quad.splitlevel); //quad splits the area in half in the x and y axis
    int totalquads = numquads * numquads; //total number of sub quads at the split level

    double boundspersubquadx = boundsx / numquads; //the equal division of space in the x axis for the split level
    double boundspersubquady = boundsy / numquads; //the equal division of space in the y axis for the split level

    //for loops for iterating through split level trees
    for (int v = 0; v < numquads; v++) //move vertically through sub trees
    {
        //Calculate the offset to add when seaching within the loop, the +1 is to push it into the next subquad if subquad is the bottom left no offset is required
        //int offsety = v == 0 ? 0 : std::floor(boundspersubquady * v)+1;

        int offsety;
        if (v == 0)
        {
            offsety = 0; //confirmed
            boundspersubquady = 882; //confirmed
        }
        if (v == 1)
        {
            offsety = 882; //confirmed
            boundspersubquady = 882; //confirmed
        }
        if (v == 2)
        {
            offsety = 1764; //confirmed
            boundspersubquady = 881; //confirmed
        }
        if (v == 3)
        {
            offsety = 2645; //confirmed
            boundspersubquady = 882; //confirmed
        }
        if (v == 4)
        {
            offsety = 3527; //confirmed
            boundspersubquady = 881; //confirmed
        }
        if (v == 5)
        {
            offsety = 4408; //confirmed
            boundspersubquady = 882; //confirmed
        }
        if (v == 6)
        {
            offsety = 5290;
            boundspersubquady = 881;
        }
        if (v == 7)
        {
            offsety = 6171;
            boundspersubquady = 882;
        }

        for (int w = 0; w < numquads; w++) //move horizontally through sub trees
        {
            //Same as the y offset
            //int offsetx = w == 0 ? 0 : std::floor(boundspersubquadx * w)+1;

            int offsetx;
            if (w == 0) //Quad 1
            {
                offsetx = 0;
                boundspersubquadx = 882;
            }
            if (w == 1) //Quad 2
            {
                offsetx = 882;
                boundspersubquadx = 881;
            }
            if (w == 2) //Quad 3
            {
                offsetx = 1763;
                boundspersubquadx = 882;
            }
            if (w == 3) //Quad 4
            {
                offsetx = 2645;
                boundspersubquadx = 881;
            }
            if (w == 4) //Quad 5
            {
                offsetx = 3526;
                boundspersubquadx = 882;
            }
            if (w == 5) //Quad 6
            {
                offsetx = 4408;
                boundspersubquadx = 881;
            }
            if (w == 6) //Quad 7
            {
                offsetx = 5289;
                boundspersubquadx = 882;
            }
            if (w == 7) //Quad 8
            {
                offsetx = 6171;
                boundspersubquadx = 882;
            }
            std::cout << "\rProcessing Quad " << v * numquads + (w + 1) << " of " << totalquads;

            for (int y = 0; y < boundspersubquady; y++) //move through each coord in the y direction of the subtree
            {
                for (int x = 0; x < boundspersubquadx; x++)//move through each coord in the x direction of the subtree
                {
                    //if (y == 0 && x == 0)
                    //    std::cout << std::fixed << "\nStart of line: " << x + offsetx + left << "\n";

                    //if (y == 0 && x >= boundspersubquadx - 1)
                    //    std::cout << std::fixed << "\nEnd of line: " << x + offsetx + left << "\n";

                    if (y < storenum || (boundspersubquady - y) <= storenum || x < storenum || (boundspersubquadx - x) <= storenum)
                    {
                        Node<Coordinates>* node = quad.Search(Coordinates(x + offsetx + left, y + offsety + bottom));

                        if (node != nullptr)
                        {
                            Coordinates coord = node->pos;
                            gaps.Insert(new Node<Coordinates>(coord));
                        }
                    }
                    else
                    {
                        Node<Coordinates>* node = quad.Search(Coordinates(x + offsetx + left, y + offsety + bottom));

                        if (node != nullptr)
                        {
                            Coordinates coord = node->pos;

                            std::vector<Coordinates> vecCoords;

                            for (int j = -storenum; j <= storenum; j++)
                                for (int i = -storenum; i <= storenum; i++)
                                {
                                    Node<Coordinates>* n = quad.Search(Coordinates((i + x) + offsetx + left, (j + y) + offsety + bottom));

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

        }
    }

    smooth.Cleanup();
    gaps.Cleanup();
    quad.Cleanup();

    std::cout << "\nComplete\nFilling Gaps...\n";

    //for loops for iterating through split level trees
    for (int v = 0; v < numquads; v++) //move vertically through sub trees
    {
        //Calculate the offset to add when seaching within the loop, the +1 is to push it into the next subquad if subquad is the bottom left no offset is required
                //int offsety = v == 0 ? 0 : std::floor(boundspersubquady * v)+1;

        int offsety;
        if (v == 0)
        {
            offsety = 0; //confirmed
            boundspersubquady = 882; //confirmed
        }
        if (v == 1)
        {
            offsety = 882; //confirmed
            boundspersubquady = 882; //confirmed
        }
        if (v == 2)
        {
            offsety = 1764; //confirmed
            boundspersubquady = 881; //confirmed
        }
        if (v == 3)
        {
            offsety = 2645; //confirmed
            boundspersubquady = 882; //confirmed
        }
        if (v == 4)
        {
            offsety = 3527; //confirmed
            boundspersubquady = 881; //confirmed
        }
        if (v == 5)
        {
            offsety = 4408; //confirmed
            boundspersubquady = 882; //confirmed
        }
        if (v == 6)
        {
            offsety = 5290;
            boundspersubquady = 881;
        }
        if (v == 7)
        {
            offsety = 6171;
            boundspersubquady = 882;
        }

        for (int w = 0; w < numquads; w++) //move horizontally through sub trees
        {
            //Same as the y offset
                        //int offsetx = w == 0 ? 0 : std::floor(boundspersubquadx * w)+1;

            int offsetx;
            if (w == 0) //Quad 1
            {
                offsetx = 0;
                boundspersubquadx = 882;
            }
            if (w == 1) //Quad 2
            {
                offsetx = 882;
                boundspersubquadx = 881;
            }
            if (w == 2) //Quad 3
            {
                offsetx = 1763;
                boundspersubquadx = 882;
            }
            if (w == 3) //Quad 4
            {
                offsetx = 2645;
                boundspersubquadx = 881;
            }
            if (w == 4) //Quad 5
            {
                offsetx = 3526;
                boundspersubquadx = 882;
            }
            if (w == 5) //Quad 6
            {
                offsetx = 4408;
                boundspersubquadx = 881;
            }
            if (w == 6) //Quad 7
            {
                offsetx = 5289;
                boundspersubquadx = 882;
            }
            if (w == 7) //Quad 8
            {
                offsetx = 6171;
                boundspersubquadx = 882;
            }

            std::cout << "\rProcessing Quad " << v * numquads + (w + 1) << " of " << totalquads;

            //Top
            for (int x = 0; x < boundspersubquadx; x++)
                for (int y = boundspersubquady - storenum-1; y < boundspersubquady; y++)
                {
                    Node<Coordinates>* node = gaps.Search(Coordinates(x + offsetx + left, y + offsety + bottom));

                    if (node != nullptr)
                    {
                        Coordinates coord = node->pos;

                        std::vector<Coordinates> vecCoords;

                        for (int j = -storenum; j <= storenum; j++)
                            for (int i = -storenum; i <= storenum; i++)
                            {
                                Node<Coordinates>* n = gaps.Search(Coordinates((i + x) + offsetx + left, (j + y) + offsety + bottom));

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
            for (int x = 0; x < boundspersubquadx; x++)
                for (int y = 0; y <= storenum; y++)
                {
                    Node<Coordinates>* node = gaps.Search(Coordinates(x + offsetx + left, y + offsety + bottom));

                    if (node != nullptr)
                    {
                        Coordinates coord = node->pos;

                        std::vector<Coordinates> vecCoords;

                        for (int j = -storenum; j <= storenum; j++)
                            for (int i = -storenum; i <= storenum; i++)
                            {
                                Node<Coordinates>* n = gaps.Search(Coordinates((i + x) + offsetx + left, (j + y) + offsety + bottom));

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

            for (int x = boundspersubquadx - storenum-1; x < boundspersubquadx; x++)
                for (int y = storenum; y < boundspersubquady - storenum; y++)
                {
                    Node<Coordinates>* node = gaps.Search(Coordinates(x + offsetx + left, y + offsety + bottom));

                    if (node != nullptr)
                    {
                        Coordinates coord = node->pos;

                        std::vector<Coordinates> vecCoords;

                        for (int j = -storenum; j <= storenum; j++)
                            for (int i = -storenum; i <= storenum; i++)
                            {
                                Node<Coordinates>* n = gaps.Search(Coordinates((i + x) + offsetx + left, (j + y) + offsety + bottom));

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
            for (int x = 0; x <= storenum; x++)
                for (int y = 0; y < boundspersubquady; y++)
                {
                    Node<Coordinates>* node = gaps.Search(Coordinates(x + offsetx + left, y + offsety + bottom));

                    if (node != nullptr)
                    {
                        Coordinates coord = node->pos;

                        std::vector<Coordinates> vecCoords;

                        for (int j = -storenum; j <= storenum; j++)
                            for (int i = -storenum; i <= storenum; i++)
                            {
                                Node<Coordinates>* n = gaps.Search(Coordinates((i + x) + offsetx + left, (j + y) + offsety + bottom));

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

            auto c = smooth.Search(Coordinates(x + left, y + bottom));

            if (c == nullptr)
                continue;

            p1 = Vec3(c->pos.x, c->pos.y, c->pos.z);

            c = smooth.Search(Coordinates(x + left, y + bottom + 1));

            if (c == nullptr)
                continue;

            p2 = Vec3(c->pos.x, c->pos.y, c->pos.z);

            c = smooth.Search(Coordinates(x + left + 1, y + bottom + 1));

            if (c == nullptr)
                continue;

            p3 = Vec3(c->pos.x, c->pos.y, c->pos.z);

            c = smooth.Search(Coordinates(x + left + 1, y + bottom));

            if (c == nullptr)
                continue;

            p4 = Vec3(c->pos.x, c->pos.y, c->pos.z);
            
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

void CatchmentBuilder::CalculateNormalsSplit(QuadtreeManager<Coordinates>& smooth, QuadtreeManager<Normal>& normal)
{
    std::cout << "Generating Normals\n";

    ////Create quadtree for storing edge cases
    QuadtreeManager<Coordinates> gaps(smooth.topLeft, smooth.bottomRight);
    gaps.prePath = "Temp/NormalTree/GapTree";
    gaps.spacing = smooth.spacing;
    gaps.splitlevel = smooth.splitlevel - 2;
    gaps.SetTreeType(smooth.type);

    double boundsx = (smooth.BottomRight().x) - (smooth.TopLeft().x);
    double boundsy = (smooth.TopLeft().y) - (smooth.BottomRight().y);
    double bottom = (smooth.BottomRight().y);
    double left = (smooth.TopLeft().x);

    int numquads = std::pow(2, smooth.splitlevel); //quad splits the area in half in the x and y axis
    int totalquads = numquads * numquads; //total number of sub quads at the split level

    double boundspersubquadx = boundsx / numquads; //the equal division of space in the x axis for the split level
    double boundspersubquady = boundsy / numquads; //the equal division of space in the y axis for the split level

    //for loops for iterating through split level trees
    for (int v = 0; v < numquads; v++) //move vertically through sub trees
    {
        //Calculate the offset to add when seaching within the loop, the +1 is to push it into the next subquad if subquad is the bottom left no offset is required
        //int offsety = v == 0 ? 0 : std::floor(boundspersubquady * v)+1;

        int offsety;
        if (v == 0)
        {
            offsety = 0; //confirmed
            boundspersubquady = 882; //confirmed
        }
        if (v == 1)
        {
            offsety = 882; //confirmed
            boundspersubquady = 882; //confirmed
        }
        if (v == 2)
        {
            offsety = 1764; //confirmed
            boundspersubquady = 881; //confirmed
        }
        if (v == 3)
        {
            offsety = 2645; //confirmed
            boundspersubquady = 882; //confirmed
        }
        if (v == 4)
        {
            offsety = 3527; //confirmed
            boundspersubquady = 881; //confirmed
        }
        if (v == 5)
        {
            offsety = 4408; //confirmed
            boundspersubquady = 882; //confirmed
        }
        if (v == 6)
        {
            offsety = 5290;
            boundspersubquady = 881;
        }
        if (v == 7)
        {
            offsety = 6171;
            boundspersubquady = 882;
        }

        for (int w = 0; w < numquads; w++) //move horizontally through sub trees
        {
            //Same as the y offset
            //int offsetx = w == 0 ? 0 : std::floor(boundspersubquadx * w)+1;

            int offsetx;
            if (w == 0) //Quad 1
            {
                offsetx = 0;
                boundspersubquadx = 882;
            }
            if (w == 1) //Quad 2
            {
                offsetx = 882;
                boundspersubquadx = 881;
            }
            if (w == 2) //Quad 3
            {
                offsetx = 1763;
                boundspersubquadx = 882;
            }
            if (w == 3) //Quad 4
            {
                offsetx = 2645;
                boundspersubquadx = 881;
            }
            if (w == 4) //Quad 5
            {
                offsetx = 3526;
                boundspersubquadx = 882;
            }
            if (w == 5) //Quad 6
            {
                offsetx = 4408;
                boundspersubquadx = 881;
            }
            if (w == 6) //Quad 7
            {
                offsetx = 5289;
                boundspersubquadx = 882;
            }
            if (w == 7) //Quad 8
            {
                offsetx = 6171;
                boundspersubquadx = 882;
            }
            std::cout << "\rProcessing Quad " << v * numquads + (w + 1) << " of " << totalquads;

            for (int y = 0; y < boundspersubquady; y++) //move through each coord in the y direction of the subtree
                for (int x = 0; x < boundspersubquadx; x++)//move through each coord in the x direction of the subtree
                {
                    if (y < 1 || (boundspersubquady - y) <= 1 || x < 1 || (boundspersubquadx - x) <= 1)
                    {
                        Node<Coordinates>* node = smooth.Search(Coordinates(x + offsetx + left, y + offsety + bottom));

                        if (node != nullptr)
                        {
                            Coordinates coord = node->pos;
                            gaps.Insert(new Node<Coordinates>(coord));
                        }
                    }
                    
                    if(x <= boundspersubquadx-1 && y <= boundspersubquady-1)
                    {
                        //Get points in a quad going clockwise starting from the BL

                        Vec3 p1, p2, p3, p4, vec1, vec2, vec3, vec4, translation, normal1, normal2;

                        auto c = smooth.Search(Coordinates(x + offsetx + left, y + offsety + bottom));

                        if (c == nullptr)
                            continue;

                        p1 = Vec3(c->pos.x, c->pos.y, c->pos.z);

                        c = smooth.Search(Coordinates(x + offsetx + left, y + offsety + bottom + 1));

                        if (c == nullptr)
                            continue;

                        p2 = Vec3(c->pos.x, c->pos.y, c->pos.z);

                        c = smooth.Search(Coordinates(x + offsetx + left + 1, y + offsety + bottom + 1));

                        if (c == nullptr)
                            continue;

                        p3 = Vec3(c->pos.x, c->pos.y, c->pos.z);

                        c = smooth.Search(Coordinates(x + offsetx + left + 1, y + offsety + bottom));

                        if (c == nullptr)
                            continue;

                        p4 = Vec3(c->pos.x, c->pos.y, c->pos.z);

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
        }
    }
    smooth.Cleanup();
    normal.Cleanup();

    std::cout << "\nComplete\nFilling Gaps...\n";

    //for loops for iterating through split level trees
    for (int v = 0; v < numquads; v++) //move vertically through sub trees
    {
        //Calculate the offset to add when seaching within the loop, the +1 is to push it into the next subquad if subquad is the bottom left no offset is required
        //int offsety = v == 0 ? 0 : std::floor(boundspersubquady * v)+1;

        int offsety;
        if (v == 0)
        {
            offsety = 0; //confirmed
            boundspersubquady = 882; //confirmed
        }
        if (v == 1)
        {
            offsety = 882; //confirmed
            boundspersubquady = 882; //confirmed
        }
        if (v == 2)
        {
            offsety = 1764; //confirmed
            boundspersubquady = 881; //confirmed
        }
        if (v == 3)
        {
            offsety = 2645; //confirmed
            boundspersubquady = 882; //confirmed
        }
        if (v == 4)
        {
            offsety = 3527; //confirmed
            boundspersubquady = 881; //confirmed
        }
        if (v == 5)
        {
            offsety = 4408; //confirmed
            boundspersubquady = 882; //confirmed
        }
        if (v == 6)
        {
            offsety = 5290;
            boundspersubquady = 881;
        }
        if (v == 7)
        {
            offsety = 6171;
            boundspersubquady = 882;
        }
        for (int w = 0; w < numquads; w++) //move horizontally through sub trees
        {
            //Same as the y offset
            //int offsetx = w == 0 ? 0 : std::floor(boundspersubquadx * w)+1;

            int offsetx;
            if (w == 0) //Quad 1
            {
                offsetx = 0;
                boundspersubquadx = 882;
            }
            if (w == 1) //Quad 2
            {
                offsetx = 882;
                boundspersubquadx = 881;
            }
            if (w == 2) //Quad 3
            {
                offsetx = 1763;
                boundspersubquadx = 882;
            }
            if (w == 3) //Quad 4
            {
                offsetx = 2645;
                boundspersubquadx = 881;
            }
            if (w == 4) //Quad 5
            {
                offsetx = 3526;
                boundspersubquadx = 882;
            }
            if (w == 5) //Quad 6
            {
                offsetx = 4408;
                boundspersubquadx = 881;
            }
            if (w == 6) //Quad 7
            {
                offsetx = 5289;
                boundspersubquadx = 882;
            }
            if (w == 7) //Quad 8
            {
                offsetx = 6171;
                boundspersubquadx = 882;
            }
            std::cout << "\rProcessing Quad " << v * numquads + (w + 1) << " of " << totalquads;

            //Top
            int y = boundspersubquady - 1;
            for (int x = 0; x <= boundspersubquadx; x++)
            {
                //Get points in a quad going clockwise starting from the BL

                Vec3 p1, p2, p3, p4, vec1, vec2, vec3, vec4, translation, normal1, normal2;

                auto c = gaps.Search(Coordinates(x + offsetx + left, y + offsety + bottom));

                if (c == nullptr)
                    continue;

                p1 = Vec3(c->pos.x, c->pos.y, c->pos.z);

                c = gaps.Search(Coordinates(x + offsetx + left, y + offsety + bottom + 1));

                if (c == nullptr)
                    continue;

                p2 = Vec3(c->pos.x, c->pos.y, c->pos.z);

                c = gaps.Search(Coordinates(x + offsetx + left + 1, y + offsety + bottom + 1));

                if (c == nullptr)
                    continue;

                p3 = Vec3(c->pos.x, c->pos.y, c->pos.z);

                c = gaps.Search(Coordinates(x + offsetx + left + 1, y + offsety + bottom));

                if (c == nullptr)
                    continue;

                p4 = Vec3(c->pos.x, c->pos.y, c->pos.z);

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

            //Right
            int x = boundspersubquadx - 1;
            for (int y = 0; y <= boundspersubquady; y++)
            {
                //Get points in a quad going clockwise starting from the BL

                Vec3 p1, p2, p3, p4, vec1, vec2, vec3, vec4, translation, normal1, normal2;

                auto c = gaps.Search(Coordinates(x + offsetx + left, y + offsety + bottom));

                if (c == nullptr)
                    continue;

                p1 = Vec3(c->pos.x, c->pos.y, c->pos.z);

                c = gaps.Search(Coordinates(x + offsetx + left, y + offsety + bottom + 1));

                if (c == nullptr)
                    continue;

                p2 = Vec3(c->pos.x, c->pos.y, c->pos.z);

                c = gaps.Search(Coordinates(x + offsetx + left + 1, y + offsety + bottom + 1));

                if (c == nullptr)
                    continue;

                p3 = Vec3(c->pos.x, c->pos.y, c->pos.z);

                c = gaps.Search(Coordinates(x + offsetx + left + 1, y + offsety + bottom));

                if (c == nullptr)
                    continue;

                p4 = Vec3(c->pos.x, c->pos.y, c->pos.z);

                double avz = (p1.z + p2.z + p3.z + p4.z) / 4;
                translation = Vec3(p1.x + smooth.spacing / 2, p1.y + smooth.spacing / 2, avz);

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

                //normal.Insert(new Node<Normal>(Normal(translation, normalq)));
            }

        }
    }

    std::cout << "\n";
    gaps.~QuadtreeManager();
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

void CatchmentBuilder::CalculateFlowDirectionSplit(QuadtreeManager<FlowDirection>& flowdirection, QuadtreeManager<Normal>& normal)
{
    std::cout << "Calculating Flow Directions\n";

    double boundsx = (normal.BottomRight().x) - (normal.TopLeft().x);
    double boundsy = (normal.TopLeft().y) - (normal.BottomRight().y);
    double bottom = (normal.BottomRight().y);
    double left = (normal.TopLeft().x);

    int numquads = std::pow(2, normal.splitlevel); //quad splits the area in half in the x and y axis
    int totalquads = numquads * numquads; //total number of sub quads at the split level

    double boundspersubquadx = (boundsx / numquads); //the equal division of space in the x axis for the split level
    double boundspersubquady = (boundsy / numquads); //the equal division of space in the y axis for the split level

    //for loops for iterating through split level trees
    for (int v = 0; v < numquads; v++) //move vertically through sub trees
    {
        //Calculate the offset to add when seaching within the loop, the +1 is to push it into the next subquad if subquad is the bottom left no offset is required
        //int offsety = v == 0 ? 0 : std::floor(boundspersubquady * v)+1;

        int offsety;
        if (v == 0)
        {
            offsety = 0; //confirmed
            boundspersubquady = 882; //confirmed
        }
        if (v == 1)
        {
            offsety = 882; //confirmed
            boundspersubquady = 882; //confirmed
        }
        if (v == 2)
        {
            offsety = 1764; //confirmed
            boundspersubquady = 881; //confirmed
        }
        if (v == 3)
        {
            offsety = 2645; //confirmed
            boundspersubquady = 882; //confirmed
        }
        if (v == 4)
        {
            offsety = 3527; //confirmed
            boundspersubquady = 881; //confirmed
        }
        if (v == 5)
        {
            offsety = 4408; //confirmed
            boundspersubquady = 882; //confirmed
        }
        if (v == 6)
        {
            offsety = 5290;
            boundspersubquady = 881;
        }
        if (v == 7)
        {
            offsety = 6171;
            boundspersubquady = 882;
        }

        for (int w = 0; w < numquads; w++) //move horizontally through sub trees
        {
            //Same as the y offset
            //int offsetx = w == 0 ? 0 : std::floor(boundspersubquadx * w)+1;

            int offsetx;
            if (w == 0)
            {
                offsetx = 0;
                boundspersubquadx = 1763;
            }
            if (w == 1)
            {
                offsetx = 1764;
                boundspersubquadx = 3526;
            }
            if (w == 2)
            {
                offsetx = 3527;
                boundspersubquadx = 5289;
            }
            if (w == 3)
            {
                offsetx = 5290;
                boundspersubquadx = 7052;
            }

            std::cout << "\nProcessing Quad " << v * numquads + (w + 1) << " of " << totalquads << "\n";

            for (int y = 0; y < boundspersubquady; y++) //move through each coord in the y direction of the subtree
            {
                for (int x = 0; x < boundspersubquadx; x++)//move through each coord in the x direction of the subtree
                {
                    if (y == 0 && x == 0)
                        std::cout << "Start of line: " << x + offsetx + left << "\n";

                    if (y == 0 && x >= boundspersubquadx-1)
                        std::cout << "End of line: " << x + offsetx + left << "\n";

                    auto f = normal.Search(Normal(x + offsetx + left, y + offsety + bottom));

                    if (f != nullptr)
                    {
                        auto n = f->pos;

                        Vec2 translated(n.norm.x - n.x, n.norm.y - n.y);

                        float angle = std::atan2(translated.x, translated.y);

                        int octant = (int)std::round(8 * angle / (2 * PI) + 8) % 8;

                        Direction dir = (Direction)octant;

                        flowdirection.Insert(new Node<FlowDirection>(FlowDirection(n.x, n.y, dir)));
                    }
                }
            }
        }
    }
    std::cout << "\nComplete\n";
}

void CatchmentBuilder::CalculateFlowAccumulationSingle(QuadtreeManager<FlowDirection>& flowdirection, QuadtreeManager<FlowGeneral>& flowaccum)
{
    std::cout << "Calculating Flow Accumulations\n";

    QuadtreeManager<FlowGeneral> NIDP(flowaccum.TopLeft(), flowaccum.BottomRight());

    //  NIDP quadtree for storing the number of cells that flow into a given cell
    //  If the NIDP value is 0 then the cell is a source cell and is the top of the flow path
    //  If the NIDP value is 1 then the cell is an interior cell and simply take the flow and passes it to the next cell
    //  If the NIDP value is >=2 then the cell is an intersectionm of flow paths
    

    NIDP.prePath = "Temp/AccumulationTree/NIDPTree";
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

void CatchmentBuilder::CalculateFlowAccumulationSplit(QuadtreeManager<FlowDirection>& flowdirection, QuadtreeManager<FlowGeneral>& flowaccum)
{
    std::cout << "Calculating NIDP Grid\n";

    //initialise the flow accumulation grid at 1

    double boundsx = (flowdirection.BottomRight().x) - (flowdirection.TopLeft().x);
    double boundsy = (flowdirection.TopLeft().y) - (flowdirection.BottomRight().y);
    double bottom = (flowdirection.BottomRight().y);
    double left = (flowdirection.TopLeft().x);

    int numquads = std::pow(2, flowdirection.splitlevel); //quad splits the area in half in the x and y axis
    int totalquads = numquads * numquads; //total number of sub quads at the split level

    double boundspersubquadx = (boundsx / numquads); //the equal division of space in the x axis for the split level
    double boundspersubquady = (boundsy / numquads); //the equal division of space in the y axis for the split level

    //for loops for iterating through split level trees
    for (int v = 0; v < numquads; v++) //move vertically through sub trees
    {
        //Calculate the offset to add when seaching within the loop, the +1 is to push it into the next subquad if subquad is the bottom left no offset is required
        //int offsety = v == 0 ? 0 : std::floor(boundspersubquady * v)+1;

        int offsety;
        if (v == 0)
        {
            offsety = 0;
            boundspersubquady = 1000;
        }
        if (v == 1)
        {
            offsety = 1000;
            boundspersubquady = 999;
        }
        if (v == 2)
        {
            offsety = 1999;
            boundspersubquady = 1000;
        }
        if (v == 3)
        {
            offsety = 2999;
            boundspersubquady = 999;
        }

        for (int w = 0; w < numquads; w++) //move horizontally through sub trees
        {
            //Same as the y offset
            //int offsetx = w == 0 ? 0 : std::floor(boundspersubquadx * w)+1;

            int offsetx;
            if (w == 0)
            {
                offsetx = 0;
                boundspersubquadx = 1000;
            }
            if (w == 1)
            {
                offsetx = 1000;
                boundspersubquadx = 999;
            }
            if (w == 2)
            {
                offsetx = 1999;
                boundspersubquadx = 1000;
            }
            if (w == 3)
            {
                offsetx = 2999;
                boundspersubquadx = 999;
            }

            std::cout << "\nProcessing Quad " << v * numquads + (w + 1) << " of " << totalquads << "\n";

            for (int y = 0; y < boundspersubquady; y++) //move through each coord in the y direction of the subtree
            {
                for (int x = 0; x < boundspersubquadx; x++)//move through each coord in the x direction of the subtree
                {
                    flowaccum.Insert(new Node<FlowGeneral>(FlowGeneral(x + offsetx + left, y + offsety + bottom, 1)));
                }
            }
        }
    }
        

    //  NIDP quadtree for storing the number of cells that flow into a given cell
    //  If the NIDP value is 0 then the cell is a source cell and is the top of the flow path
    //  If the NIDP value is 1 then the cell is an interior cell and simply take the flow and passes it to the next cell
    //  If the NIDP value is >=2 then the cell is an intersectionm of flow paths

    int storenum = 3;

    QuadtreeManager<FlowGeneral> NIDP(flowaccum.TopLeft(), flowaccum.BottomRight());
    NIDP.prePath = "Temp/AccumulationTree/NIDPTree";
    NIDP.spacing = flowaccum.spacing;
    NIDP.splitlevel = flowaccum.splitlevel;
    NIDP.SetTreeType(flowaccum.type);

    ////Create quadtree for storing edge cases
    QuadtreeManager<FlowDirection> gaps(flowdirection.topLeft, flowdirection.bottomRight);
    gaps.prePath = "Temp/AccumulationTree/NIDPGapTree";
    gaps.spacing = flowdirection.spacing;
    gaps.splitlevel = flowdirection.splitlevel - 2;
    gaps.SetTreeType(flowdirection.type);

    //for loops for iterating through split level trees
    for (int v = 0; v < numquads; v++) //move vertically through sub trees
    {
        //Calculate the offset to add when seaching within the loop, the +1 is to push it into the next subquad if subquad is the bottom left no offset is required
        //int offsety = v == 0 ? 0 : std::floor(boundspersubquady * v)+1;

        int offsety;
        if (v == 0)
        {
            offsety = 0;
            boundspersubquady = 1000;
        }
        if (v == 1)
        {
            offsety = 1000;
            boundspersubquady = 999;
        }
        if (v == 2)
        {
            offsety = 1999;
            boundspersubquady = 1000;
        }
        if (v == 3)
        {
            offsety = 2999;
            boundspersubquady = 999;
        }

        for (int w = 0; w < numquads; w++) //move horizontally through sub trees
        {
            //Same as the y offset
            //int offsetx = w == 0 ? 0 : std::floor(boundspersubquadx * w)+1;

            int offsetx;
            if (w == 0)
            {
                offsetx = 0;
                boundspersubquadx = 1000;
            }
            if (w == 1)
            {
                offsetx = 1000;
                boundspersubquadx = 999;
            }
            if (w == 2)
            {
                offsetx = 1999;
                boundspersubquadx = 1000;
            }
            if (w == 3)
            {
                offsetx = 2999;
                boundspersubquadx = 999;
            }

            std::cout << "\nProcessing Quad " << v * numquads + (w + 1) << " of " << totalquads << "\n";

            for (int y = 0; y < boundspersubquady; y++) //move through each coord in the y direction of the subtree
            {
                for (int x = 0; x < boundspersubquadx; x++)//move through each coord in the x direction of the subtree
                {
                    if (y < storenum || (boundspersubquady - y) <= storenum || x < storenum || (boundspersubquadx - x) <= storenum)
                    {
                        Node<FlowDirection>* node = flowdirection.Search(FlowDirection(x + offsetx + left, y + offsety + bottom));

                        if (node != nullptr)
                        {
                            FlowDirection coord = node->pos;
                            gaps.Insert(new Node<FlowDirection>(coord));
                        }
                    }
                    else
                    {
                        Node<FlowDirection>* node = flowdirection.Search(FlowDirection(x + offsetx + left, y + offsety + bottom));

                        if (node != nullptr)
                        {
                            int NIDPval = 0;

                            node = flowdirection.Search(FlowDirection(x + offsetx + left, y - 1 + offsety + bottom - 1));
                            if (node != nullptr)
                            {
                                if (node->pos.direction == Direction::NE)
                                    NIDPval++;
                            }

                            node = flowdirection.Search(FlowDirection(x + offsetx + left, y + offsety + bottom - 1));
                            if (node != nullptr)
                            {
                                if (node->pos.direction == Direction::N)
                                    NIDPval++;
                            }

                            node = flowdirection.Search(FlowDirection(x + offsetx + left + 1, y + offsety + bottom - 1));
                            if (node != nullptr)
                            {
                                if (node->pos.direction == Direction::NW)
                                    NIDPval++;
                            }

                            node = flowdirection.Search(FlowDirection(x + offsetx + left - 1, y + offsety + bottom));
                            if (node != nullptr)
                            {
                                if (node->pos.direction == Direction::E)
                                    NIDPval++;
                            }

                            node = flowdirection.Search(FlowDirection(x + offsetx + left + 1, y + offsety + bottom));
                            if (node != nullptr)
                            {
                                if (node->pos.direction == Direction::W)
                                    NIDPval++;
                            }

                            node = flowdirection.Search(FlowDirection(x + offsetx + left - 1, y + offsety + bottom + 1));
                            if (node != nullptr)
                            {
                                if (node->pos.direction == Direction::SE)
                                    NIDPval++;
                            }

                            node = flowdirection.Search(FlowDirection(x + offsetx + left, y + offsety + bottom + 1));
                            if (node != nullptr)
                            {
                                if (node->pos.direction == Direction::S)
                                    NIDPval++;
                            }

                            node = flowdirection.Search(FlowDirection(x + offsetx + left + 1, y + offsety + bottom + 1));
                            if (node != nullptr)
                            {
                                if (node->pos.direction == Direction::SW)
                                    NIDPval++;
                            }

                            NIDP.Insert(new Node<FlowGeneral>(FlowGeneral(x + offsetx + left, y + offsety + bottom, NIDPval)));
                        }
                    }
                }
            }
        }
    }

    flowdirection.Cleanup();
    gaps.Cleanup();
    NIDP.Cleanup();

    std::cout << "\nComplete\nFilling Gaps...\n";

    for (int v = 0; v < numquads; v++) //move vertically through sub trees
    {
        //Calculate the offset to add when seaching within the loop, the +1 is to push it into the next subquad if subquad is the bottom left no offset is required
        //int offsety = v == 0 ? 0 : std::floor(boundspersubquady * v) + 1;

        int offsety;
        if (v == 0)
        {
            offsety = 0;
            boundspersubquady = 1000;
        }
        if (v == 1)
        {
            offsety = 1000;
            boundspersubquady = 999;
        }
        if (v == 2)
        {
            offsety = 1999;
            boundspersubquady = 1000;
        }
        if (v == 3)
        {
            offsety = 2999;
            boundspersubquady = 999;
        }

        for (int w = 0; w < numquads; w++) //move horizontally through sub trees
        {
            //Same as the y offset
            //int offsetx = w == 0 ? 0 : std::floor(boundspersubquadx * w) + 1;

            int offsetx;
            if (w == 0)
            {
                offsetx = 0;
                boundspersubquadx = 1000;
            }
            if (w == 1)
            {
                offsetx = 1000;
                boundspersubquadx = 999;
            }
            if (w == 2)
            {
                offsetx = 1999;
                boundspersubquadx = 1000;
            }
            if (w == 3)
            {
                offsetx = 2999;
                boundspersubquadx = 999;
            }

            std::cout << "\rProcessing Quad " << v * numquads + (w + 1) << " of " << totalquads;

            //Top
            for (int x = 0; x <= boundspersubquadx; x++)
                for (int y = boundspersubquady - storenum; y <= boundspersubquady; y++)
                {
                    Node<FlowDirection>* node = gaps.Search(FlowDirection(x + offsetx + left, y + offsety + bottom));

                    if (node != nullptr)
                    {
                        int NIDPval = 0;

                        node = gaps.Search(FlowDirection(x + offsetx + left-1, y + offsety + bottom - 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::NE)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left, y + offsety + bottom - 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::N)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left+1, y + offsety + bottom - 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::NW)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left-1, y + offsety + bottom));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::E)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left+1, y + offsety + bottom));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::W)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left-1, y + offsety + bottom + 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::SE)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left, y + offsety + bottom + 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::S)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left+1, y + offsety + bottom + 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::SW)
                                NIDPval++;
                        }

                        NIDP.Insert(new Node<FlowGeneral>(FlowGeneral(x + offsetx + left, y + offsety + bottom, NIDPval)));
                    }
                }
            //Bottom
            for (int x = 0; x <= boundspersubquadx; x++)
                for (int y = 0; y <= storenum; y++)
                {
                    Node<FlowDirection>* node = gaps.Search(FlowDirection(x + offsetx + left, y + offsety + bottom));

                    if (node != nullptr)
                    {
                        int NIDPval = 0;

                        node = gaps.Search(FlowDirection(x + offsetx + left - 1, y + offsety + bottom - 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::NE)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left, y + offsety + bottom - 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::N)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left + 1, y + offsety + bottom - 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::NW)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left - 1, y + offsety + bottom));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::E)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left + 1, y + offsety + bottom));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::W)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left - 1, y + offsety + bottom + 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::SE)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left, y + offsety + bottom + 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::S)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left + 1, y + offsety + bottom + 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::SW)
                                NIDPval++;
                        }

                        NIDP.Insert(new Node<FlowGeneral>(FlowGeneral(x + offsetx + left, y + offsety + bottom, NIDPval)));
                    }
                }
            //Right

            for (int x = boundspersubquadx - storenum; x <= boundspersubquadx; x++)
                for (int y = storenum; y <= boundspersubquady - storenum; y++)
                {
                    Node<FlowDirection>* node = gaps.Search(FlowDirection(x + offsetx + left, y + offsety + bottom));

                    if (node != nullptr)
                    {
                        int NIDPval = 0;

                        node = gaps.Search(FlowDirection(x + offsetx + left - 1, y + offsety + bottom - 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::NE)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left, y + offsety + bottom - 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::N)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left + 1, y + offsety + bottom - 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::NW)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left - 1, y + offsety + bottom));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::E)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left + 1, y + offsety + bottom));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::W)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left - 1, y + offsety + bottom + 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::SE)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left, y + offsety + bottom + 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::S)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left + 1, y + offsety + bottom + 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::SW)
                                NIDPval++;
                        }

                        NIDP.Insert(new Node<FlowGeneral>(FlowGeneral(x + offsetx + left, y + offsety + bottom, NIDPval)));
                    }
                }
            //Left
            for (int x = 0; x <= storenum; x++)
                for (int y = 0; y <= boundspersubquady; y++)
                {
                    Node<FlowDirection>* node = gaps.Search(FlowDirection(x + offsetx + left, y + offsety + bottom));

                    if (node != nullptr)
                    {
                        int NIDPval = 0;

                        node = gaps.Search(FlowDirection(x + offsetx + left - 1, y + offsety + bottom - 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::NE)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left, y + offsety + bottom - 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::N)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left + 1, y + offsety + bottom - 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::NW)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left - 1, y + offsety + bottom));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::E)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left + 1, y + offsety + bottom));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::W)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left - 1, y + offsety + bottom + 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::SE)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left, y + offsety + bottom + 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::S)
                                NIDPval++;
                        }

                        node = gaps.Search(FlowDirection(x + offsetx + left + 1, y + offsety + bottom + 1));
                        if (node != nullptr)
                        {
                            if (node->pos.direction == Direction::SW)
                                NIDPval++;
                        }

                        NIDP.Insert(new Node<FlowGeneral>(FlowGeneral(x + offsetx + left, y + offsety + bottom, NIDPval)));
                    }
                }

        }
    }

    flowdirection.Cleanup();
    gaps.Cleanup();
    NIDP.Cleanup();

    //FLOW ACCUM
    std::cout << "Calculating Flow Accumulations\n";

    //for loops for iterating through split level trees
    for (int v = 0; v < numquads; v++) //move vertically through sub trees
    {
        //Calculate the offset to add when seaching within the loop, the +1 is to push it into the next subquad if subquad is the bottom left no offset is required
        //int offsety = v == 0 ? 0 : std::floor(boundspersubquady * v)+1;

        int offsety;
        if (v == 0)
        {
            offsety = 0;
            boundspersubquady = 1000;
        }
        if (v == 1)
        {
            offsety = 1000;
            boundspersubquady = 999;
        }
        if (v == 2)
        {
            offsety = 1999;
            boundspersubquady = 1000;
        }
        if (v == 3)
        {
            offsety = 2999;
            boundspersubquady = 999;
        }

        for (int w = 0; w < numquads; w++) //move horizontally through sub trees
        {
            //Same as the y offset
            //int offsetx = w == 0 ? 0 : std::floor(boundspersubquadx * w)+1;

            int offsetx;
            if (w == 0)
            {
                offsetx = 0;
                boundspersubquadx = 1000;
            }
            if (w == 1)
            {
                offsetx = 1000;
                boundspersubquadx = 999;
            }
            if (w == 2)
            {
                offsetx = 1999;
                boundspersubquadx = 1000;
            }
            if (w == 3)
            {
                offsetx = 2999;
                boundspersubquadx = 999;
            }

            std::cout << "\nProcessing Quad " << v * numquads + (w + 1) << " of " << totalquads << "\n";

            for (int y = 0; y < boundspersubquady; y++) //move through each coord in the y direction of the subtree
            {
                for (int x = 0; x < boundspersubquadx; x++)//move through each coord in the x direction of the subtree
                {
                    Node<FlowGeneral>* nNIDP = NIDP.Search(FlowGeneral(x + offsetx + left, y + offsety + bottom));

                    if (nNIDP != nullptr)
                    {
                        if (nNIDP->pos.iValue == 0)
                        {
                            //Source node found, trace to the downstream intersection node or boundary of the data
                            int Accum = 0;
                            int i = x + offsetx + left;
                            int j = y + offsety + bottom;

                            do
                            {
                                auto nAcc = flowaccum.Search(FlowGeneral(i, j));
                                if (nAcc != nullptr)
                                {
                                    nAcc->pos.iValue += Accum;
                                    Accum = nAcc->pos.iValue;

                                    Direction d = flowdirection.Search(FlowDirection(i, j))->pos.direction;

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

                                    nNIDP = NIDP.Search(FlowGeneral(i, j));
                                }
                            } while (nNIDP != nullptr);
                        }
                    }
                }
            }
        }
    }

    std::cout << "\nComplete\n";
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

            if (flowacc == nullptr)
                continue;

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

std::vector<FlowPath> CatchmentBuilder::StreamLinkingSplit(QuadtreeManager<FlowGeneral>& flowaccum, QuadtreeManager<FlowDirection>& flowdirection, int acctarget)
{
    std::cout << "Stream Linking\n";

    double boundsx = (flowaccum.BottomRight().x) - (flowaccum.TopLeft().x);
    double boundsy = (flowaccum.TopLeft().y) - (flowaccum.BottomRight().y);
    double bottom = (flowaccum.BottomRight().y);
    double left = (flowaccum.TopLeft().x);

    std::vector<std::vector<Vec2>> flowpaths;

    int numquads = std::pow(2, flowaccum.splitlevel); //quad splits the area in half in the x and y axis
    int totalquads = numquads * numquads; //total number of sub quads at the split level

    double boundspersubquadx = (boundsx / numquads); //the equal division of space in the x axis for the split level
    double boundspersubquady = (boundsy / numquads); //the equal division of space in the y axis for the split level

    //for loops for iterating through split level trees
    for (int v = 0; v < numquads; v++) //move vertically through sub trees
    {
        //Calculate the offset to add when seaching within the loop, the +1 is to push it into the next subquad if subquad is the bottom left no offset is required
        //int offsety = v == 0 ? 0 : std::floor(boundspersubquady * v)+1;

        int offsety;
        if (v == 0)
        {
            offsety = 0;
            boundspersubquady = 1000;
        }
        if (v == 1)
        {
            offsety = 1000;
            boundspersubquady = 999;
        }
        if (v == 2)
        {
            offsety = 1999;
            boundspersubquady = 1000;
        }
        if (v == 3)
        {
            offsety = 2999;
            boundspersubquady = 999;
        }

        for (int w = 0; w < numquads; w++) //move horizontally through sub trees
        {
            //Same as the y offset
            //int offsetx = w == 0 ? 0 : std::floor(boundspersubquadx * w)+1;

            int offsetx;
            if (w == 0)
            {
                offsetx = 0;
                boundspersubquadx = 1000;
            }
            if (w == 1)
            {
                offsetx = 1000;
                boundspersubquadx = 999;
            }
            if (w == 2)
            {
                offsetx = 1999;
                boundspersubquadx = 1000;
            }
            if (w == 3)
            {
                offsetx = 2999;
                boundspersubquadx = 999;
            }

            std::cout << "\nProcessing Quad " << v * numquads + (w + 1) << " of " << totalquads << "\n";

            for (int y = 0; y < boundspersubquady; y++) //move through each coord in the y direction of the subtree
            {
                for (int x = 0; x < boundspersubquadx; x++)//move through each coord in the x direction of the subtree
                {
                    Node<FlowGeneral>* flowacc = flowaccum.Search(FlowGeneral(x + offsetx + left, y + offsety + bottom));
                    if (flowacc == nullptr)
                        continue;

                    if (flowacc->pos.iValue > acctarget)
                        TraceFlowPath(flowdirection, &flowpaths, x + offsetx, y + offsety);
                }
            }
        }
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

                if (flowpaths[a][1] == joinedflowpaths[joinedflowpaths.size() - 1][0])
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

    std::sort(copypaths.begin(), copypaths.end());
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
            if (comp1 < 0.0001 && comp2 < 0.0001)
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

void CatchmentBuilder::CatchmentProperties(std::vector<Catchment>& catchlist, std::vector<DischargePoint> dispoints, QuadtreeManager<FlowGeneral>& catchclass, QuadtreeManager<FlowDirection>& flowdirection)
{
    std::cout << "Processing Catchments\n";

    //Temporary tree for storing the isochrones, plotting purposes only
    FlowGeneral tL3(flowdirection.topLeft.x, flowdirection.topLeft.y);
    FlowGeneral bR3(flowdirection.bottomRight.x, flowdirection.bottomRight.y);
    QuadtreeManager<FlowGeneral> temp(tL3, bR3);
    temp.prePath = "Temp/AccumulationTree/Tree";
    temp.spacing = flowdirection.spacing;
    temp.splitlevel = flowdirection.splitlevel;
    temp.SetTreeType(flowdirection.type);

    for (int a = 0; a < dispoints.size(); a++) //loop through all the discharge points 
    {
        std::cout << "\rProcessing Catchment " << a + 1 << " of " << dispoints.size();

        Catchment c;
        c.dp = dispoints[a];
        c.id = dispoints[a].index;

        double boundsx = (catchclass.BottomRight().x) - (catchclass.TopLeft().x);
        double boundsy = (catchclass.TopLeft().y) - (catchclass.BottomRight().y);
        double bottom = (catchclass.BottomRight().y);
        double left = (catchclass.TopLeft().x);

        //narrow down the bounds to the catchment to avoid unnecessary iterations
        MinMax catchmentMM;

        for (int x = 0; x <= boundsx; x++)
            for (int y = 0; y <= boundsy; y++)
            {
                auto node = catchclass.Search(FlowGeneral(x + left, y + bottom));
                if (node != nullptr && node->pos.iValue == c.id)
                {
                    if (x + left > catchmentMM.maxx)
                        catchmentMM.maxx = x + left;

                    if (y + bottom > catchmentMM.maxy)
                        catchmentMM.maxy = y + bottom;

                    if (x + left < catchmentMM.minx)
                        catchmentMM.minx = x + left;

                    if (y + bottom < catchmentMM.miny)
                        catchmentMM.miny = y + bottom;
                }
            }

        c.bounds = catchmentMM;

        auto topLeft = FlowGeneral(c.bounds.minx - 0.5, c.bounds.maxy + 0.5);
        auto bottomRight = FlowGeneral(c.bounds.maxx + 0.5, c.bounds.miny - 0.5);

        bottom = c.bounds.miny;
        left = c.bounds.minx;
        boundsx = c.bounds.maxx;
        boundsy = c.bounds.maxy;

        QuadtreeManager<FlowGeneral> catchmentTree(topLeft, bottomRight);

        catchmentTree.prePath = "Temp/Catchment/Tree";
        catchmentTree.spacing = catchclass.spacing;
        catchmentTree.splitlevel = 0;
        catchmentTree.SetTreeType(TreeType::Single);

        int count = 0;
        //Copy all nodes with the matching catchment ID to a new tree for faster read write
        for (double x = left; x <= boundsx; x++)
            for (double y = bottom; y <= boundsy; y++)
            {
                auto node = catchclass.Search(FlowGeneral(x, y));
                if (node != nullptr)
                    if (node->pos.iValue == c.id)
                    {
                        //insert at the four corners of the cell
                        catchmentTree.Insert(new Node<FlowGeneral>(FlowGeneral(x, y, c.id)));
                        count++;
                    }
            }

        if (count == 0)
            continue;

        //Polygonise

        PolygoniseCatchment(catchmentTree, c);

        //Get all flow paths and store longest in struct with length
        std::vector<FlowPath> flowpaths;

        c.longest = LongestFlowPath(catchmentTree, c, flowdirection, flowpaths);

        if (flowpaths.size() == 0)
            continue;

        c.longestfplength = c.longest.Length();

        /* DO MANNINGS AND LOSS FINDING HERE*/
        //In place of the mannings polygon function set the mannings value #REPLACE THIS LATER#
        c.mannings = 0.0833;
        c.IL = 0;
        c.CL = 2.5;

        //find average slope and flow distance
        Normal tL = Normal(catchclass.TopLeft().x, catchclass.TopLeft().y);
        Normal bR = Normal(catchclass.BottomRight().x, catchclass.BottomRight().y);

        QuadtreeManager<Normal> normal2(tL, bR);
        normal2.prePath = "Temp/NormalTree/Tree";
        normal2.ReadManagerFromFile();

        CalculateCatchmentParams(c, normal2);
        normal2.~QuadtreeManager();

        //Iso chrones

        IsochroneGeneration(catchmentTree, flowpaths, c);

        for (double x = left; x <= boundsx; x++)
            for (double y = bottom; y <= boundsy; y++)
            {
                auto node = catchmentTree.Search(FlowGeneral(x, y));
                if (node != nullptr)
                {
                    //if (node->pos.iValue >= 1)
                    //{
                    //    c.isochroneareas[node->pos.iValue]++;
                    //}
                    temp.Insert(new Node<FlowGeneral>(FlowGeneral(x, y, node->pos.iValue + 1000 * c.id)));
                }

            }
        catchmentTree.~QuadtreeManager();
        catchlist.push_back(c);
    }

    std::cout << "\n";
    FileWriter::WriteFlowGeneralTreeASC("./Exports/Surfaces/Isochrone", temp);
    temp.~QuadtreeManager();
}

void CatchmentBuilder::PolygoniseCatchment(QuadtreeManager<FlowGeneral>& catchclass, Catchment& catchment)
{
    double bottom = catchment.bounds.miny;
    double left = catchment.bounds.minx;
    double boundsx = catchment.bounds.maxx;
    double boundsy = catchment.bounds.maxy;

    std::vector<Vec2> temppoints;

    //Copy all boundary cells to temppoints
    for (double x = left; x <= boundsx; x++)
        for (double y = bottom; y <= boundsy; y++)
        {
            if (catchclass.Search(FlowGeneral(x, y)) != nullptr)
            {
                if (catchclass.Search(FlowGeneral(x - 1, y)) == nullptr || catchclass.Search(FlowGeneral(x + 1, y)) == nullptr ||
                    catchclass.Search(FlowGeneral(x, y - 1)) == nullptr || catchclass.Search(FlowGeneral(x, y + 1)) == nullptr)
                {
                    temppoints.push_back(Vec2(x, y));
                }
            }
        }

    if (temppoints.size() == 0)
        return;

    //find the bottom most point
    int lowestindex = 0;
    float lowest = std::numeric_limits<float>::max();

    for (int i = 0; i < temppoints.size(); i++)
    {
        if (temppoints[i].y <= lowest)
        {
            lowestindex = i;
            lowest = temppoints[i].y;
        }
    }

    catchment.points.push_back(temppoints[lowestindex]);
    temppoints.erase(std::begin(temppoints) + lowestindex);

    double x = catchment.points[0].x;
    double y = catchment.points[0].y;

    while (true)
    {
        if (temppoints.size() == 0)
            break;

        int index1 = 0;
        int index2 = 0;
        int index3 = 0;
        float dist1 = std::numeric_limits<float>::max();
        float dist2 = std::numeric_limits<float>::max();
        float dist3 = std::numeric_limits<float>::max();

        for (int i = 0; i < temppoints.size(); i++)
        {
            float dist = DistBetween(catchment.points[catchment.points.size() - 1], temppoints[i]);
            if (dist == 0)
                continue;

            if (dist <= dist3)
            {
                if (dist <= dist2)
                {
                    if (dist <= dist1)
                    {
                        dist3 = dist2;
                        index3 = index2;
                        dist2 = dist1;
                        index2 = index1;
                        dist1 = dist;
                        index1 = i;
                    }
                    else
                    {
                        dist3 = dist2;
                        index3 = index2;
                        dist2 = dist;
                        index2 = i;
                    }
                }
                else
                {
                    dist3 = dist;
                    index3 = i;
                }
            }
        }

        Vec2 n;

        if (catchment.points.size() == 1)
        {
            n = catchment.points[0];
            n.y--;
        }
        else
        {
            n = catchment.points[catchment.points.size() - 2];
        }

        Vec2 o = catchment.points[catchment.points.size() - 1];

        Vec2 p = temppoints[index1];
        Vec2 q = temppoints[index2];
        Vec2 r = temppoints[index3];

        Vec2 on = Vec2(n.x - o.x, n.y - o.y);
        Vec2 op = Vec2(p.x - o.x, p.y - o.y);
        Vec2 oq = Vec2(q.x - o.x, q.y - o.y);
        Vec2 or = Vec2(r.x - o.x, r.y - o.y);

        float dotnop = on.x * op.x + on.y * op.y;
        float detnop = on.x * op.y - on.y * op.x;

        float anglenop = 360 - (std::atan2(detnop, dotnop) * 180 / PI);

        float dotnoq = on.x * oq.x + on.y * oq.y;
        float detnoq = on.x * oq.y - on.y * oq.x;

        float anglenoq = 360 - (std::atan2(detnoq, dotnoq) * 180 / PI);

        float dotnor = on.x * or .x + on.y * or .y;
        float detnor = on.x * or .y - on.y * or .x;

        float anglenor = 360 - (std::atan2(detnor, dotnor) * 180 / PI);

        if (dist1 == dist2 && dist1 == dist3)
        {
            //filter by angle
            if (anglenop < anglenoq && anglenop < anglenor)
            {
                catchment.points.push_back(temppoints[index1]);
                temppoints.erase(std::begin(temppoints) + index1);
                continue;
            }
            else if (anglenoq < anglenop && anglenoq < anglenor)
            {
                catchment.points.push_back(temppoints[index2]);
                temppoints.erase(std::begin(temppoints) + index2);
                continue;
            }
            else if (anglenor < anglenop && anglenor < anglenoq)
            {
                catchment.points.push_back(temppoints[index3]);
                temppoints.erase(std::begin(temppoints) + index3);
                continue;
            }
        }
        if (dist1 == dist2 && dist2 < dist3)
        {
            //dist 1 and 2 are equal but greater than 3
            //filter by angle
            if (anglenop < anglenoq)
            {
                catchment.points.push_back(temppoints[index1]);
                temppoints.erase(std::begin(temppoints) + index1);
                continue;
            }
            else if (anglenoq < anglenop)
            {
                catchment.points.push_back(temppoints[index2]);
                temppoints.erase(std::begin(temppoints) + index2);
                continue;
            }
        }
        if (dist1 < dist2 && dist1 < 10)
        {
            //dist1 is closest
            catchment.points.push_back(temppoints[index1]);
            temppoints.erase(std::begin(temppoints) + index1);
            continue;
        }
        else
        {
            break;
        }
    }
}

float CatchmentBuilder::DistBetween(Vec2 v1, Vec2 v2)
{
    return std::sqrt(std::pow(v2.y - v1.y, 2) + std::pow(v2.x - v1.x, 2));
}

FlowPath CatchmentBuilder::LongestFlowPath(QuadtreeManager<FlowGeneral>& catchclass, Catchment& catchment, QuadtreeManager<FlowDirection>& flowdirection, std::vector<FlowPath>& flowpaths)
{
    //for the catchment generate all flow paths and store in a vector and return the longest
    double bottom = catchment.bounds.miny;
    double left = catchment.bounds.minx;
    double boundsx = catchment.bounds.maxx;
    double boundsy = catchment.bounds.maxy;

    FlowPath longest;

    for (double x = left; x <= boundsx; x++)
        for (double y = bottom; y <= boundsy; y++)
        {
            auto node = catchclass.Search(FlowGeneral(x, y));
            if (node != nullptr)
            {
                FlowPath temp = GetFlowPathFrom(flowdirection, catchment.dp, Vec2(x, y));
                temp.id = catchment.id;
                flowpaths.push_back(temp);
                if (longest.path.size() == 0)
                {
                    longest = temp;
                }
                else if (longest.Length() < temp.Length())
                {
                    longest = temp;
                }
            }
        }
    return longest;
}

FlowPath CatchmentBuilder::GetFlowPathFrom(QuadtreeManager<FlowDirection>& flowdirection, DischargePoint dischargepoint, Vec2 point)
{
    int exitcond = 0; //if 0 not reached exit, if 1 discharge point found, if 2 nullptr found, if 3 circular flowpath found
    int flowpathid = 0;
    FlowPath flowpath;

    double i = point.x;
    double j = point.y;

    auto d = flowdirection.Search(FlowDirection(i, j));

    while (exitcond == 0)
    {
        flowpath.path.push_back(Vec2(d->pos.x, d->pos.y));

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

        if (d == nullptr)
        {
            exitcond = 2;
            break;
        }


        if (dischargepoint.location == Vec2(i, j))
        {
            exitcond = 1;
            flowpathid = dischargepoint.index;
            break;
        }


        for (size_t a = 0; a < flowpath.path.size(); a++)
        {
            double comp1 = std::abs(flowpath.path[a].x - i);
            double comp2 = std::abs(flowpath.path[a].y - j);
            if (comp1 < 0.0001 && comp2 < 0.0001)
            {
                exitcond = 3;
            }
        }
    }

    return flowpath;
}

void CatchmentBuilder::CalculateCatchmentParams(Catchment& c, QuadtreeManager<Normal>& normal)
{
    //Check that the longest fp is not 0
    if (c.longest.path.size() == 0)
        return;

    //store all height along the FP
    std::vector<Vec3> points;

    for each (Vec2 v in c.longest.path)
    {
        auto node = normal.Search(Normal(v.x, v.y));
        if (node != nullptr)
        {
            points.push_back(node->pos);
        }
    }

    //calculate the slope for all segments
    std::vector<float> slopes;

    for (int i = 1; i < points.size(); i++)
    {
        float slope = (points[i - 1].z - points[i].z) / DistBetween(Vec2(points[i - 1].x, points[i - 1].y), Vec2(points[i].x, points[i].y));
        slopes.push_back(slope);
    }

    //Sum the slopes to find an average
    float sumslope = 0;

    for (int i = 1; i < slopes.size(); i++)
    {
        sumslope += slopes[i];
    }

    //calculate the average slope
    float avgslope = sumslope / c.longestfplength;

    //Flow Distance
    //Using Friends Eq calculate the distance up the slope water will travel in the timestep
    //rainfall data is in 30min inc, this will need to be fixed when rainfall function is finished
    int timestep = 30; //get the dynamically later

    double temp2 = std::pow(avgslope, 0.2);
    double temp1 = (timestep * (temp2)) / (c.mannings * 107);
    float dist = std::pow(temp1, 3);

    c.avgslope = avgslope;
    c.flowdistance = dist;
    c.highestpt = points[0].z;
    c.lowestpt = points[points.size() - 1].z;

}

void CatchmentBuilder::IsochroneGeneration(QuadtreeManager<FlowGeneral>& catchclass, std::vector<FlowPath> flowpaths, Catchment& catchm)
{
    double bottom = catchm.bounds.miny;
    double left = catchm.bounds.minx;
    double boundsx = catchm.bounds.maxx;
    double boundsy = catchm.bounds.maxy;
    
    //Set nodes to 0
    for (double x = left; x <= boundsx; x++)
        for (double y = bottom; y <= boundsy; y++)
        {
            auto node = catchclass.Search(FlowGeneral(x, y));
            if (node != nullptr)
                node->pos.iValue = 0;
        }

    ClassifyIsochrones(catchclass, catchm, flowpaths);

    int catcharea = 0;
    int maxiso = 0;

    for (double x = left; x <= boundsx; x++)
        for (double y = bottom; y <= boundsy; y++)
        {
            auto node = catchclass.Search(FlowGeneral(x, y));
            if (node != nullptr)
            {
                catcharea++;
                if (node->pos.iValue > maxiso)
                    maxiso = node->pos.iValue;
            }

        }

    catchm.area = catcharea;

    for (int i = 1; i <= maxiso; i++)
    {
        int area = 0;
        for (double x = left; x <= boundsx; x++)
            for (double y = bottom; y <= boundsy; y++)
            {
                auto node = catchclass.Search(FlowGeneral(x, y));
                if (node != nullptr)
                {
                    if (node->pos.iValue == i)
                        area++;
                }

            }
        catchm.isochroneareas.push_back(area);
    }
}

void CatchmentBuilder::ClassifyIsochrones(QuadtreeManager<FlowGeneral>& catchment, Catchment c, std::vector<FlowPath> flowpaths)
{
    for each (FlowPath fp in flowpaths)
    {
        //Classify Isos here
        std::vector<Vec2> inisochrone;
        int isoid = 1;

        float fplength = fp.Length();

        for (int dist = 0; dist < fplength; dist += c.flowdistance)
        {
            inisochrone = fp.GetPointsBetween(dist, dist + c.flowdistance);

            for each (Vec2 fg in inisochrone)
            {
                auto c = catchment.Search(FlowGeneral(fg.x, fg.y));
                if (c != nullptr)
                    c->pos.iValue = isoid;
            }

            inisochrone.clear();
            isoid++;
        }
    }
}
