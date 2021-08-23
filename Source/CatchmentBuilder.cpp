#include "../Headers/CatchmentBuilder.h"
#include "../Headers/FileWriter.h"

#include <iostream>

void CatchmentBuilder::CreateCatchments(QuadtreeManager<Coordinates>& quad)
{
    std::cout << "Smoothing Surface\n";

    QuadtreeManager<Coordinates> smooth(quad.topLeft,quad.bottomRight);

    smooth.prePath = "Temp/SmoothTree/Tree";
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

    //Normal tL(quad.topLeft.x + quad.spacing / 2, quad.topLeft.y - quad.spacing / 2);
    //Normal bR(quad.bottomRight.x - quad.spacing / 2, quad.bottomRight.y + quad.spacing / 2);
    //
    //QuadtreeManager<Normal> normal(tL,bR);

    //smooth.prePath = "Temp/NormalTree/Tree";
    //smooth.spacing = quad.spacing;
    //smooth.splitlevel = quad.splitlevel;
    //smooth.SetTreeType(quad.type);

    //CalculateNormals(quad, normal);
}

void CatchmentBuilder::CalculateNormals(QuadtreeManager<Coordinates>& quad, QuadtreeManager<Normal>& normal)
{
    //std::cout << "Generating Normals\n";
    //float boundsx = (smoothQuad->BottomRight().x) - (smoothQuad->TopLeft().x);
    //float boundsy = (smoothQuad->TopLeft().y) - (smoothQuad->BottomRight().y);
    //float bottom = (smoothQuad->BottomRight().y);
    //float left = (smoothQuad->TopLeft().x);

    //for (int x = 0; x < boundsx; x++)
    //    for (int y = 0; y < boundsy; y++)
    //    {
    //        //Get points in a quad going clockwise starting from the BL

    //        Vec3 p1, p2, p3, p4, vec1, vec2, vec3, vec4, translation, normal1, normal2;

    //        Coordinates c = smoothQuad->search(Coordinates(x + left, y + bottom))->pos;

    //        p1 = Vec3(c.x, c.y, c.z);

    //        c = smoothQuad->search(Coordinates(x + left, y + bottom + 1))->pos;

    //        p2 = Vec3(c.x, c.y, c.z);

    //        c = smoothQuad->search(Coordinates(x + left + 1, y + bottom + 1))->pos;

    //        p3 = Vec3(c.x, c.y, c.z);

    //        c = smoothQuad->search(Coordinates(x + left + 1, y + bottom))->pos;

    //        p4 = Vec3(c.x, c.y, c.z);
    //        
    //        float avz = (p1.z + p2.z + p3.z + p4.z) / 4;
    //        translation = Vec3(p1.x + 0.5f, p1.y + .5f, avz);

    //        //translate each vector so each side of the quad is represented
    //        vec1 = p2 - p1;
    //        vec2 = p3 - p2;
    //        vec3 = p4 - p3;
    //        vec4 = p1 - p4;

    //        //Cross product to calculate normal
    //        normal1 = vec1 % vec2;

    //        normal2 = vec3 % vec4;

    //        //average normals to get normal for quad
    //        Vec3 normalq = Vec3((normal1.x + normal2.x) / 2, (normal1.y + normal2.y) / 2, (normal1.z + normal2.z) / 2);
    //        normalq.Normalize();

    //        normalq += translation;
    //        
    //        normalquad->insert(new Node<Normal>(Normal(translation, normalq)));
    //    }

    //std::cout << "Writing Normals to File.\n";
    ////FileWriter::WriteVecNormals3dWKT("./Exports/Normals/SmoothNormals3dWKT", normalquad);
    ////FileWriter::WriteVecNormals2dWKT("./Exports/Normals/SmoothNormals2dWKT", normalquad);
}

