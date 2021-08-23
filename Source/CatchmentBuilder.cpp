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

    int numquads;
    int blurrad = 5; //odd numbers only
    int storenum = (blurrad - 1) / 2;

    numquads = quad.splitlevel * 2; //quad splits the area in half in the x and y axis
    double boundsperquadx = boundsx / numquads;
    double boundsperquady = boundsy / numquads;

    int totalquads = numquads * numquads;

    //for each column store vector of the quads containing a vector of the coords for each side of the quad
    std::vector< std::vector<std::vector<Node<Coordinates>*>>> lista; //list for coords along the perimetre to cache - top
    std::vector< std::vector<std::vector<Node<Coordinates>*>>> listb; //list for coords along the perimetre to cache - centre
    std::vector< std::vector<std::vector<Node<Coordinates>*>>> listc; //list for coords along the perimetre to cache - bottom

    //for loops for iterating through split level trees
    for (int v = 0; v < numquads; v++) //move vertically
    {
        lista = std::move(listb);
        listb = std::move(listc);
        listc.clear();

        for (int w = 0; w < numquads; w++) //move horizontally
        {
            std::vector<Node<Coordinates>*> listTop;
            std::vector<Node<Coordinates>*> listBottom;
            std::vector<Node<Coordinates>*> listLeft;
            std::vector<Node<Coordinates>*> listRight;

            std::vector<std::vector<Node<Coordinates>*>> list1;

            list1.push_back(listTop);
            list1.push_back(listBottom);
            list1.push_back(listLeft);
            list1.push_back(listRight);

            listc.push_back(list1);

            std::cout << "\rQuad " << v * numquads + (w+1) << " of " << totalquads << " Complete";

            for (int y = 0; y < boundsperquady; y++)
                for (int x = 0; x < boundsperquadx; x++)
                {
                    if (y < storenum || (boundsperquady - y) <= storenum || x < storenum || (boundsperquadx - x) <= storenum)
                    {
                        Node<Coordinates>* node = quad.Search(Coordinates(x + w * boundsperquadx + left, y + v * boundsperquady + bottom));
                        
                        if (y < storenum) //Top side of quad
                        {
                            listc[w][0].push_back(node);
                        }
                        else if((boundsperquady - y) <= storenum) //Bottom Of Quad
                        {
                            listc[w][1].push_back(node);
                        }
                        else if (x < storenum) // Left of quad
                        {
                            listc[w][2].push_back(node);
                        }
                        else //right of quad
                        {
                            listc[w][3].push_back(node);
                        }
                    }
                    else
                    {
                        Node<Coordinates>* node = quad.Search(Coordinates(x + w * boundsperquadx + left, y + v * boundsperquady + bottom));

                        if (node != nullptr)
                        {
                            Coordinates coord = node->pos;

                            std::vector<Coordinates> vecCoords;

                            for (int j = y - storenum; j <= y + storenum; j++)
                                for (int i = x - storenum; i <= x + storenum; i++)
                                {
                                    Node<Coordinates>* n = quad.Search(Coordinates(i + w * boundsperquadx + left, j + v * boundsperquady + bottom));

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
        }

        ////EDGE OF SUB-QUAD PROCESSING
        ////process row here
        //if (v == 0) //first row of sub-quads: do not process the row above
        //{
        //    //for loops for iterating through split level trees

        //        for (int w = 0; w < numquads; w++)
        //        {
        //            //Bottom
        //            for (int x = 0; x < boundsperquadx; x++)
        //                for (int y = 0; y < storenum; y++)
        //                {
        //                    Node<Coordinates>* node = quad.Search(Coordinates(x + v * boundsperquadx + left, y + w * boundsperquady + bottom)); //Get Node at centre

        //                    if (node != nullptr) //if not nullptr continue
        //                    {
        //                        Coordinates coord = node->pos; //Get coord from Node

        //                        std::vector<Coordinates> vecCoords; //create a list to use as a container for all points to be blurred

        //                        for (int j = y - storenum; j <= y + storenum; j++)
        //                            for (int i = x - storenum; i <= x + storenum; i++) //Iterate over the blur radius around the point
        //                            {
        //                                if (j > y) //if point is in another quad use the cached list otherwise read it from the loaded quad
        //                                {
        //                                    Node<Coordinates>* n = quad.Search(Coordinates(i + v * boundsperquadx + left, j + w * boundsperquady + bottom));

        //                                    if (n != nullptr)
        //                                    {
        //                                        Coordinates coord = n->pos;
        //                                        vecCoords.push_back(coord);
        //                                    }

        //                                }
        //                                else
        //                                {
        //                                    Coordinates coord(i + v * boundsperquadx + left, j + w * boundsperquady + bottom);
        //                                    for (int a = 0; a < lista[w][0].size(); a++)
        //                                    {
        //                                        if (lista[w][0][a]->pos == coord)
        //                                        {
        //                                            coord = lista[w][0][a]->pos;
        //                                            vecCoords.push_back(coord);
        //                                        }
        //                                    }

        //                                }
        //                            }

        //                        float zavg = 0.0f;

        //                        for (auto const c : vecCoords)
        //                        {
        //                            zavg += c.z;
        //                        }

        //                        zavg /= vecCoords.size();

        //                        coord.z = zavg;

        //                        smooth.Insert(new Node<Coordinates>(coord));
        //                    }
        //                }
        //            //Right
        //            if (w != (numquads - 1))
        //            {
        //                for (int x = boundsperquadx - storenum - 1; x < boundsperquadx; x++)
        //                    for (int y = storenum; y < boundsperquady - storenum - 1; y++)
        //                    {
        //                        Node<Coordinates>* node = quad.Search(Coordinates(x + v * boundsperquadx + left, y + w * boundsperquady + bottom)); //Get Node at centre

        //                        if (node != nullptr) //if not nullptr continue
        //                        {
        //                            Coordinates coord = node->pos; //Get coord from Node

        //                            std::vector<Coordinates> vecCoords; //create a list to use as a container for all points to be blurred

        //                            for (int j = y - storenum; j <= y + storenum; j++)
        //                                for (int i = x - storenum; i <= x + storenum; i++) //Iterate over the blur radius around the point
        //                                {
        //                                    if (i > x) //if point is in another quad use the cached list otherwise read it from the loaded quad
        //                                    {
        //                                        Node<Coordinates>* n = quad.Search(Coordinates(i + v * boundsperquadx + left, j + w * boundsperquady + bottom));

        //                                        if (n != nullptr)
        //                                        {
        //                                            Coordinates coord = n->pos;
        //                                            vecCoords.push_back(coord);
        //                                        }

        //                                    }
        //                                    else
        //                                    {
        //                                        Coordinates coord(i + v * boundsperquadx + left, j + w * boundsperquady + bottom);
        //                                        for (int a = 0; a < listb[w][2].size(); a++)
        //                                        {
        //                                            if (listb[w][2][a]->pos == coord)
        //                                            {
        //                                                coord = listb[w][2][a]->pos;
        //                                                vecCoords.push_back(coord);
        //                                            }
        //                                        }

        //                                    }
        //                                }

        //                            float zavg = 0.0f;

        //                            for (auto const c : vecCoords)
        //                            {
        //                                zavg += c.z;
        //                            }

        //                            zavg /= vecCoords.size();

        //                            coord.z = zavg;

        //                            smooth.Insert(new Node<Coordinates>(coord));
        //                        }
        //                    }
        //            }
        //            
        //            //Left
        //            if (w != 0)
        //            {
        //                for (int x = 0; x < storenum; x++)
        //                    for (int y = storenum; y < boundsperquady - storenum - 1; y++)
        //                    {
        //                        Node<Coordinates>* node = quad.Search(Coordinates(x + v * boundsperquadx + left, y + w * boundsperquady + bottom)); //Get Node at centre

        //                        if (node != nullptr) //if not nullptr continue
        //                        {
        //                            Coordinates coord = node->pos; //Get coord from Node

        //                            std::vector<Coordinates> vecCoords; //create a list to use as a container for all points to be blurred

        //                            for (int j = y - storenum; j <= y + storenum; j++)
        //                                for (int i = x - storenum; i <= x + storenum; i++) //Iterate over the blur radius around the point
        //                                {
        //                                    if (i < x) //if point is in another quad use the cached list otherwise read it from the loaded quad
        //                                    {
        //                                        Node<Coordinates>* n = quad.Search(Coordinates(i + v * boundsperquadx + left, j + w * boundsperquady + bottom));

        //                                        if (n != nullptr)
        //                                        {
        //                                            Coordinates coord = n->pos;
        //                                            vecCoords.push_back(coord);
        //                                        }

        //                                    }
        //                                    else
        //                                    {
        //                                        Coordinates coord(i + v * boundsperquadx + left, j + w * boundsperquady + bottom);
        //                                        for (int a = 0; a < listb[w][3].size(); a++)
        //                                        {
        //                                            if (listb[w][3][a]->pos == coord)
        //                                            {
        //                                                coord = listb[w][3][a]->pos;
        //                                                vecCoords.push_back(coord);
        //                                            }
        //                                        }

        //                                    }
        //                                }

        //                            float zavg = 0.0f;

        //                            for (auto const c : vecCoords)
        //                            {
        //                                zavg += c.z;
        //                            }

        //                            zavg /= vecCoords.size();

        //                            coord.z = zavg;

        //                            smooth.Insert(new Node<Coordinates>(coord));
        //                        }
        //                    }
        //                    
        //            }
        //        }
        //    
        //}
        //else if (v == numquads-1) //bottom row of sub-quads: do not process the row below
        //{
        //    //for loops for iterating through split level trees

        //    for (int w = 0; w < numquads; w++)
        //    {
        //        //Top
        //        for (int x = 0; x < boundsperquadx; x++)
        //            for (int y = boundsperquady-storenum-1; y < boundsperquady; y++)
        //            {
        //                Node<Coordinates>* node = quad.Search(Coordinates(x + v * boundsperquadx + left, y + w * boundsperquady + bottom)); //Get Node at centre

        //                if (node != nullptr) //if not nullptr continue
        //                {
        //                    Coordinates coord = node->pos; //Get coord from Node

        //                    std::vector<Coordinates> vecCoords; //create a list to use as a container for all points to be blurred

        //                    for (int j = y - storenum; j <= y + storenum; j++)
        //                        for (int i = x - storenum; i <= x + storenum; i++) //Iterate over the blur radius around the point
        //                        {
        //                            if (j < y) //if point is in another quad use the cached list otherwise read it from the loaded quad
        //                            {
        //                                Node<Coordinates>* n = quad.Search(Coordinates(i + v * boundsperquadx + left, j + w * boundsperquady + bottom));

        //                                if (n != nullptr)
        //                                {
        //                                    Coordinates coord = n->pos;
        //                                    vecCoords.push_back(coord);
        //                                }

        //                            }
        //                            else
        //                            {
        //                                Coordinates coord(i + v * boundsperquadx + left, j + w * boundsperquady + bottom);
        //                                for (int a = 0; a < listc[w][1].size(); a++)
        //                                {
        //                                    if (listc[w][1][a]->pos == coord)
        //                                    {
        //                                        coord = listc[w][1][a]->pos;
        //                                        vecCoords.push_back(coord);
        //                                    }
        //                                }

        //                            }
        //                        }

        //                    float zavg = 0.0f;

        //                    for (auto const c : vecCoords)
        //                    {
        //                        zavg += c.z;
        //                    }

        //                    zavg /= vecCoords.size();

        //                    coord.z = zavg;

        //                    smooth.Insert(new Node<Coordinates>(coord));
        //                }
        //            }
        //        //Right
        //        if (w != (numquads - 1))
        //        {
        //            for (int x = boundsperquadx - storenum - 1; x < boundsperquadx; x++)
        //                for (int y = storenum; y < boundsperquady - storenum - 1; y++)
        //                {
        //                    Node<Coordinates>* node = quad.Search(Coordinates(x + v * boundsperquadx + left, y + w * boundsperquady + bottom)); //Get Node at centre

        //                    if (node != nullptr) //if not nullptr continue
        //                    {
        //                        Coordinates coord = node->pos; //Get coord from Node

        //                        std::vector<Coordinates> vecCoords; //create a list to use as a container for all points to be blurred

        //                        for (int j = y - storenum; j <= y + storenum; j++)
        //                            for (int i = x - storenum; i <= x + storenum; i++) //Iterate over the blur radius around the point
        //                            {
        //                                if (i > x) //if point is in another quad use the cached list otherwise read it from the loaded quad
        //                                {
        //                                    Node<Coordinates>* n = quad.Search(Coordinates(i + v * boundsperquadx + left, j + w * boundsperquady + bottom));

        //                                    if (n != nullptr)
        //                                    {
        //                                        Coordinates coord = n->pos;
        //                                        vecCoords.push_back(coord);
        //                                    }

        //                                }
        //                                else
        //                                {
        //                                    Coordinates coord(i + v * boundsperquadx + left, j + w * boundsperquady + bottom);
        //                                    for (int a = 0; a < listb[w][2].size(); a++)
        //                                    {
        //                                        if (listb[w][2][a]->pos == coord)
        //                                        {
        //                                            coord = listb[w][2][a]->pos;
        //                                            vecCoords.push_back(coord);
        //                                        }
        //                                    }

        //                                }
        //                            }

        //                        float zavg = 0.0f;

        //                        for (auto const c : vecCoords)
        //                        {
        //                            zavg += c.z;
        //                        }

        //                        zavg /= vecCoords.size();

        //                        coord.z = zavg;

        //                        smooth.Insert(new Node<Coordinates>(coord));
        //                    }
        //                }
        //        }

        //        //Left
        //        if (w != 0)
        //        {
        //            for (int x = 0; x < storenum; x++)
        //                for (int y = storenum; y < boundsperquady - storenum - 1; y++)
        //                {
        //                    Node<Coordinates>* node = quad.Search(Coordinates(x + v * boundsperquadx + left, y + w * boundsperquady + bottom)); //Get Node at centre

        //                    if (node != nullptr) //if not nullptr continue
        //                    {
        //                        Coordinates coord = node->pos; //Get coord from Node

        //                        std::vector<Coordinates> vecCoords; //create a list to use as a container for all points to be blurred

        //                        for (int j = y - storenum; j <= y + storenum; j++)
        //                            for (int i = x - storenum; i <= x + storenum; i++) //Iterate over the blur radius around the point
        //                            {
        //                                if (i < x) //if point is in another quad use the cached list otherwise read it from the loaded quad
        //                                {
        //                                    Node<Coordinates>* n = quad.Search(Coordinates(i + v * boundsperquadx + left, j + w * boundsperquady + bottom));

        //                                    if (n != nullptr)
        //                                    {
        //                                        Coordinates coord = n->pos;
        //                                        vecCoords.push_back(coord);
        //                                    }

        //                                }
        //                                else
        //                                {
        //                                    Coordinates coord(i + v * boundsperquadx + left, j + w * boundsperquady + bottom);
        //                                    for (int a = 0; a < listb[w][3].size(); a++)
        //                                    {
        //                                        if (listb[w][3][a]->pos == coord)
        //                                        {
        //                                            coord = listb[w][3][a]->pos;
        //                                            vecCoords.push_back(coord);
        //                                        }
        //                                    }

        //                                }
        //                            }

        //                        float zavg = 0.0f;

        //                        for (auto const c : vecCoords)
        //                        {
        //                            zavg += c.z;
        //                        }

        //                        zavg /= vecCoords.size();

        //                        coord.z = zavg;

        //                        smooth.Insert(new Node<Coordinates>(coord));
        //                    }
        //                }
        //        }
        //    }
        //}
        //else if (v > 1 && v < numquads) //inbetween rows of sub-quads: process both above and below
        //{
        //    //for loops for iterating through split level trees

        //    for (int w = 0; w < numquads; w++)
        //    {
        //        //Top
        //        for (int x = 0; x < boundsperquadx; x++)
        //            for (int y = boundsperquady - storenum - 1; y < boundsperquady; y++)
        //            {
        //                Node<Coordinates>* node = quad.Search(Coordinates(x + v * boundsperquadx + left, y + w * boundsperquady + bottom)); //Get Node at centre

        //                if (node != nullptr) //if not nullptr continue
        //                {
        //                    Coordinates coord = node->pos; //Get coord from Node

        //                    std::vector<Coordinates> vecCoords; //create a list to use as a container for all points to be blurred

        //                    for (int j = y - storenum; j <= y + storenum; j++)
        //                        for (int i = x - storenum; i <= x + storenum; i++) //Iterate over the blur radius around the point
        //                        {
        //                            if (j < y) //if point is in another quad use the cached list otherwise read it from the loaded quad
        //                            {
        //                                Node<Coordinates>* n = quad.Search(Coordinates(i + v * boundsperquadx + left, j + w * boundsperquady + bottom));

        //                                if (n != nullptr)
        //                                {
        //                                    Coordinates coord = n->pos;
        //                                    vecCoords.push_back(coord);
        //                                }

        //                            }
        //                            else
        //                            {
        //                                Coordinates coord(i + v * boundsperquadx + left, j + w * boundsperquady + bottom);
        //                                for (int a = 0; a < listc[w][1].size(); a++)
        //                                {
        //                                    if (listc[w][1][a]->pos == coord)
        //                                    {
        //                                        coord = listc[w][1][a]->pos;
        //                                        vecCoords.push_back(coord);
        //                                    }
        //                                }

        //                            }
        //                        }

        //                    float zavg = 0.0f;

        //                    for (auto const c : vecCoords)
        //                    {
        //                        zavg += c.z;
        //                    }

        //                    zavg /= vecCoords.size();

        //                    coord.z = zavg;

        //                    smooth.Insert(new Node<Coordinates>(coord));
        //                }
        //            }
        //        //Bottom
        //        for (int x = 0; x < boundsperquadx; x++)
        //            for (int y = 0; y < storenum; y++)
        //            {
        //                Node<Coordinates>* node = quad.Search(Coordinates(x + v * boundsperquadx + left, y + w * boundsperquady + bottom)); //Get Node at centre

        //                if (node != nullptr) //if not nullptr continue
        //                {
        //                    Coordinates coord = node->pos; //Get coord from Node

        //                    std::vector<Coordinates> vecCoords; //create a list to use as a container for all points to be blurred

        //                    for (int j = y - storenum; j <= y + storenum; j++)
        //                        for (int i = x - storenum; i <= x + storenum; i++) //Iterate over the blur radius around the point
        //                        {
        //                            if (j > y) //if point is in another quad use the cached list otherwise read it from the loaded quad
        //                            {
        //                                Node<Coordinates>* n = quad.Search(Coordinates(i + v * boundsperquadx + left, j + w * boundsperquady + bottom));

        //                                if (n != nullptr)
        //                                {
        //                                    Coordinates coord = n->pos;
        //                                    vecCoords.push_back(coord);
        //                                }

        //                            }
        //                            else
        //                            {
        //                                Coordinates coord(i + v * boundsperquadx + left, j + w * boundsperquady + bottom);
        //                                for (int a = 0; a < lista[w][0].size(); a++)
        //                                {
        //                                    if (lista[w][0][a]->pos == coord)
        //                                    {
        //                                        coord = lista[w][0][a]->pos;
        //                                        vecCoords.push_back(coord);
        //                                    }
        //                                }

        //                            }
        //                        }

        //                    float zavg = 0.0f;

        //                    for (auto const c : vecCoords)
        //                    {
        //                        zavg += c.z;
        //                    }

        //                    zavg /= vecCoords.size();

        //                    coord.z = zavg;

        //                    smooth.Insert(new Node<Coordinates>(coord));
        //                }
        //            }
        //        //Right
        //        if (w != (numquads - 1))
        //        {
        //            for (int x = boundsperquadx - storenum - 1; x < boundsperquadx; x++)
        //                for (int y = storenum; y < boundsperquady - storenum - 1; y++)
        //                {
        //                    Node<Coordinates>* node = quad.Search(Coordinates(x + v * boundsperquadx + left, y + w * boundsperquady + bottom)); //Get Node at centre

        //                    if (node != nullptr) //if not nullptr continue
        //                    {
        //                        Coordinates coord = node->pos; //Get coord from Node

        //                        std::vector<Coordinates> vecCoords; //create a list to use as a container for all points to be blurred

        //                        for (int j = y - storenum; j <= y + storenum; j++)
        //                            for (int i = x - storenum; i <= x + storenum; i++) //Iterate over the blur radius around the point
        //                            {
        //                                if (i > x) //if point is in another quad use the cached list otherwise read it from the loaded quad
        //                                {
        //                                    Node<Coordinates>* n = quad.Search(Coordinates(i + v * boundsperquadx + left, j + w * boundsperquady + bottom));

        //                                    if (n != nullptr)
        //                                    {
        //                                        Coordinates coord = n->pos;
        //                                        vecCoords.push_back(coord);
        //                                    }

        //                                }
        //                                else
        //                                {
        //                                    Coordinates coord(i + v * boundsperquadx + left, j + w * boundsperquady + bottom);
        //                                    for (int a = 0; a < listb[w][2].size(); a++)
        //                                    {
        //                                        if (listb[w][2][a]->pos == coord)
        //                                        {
        //                                            coord = listb[w][2][a]->pos;
        //                                            vecCoords.push_back(coord);
        //                                        }
        //                                    }

        //                                }
        //                            }

        //                        float zavg = 0.0f;

        //                        for (auto const c : vecCoords)
        //                        {
        //                            zavg += c.z;
        //                        }

        //                        zavg /= vecCoords.size();

        //                        coord.z = zavg;

        //                        smooth.Insert(new Node<Coordinates>(coord));
        //                    }
        //                }
        //        }
        //        //Left
        //        if (w != 0)
        //        {
        //            for (int x = 0; x < storenum; x++)
        //                for (int y = 0; y < boundsperquady; y++)
        //                {
        //                    Node<Coordinates>* node = quad.Search(Coordinates(x + v * boundsperquadx + left, y + w * boundsperquady + bottom)); //Get Node at centre

        //                    if (node != nullptr) //if not nullptr continue
        //                    {
        //                        Coordinates coord = node->pos; //Get coord from Node

        //                        std::vector<Coordinates> vecCoords; //create a list to use as a container for all points to be blurred

        //                        for (int j = y - storenum; j <= y + storenum; j++)
        //                            for (int i = x - storenum; i <= x + storenum; i++) //Iterate over the blur radius around the point
        //                            {
        //                                if (i < x) //if point is in another quad use the cached list otherwise read it from the loaded quad
        //                                {
        //                                    Node<Coordinates>* n = quad.Search(Coordinates(i + v * boundsperquadx + left, j + w * boundsperquady + bottom));

        //                                    if (n != nullptr)
        //                                    {
        //                                        Coordinates coord = n->pos;
        //                                        vecCoords.push_back(coord);
        //                                    }

        //                                }
        //                                else
        //                                {
        //                                    Coordinates coord(i + v * boundsperquadx + left, j + w * boundsperquady + bottom);
        //                                    for (int a = 0; a < listb[w][3].size(); a++)
        //                                    {
        //                                        if (listb[w][3][a]->pos == coord)
        //                                        {
        //                                            coord = listb[w][3][a]->pos;
        //                                            vecCoords.push_back(coord);
        //                                        }
        //                                    }

        //                                }
        //                            }

        //                        float zavg = 0.0f;

        //                        for (auto const c : vecCoords)
        //                        {
        //                            zavg += c.z;
        //                        }

        //                        zavg /= vecCoords.size();

        //                        coord.z = zavg;

        //                        smooth.Insert(new Node<Coordinates>(coord));
        //                    }
        //                }
        //        }
        //    }
        //    
        //}

    }
    std::cout << "\n";
}
