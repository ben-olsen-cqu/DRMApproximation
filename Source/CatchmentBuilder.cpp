#include "../Headers/CatchmentBuilder.h"
#include "../Headers/FileWriter.h"

#include <iostream>

void CatchmentBuilder::CreateCatchments(const Quadtree<Coordinates>* quad)
{
    //std::cout << "Smoothing Surface\n";
    //Quadtree<Coordinates>* smoothQuad = SmoothPoints(quad);

    //auto smoothTL = Normal(smoothQuad->TopLeft().x + 0.5f, smoothQuad->TopLeft().y - .5f);
    //auto smoothBR = Normal(smoothQuad->BottomRight().x - 0.5f, smoothQuad->BottomRight().y + 0.5f);

    //Quadtree<Normal>* normalquad = new Quadtree<Normal>(smoothTL, smoothBR);

    //std::cout << "Exporting Smoothed Surface\n";
    //FileWriter::WriteCoordTreeASC("Surfaces/Smooth", smoothQuad);
    //FileWriter::WriteCoordTreeASC("Surfaces/Original", quad);

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
    ////FileWriter::WriteVecNormals3dWKT("Normals/SmoothNormals3dWKT", normalquad);
    ////FileWriter::WriteVecNormals2dWKT("Normals/SmoothNormals2dWKT", normalquad);

    //delete normalquad;
    //delete smoothQuad;
}

Quadtree<Coordinates>* CatchmentBuilder::SmoothPoints(const Quadtree<Coordinates>* quad)
{
    

    /*auto smoothTL = Coordinates(quad->TopLeft().x + 3, quad->TopLeft().y - 3);
    auto smoothBR = Coordinates(quad->BottomRight().x - 3, quad->BottomRight().y + 3);

    float boundsx = (quad->BottomRight().x) - (quad->TopLeft().x) - 6;
    float boundsy = (quad->TopLeft().y) - (quad->BottomRight().y) - 6;
    float bottom = (quad->BottomRight().y) + 3;
    float left = (quad->TopLeft().x) + 3;

    Quadtree<Coordinates>* smoothquad = new Quadtree<Coordinates>(smoothTL, smoothBR);

    for (int y = 0; y <= boundsy; y++)
        for (int x = 0; x <= boundsx; x++)
        {
            Coordinates coord = quad->search(Coordinates(x + left, y + bottom))->pos;
            
            std::vector<Coordinates> vecCoords;

            for (int j = y - 3; j < y + 3; j++)
                for (int i = x - 3; i < x + 3; i++)
                {
                    vecCoords.push_back(Coordinates(quad->search(Coordinates(i + left, j + bottom))->pos));
                }

            float zavg = 0.0f;

            for (auto c : vecCoords)
            {
                zavg += c.z;
            }

            zavg /= vecCoords.size();

            coord.z = zavg;

            smoothquad->insert(new Node<Coordinates>(coord));

        }

    return smoothquad;*/
    return nullptr;
}