void CatchmentBuilder::SmoothPointsSingle(QuadtreeManager<Coordinates>& quad, QuadtreeManager<Coordinates>& smooth)
{
    double boundsx = (quad.BottomRight().x) - (quad.TopLeft().x);
    double boundsy = (quad.TopLeft().y) - (quad.BottomRight().y);
    double bottom = (quad.BottomRight().y);
    double left = (quad.TopLeft().x);

    int numquads;

    if (quad.type == TreeType::Split)
    {
        numquads = quad.splitlevel * 2; //quad splits the area in half in the x and y axis
    }
    else
    {
        numquads = 1; //if single tree quads at the split level is 1
    }
    double boundsperquadx = boundsx / numquads;
    double boundsperquady = boundsy / numquads;

    std::vector<Coordinates> list;
    int totalquads = numquads * numquads;

    //for loop for iterating through split level trees
    for (int v = 0; v < numquads; v++)
        for (int w = 0; w < numquads; w++)
            for (int y = 0; y < boundsperquady; y++)
                for (int x = 0; x < boundsperquadx; x++)
                {

                    Node<Coordinates>* node = quad.Search(Coordinates(x + v * boundsperquadx + left, y + w * boundsperquady + bottom));

                    if (node != nullptr)
                    {
                        Coordinates coord = node->pos;

                        std::vector<Coordinates> vecCoords;

                        for (int j = y - 1; j <= y + 1; j++)
                            for (int i = x - 1; i <= x + 1; i++)
                            {
                                Node<Coordinates>* n = quad.Search(Coordinates(i + v * boundsperquadx + left, j + w * boundsperquady + bottom));

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

    double boundsperquadx = std::floor((boundsx / numquads) * 2) / 2;
    double boundsperquady = std::floor((boundsy / numquads) * 2) / 2;

    if (boundsperquadx - std::floor(boundsperquadx)-0.5 > 0.001)
    {
        boundsperquadx += 0.5;
    }
    if (boundsperquady - std::floor(boundsperquady) - 0.5 > 0.001)
    {
        boundsperquady += 0.5;
    }

    std::cout << boundsperquadx << "\n";
    std::cout << boundsperquady << "\n";

    int totalquads = numquads * numquads;

    ////Create quadtree for storing edge cases
    //QuadtreeManager<Coordinates> smooth(quad.topLeft, quad.bottomRight);

    //smooth.prePath = "Temp/SmoothTree/Tree";
    //smooth.spacing = quad.spacing;
    //smooth.splitlevel = quad.splitlevel;
    //smooth.SetTreeType(quad.type);

    //for loops for iterating through split level trees
    for (int v = 0; v < numquads; v++) //move vertically through sub trees
        for (int w = 0; w < numquads; w++) //move horizontally through sub trees
        {
            std::cout << "\rProcessing Quad " << v * numquads + (w + 1) << " of " << totalquads << " Complete";

            for (int y = 0; y < boundsperquady; y++) //move through each coord in the y direction of the subtree
                for (int x = 0; x < boundsperquadx; x++)//move through each coord in the x direction of the subtree
                {
                    Node<Coordinates>* node = quad.Search(Coordinates(x + w * boundsperquadx + left, y + v * boundsperquady + bottom));

                    if (node != nullptr)
                    {
                        Coordinates coord = node->pos;

                        smooth.Insert(new Node<Coordinates>(coord));
                    }
                    else
                    {
                        std::cout << "Nullptr found at: " << x + w * boundsperquadx + left << "," << y + v * boundsperquady + bottom << "\n";
                    }
                    //if (y <= storenum || (boundsperquady - y) <= storenum || x <= storenum || (boundsperquadx - x) <= storenum) //filter out the 
                    //{

                    //}
                    //else
                    //{
                    //    Node<Coordinates>* node = quad.Search(Coordinates(x + w * boundsperquadx + left, y + v * boundsperquady + bottom));

                    //    if (node != nullptr)
                    //    {
                    //        Coordinates coord = node->pos;

                    //        std::vector<Coordinates> vecCoords;

                    //        for (int j = -storenum; j <= storenum; j++)
                    //            for (int i = -storenum; i <= storenum; i++)
                    //            {
                    //                Node<Coordinates>* n = quad.Search(Coordinates((i + x) + (w * boundsperquadx) + left, (j + y) + (v * boundsperquady) + bottom));

                    //                if (n != nullptr)
                    //                {
                    //                    Coordinates coord = n->pos;
                    //                    vecCoords.push_back(coord);
                    //                }
                    //            }

                    //        float zavg = 0.0f;

                    //        for (auto const c : vecCoords)
                    //        {
                    //            zavg += c.z;
                    //        }

                    //        zavg /= vecCoords.size();

                    //        coord.z = zavg;

                    //        smooth.Insert(new Node<Coordinates>(coord));
                    //    }
                    //}
                    //}
                    //}
                }

        }
    
    std::cout << "\n";
    std::cout << "Complete\n";
    std::cout << "Filling Gaps...\n";

    for (int v = 0; v < numquads; v++) //move vertically through sub trees
    {
        for (int w = 0; w < numquads; w++) //move horizontally through sub trees
        {
            //EDGE OF SUB-QUAD PROCESSING
            //process row here
            if (v == 0) //first row of sub-quads: do not process the row above
            {
                //for loops for iterating through split level trees

                for (int w = 0; w < numquads; w++)
                {
                    //Bottom
                    for (int x = 0; x < boundsperquadx; x++)
                        for (int y = 0; y < storenum; y++)
                        {

                        }
                    //Right
                    if (w != (numquads - 1))
                    {
                        for (int x = boundsperquadx - storenum - 1; x < boundsperquadx; x++)
                            for (int y = storenum; y < boundsperquady - storenum - 1; y++)
                            {

                            }
                    }

                    //Left
                    if (w != 0)
                    {
                        for (int x = 0; x < storenum; x++)
                            for (int y = storenum; y < boundsperquady - storenum - 1; y++)
                            {

                            }

                    }
                }

            }
            else if (v == numquads - 1) //bottom row of sub-quads: do not process the row below
            {
                //for loops for iterating through split level trees

                for (int w = 0; w < numquads; w++)
                {
                    //Top
                    for (int x = 0; x < boundsperquadx; x++)
                        for (int y = boundsperquady - storenum - 1; y < boundsperquady; y++)
                        {

                        }
                    //Right
                    if (w != (numquads - 1))
                    {
                        for (int x = boundsperquadx - storenum - 1; x < boundsperquadx; x++)
                            for (int y = storenum; y < boundsperquady - storenum - 1; y++)
                            {

                            }
                    }

                    //Left
                    if (w != 0)
                    {
                        for (int x = 0; x < storenum; x++)
                            for (int y = storenum; y < boundsperquady - storenum - 1; y++)
                            {

                            }
                    }
                }
            }
            else if (v > 1 && v < numquads) //inbetween rows of sub-quads: process both above and below
            {
                //for loops for iterating through split level trees

                for (int w = 0; w < numquads; w++)
                {
                    //Top
                    for (int x = 0; x < boundsperquadx; x++)
                        for (int y = boundsperquady - storenum - 1; y < boundsperquady; y++)
                        {

                        }
                    //Bottom
                    for (int x = 0; x < boundsperquadx; x++)
                        for (int y = 0; y < storenum; y++)
                        {

                        }
                    //Right
                    if (w != (numquads - 1))
                    {
                        for (int x = boundsperquadx - storenum - 1; x < boundsperquadx; x++)
                            for (int y = storenum; y < boundsperquady - storenum - 1; y++)
                            {

                            }
                    }
                    //Left
                    if (w != 0)
                    {
                        for (int x = 0; x < storenum; x++)
                            for (int y = 0; y < boundsperquady; y++)
                            {

                            }
                    }
                }

            }
        }
    }
    std::cout << "Complete\n";
}
