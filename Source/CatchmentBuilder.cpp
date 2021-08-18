#include "../Headers/CatchmentBuilder.h"
#include "../Headers/FileWriter.h"

#include <iostream>

void CatchmentBuilder::CreateCatchments(QuadtreeManager<Coordinates>& quad)
{
    std::cout << "Smoothing Surface\n";
    //Quadtree<Coordinates>* smoothQuad = SmoothPoints(quad);
    QuadtreeManager<Coordinates> smooth(quad.topLeft,quad.bottomRight);

    smooth.prePath = "Temp/SmoothTree/Tree";
    smooth.spacing = quad.spacing;
    smooth.splitlevel = quad.splitlevel;
    smooth.SetTreeType(quad.type);

    SmoothPoints(quad, smooth);
    std::cout << "Exporting Original Surface\n";
    FileWriter::WriteCoordTreeASC("./Exports/Surfaces/Original", quad);

    std::cout << "Exporting Smoothed Surface\n";
    FileWriter::WriteCoordTreeASC("./Exports/Surfaces/Smooth", smooth);
    
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

void CatchmentBuilder::SmoothPoints(QuadtreeManager<Coordinates>& quad, QuadtreeManager<Coordinates>& smooth)
{
    double boundsx = (quad.BottomRight().x) - (quad.TopLeft().x);
    double boundsy = (quad.TopLeft().y) - (quad.BottomRight().y);
    double bottom = (quad.BottomRight().y);
    double left = (quad.TopLeft().x);

    int numquads = quad.splitlevel * 2; //quad splits the area in half in the x and y axis
    double boundsperquadx = boundsx / numquads;
    double boundsperquady = boundsy / numquads;

    std::vector<Coordinates> list;

    //for loop for iterating through split level trees

    //for loops for iterating through coords in a split level tree

    for (int y = 0; y <= boundsy; y++)
    {
        std::cout << "\r" << (((y + 1) / (boundsy)) * 100) << "% Smoothed";
        for (int x = 0; x <= boundsx; x++)
        {
            Node<Coordinates>* node = quad.Search(Coordinates(x + left, y + bottom));
            if (node != nullptr)
            {
                Coordinates coord = node->pos;

                std::vector<Coordinates> vecCoords;

                for (int j = y - 1; j <= y + 1; j++)
                    for (int i = x - 1; i <= x + 1; i++)
                    {
                        Node<Coordinates>* n = quad.Search(Coordinates(i + left, j + bottom));

                        if (n != nullptr)
                        {
                            Coordinates coord = n->pos;
                            vecCoords.push_back(coord);
                        }
                    }
                if (vecCoords.size() == 0)
                {
                    float zavg = 0.0f;

                    for (auto const c : vecCoords)
                    {
                        zavg += c.z;
                    }

                    zavg /= vecCoords.size();

                    coord.z = zavg;

                    smooth.Insert(new Node<Coordinates>(coord));
                }
                else
                {
                    smooth.Insert(new Node<Coordinates>(coord));
                }
            }
        }
    }

    std::cout << "\n\n";
}
