#include "../Headers/Quadtree.h"
#include "../Headers/Coordinates.h"
#include "../Headers/Bit.h"
#include "../Headers/CatchMath.h"

#include <cmath>


template <typename T>
void Quadtree<T>::insert(Node<T>* node)
{
    if (node == nullptr)
        return;

    if (!inBoundary(node->pos))
    {
        return;
    }

    if (std::abs(topLeft.x - bottomRight.x) <= unit && std::abs(topLeft.y - bottomRight.y) <= unit)
    {
        if (n == nullptr)
            n = node;
        return;
    }

    if (((topLeft.x + bottomRight.x) / 2) <= node->pos.x)
    {
        // Indicates Top Right Tree 
        if (((topLeft.y + bottomRight.y) / 2) <= node->pos.y)
        {
            if (topRightTree == nullptr)
                topRightTree = new Quadtree<T>(
                    T((topLeft.x + bottomRight.x) / 2,
                        topLeft.y),
                    T(bottomRight.x,
                    (topLeft.y + bottomRight.y) / 2));
            topRightTree->insert(node);
        }

        // Indicates Bottom Right Tree 
        else
        {
            if (bottomRightTree == nullptr)
                bottomRightTree = new Quadtree<T>(
                    T((topLeft.x + bottomRight.x) / 2,
                    (topLeft.y + bottomRight.y) / 2),
                    T(bottomRight.x, bottomRight.y));
            bottomRightTree->insert(node);
        }
    }
    else
    {
        // Indicates Top Left Tree 
        if (((topLeft.y + bottomRight.y) / 2) <= node->pos.y)
        {
            if (topLeftTree == nullptr)
                topLeftTree = new Quadtree<T>(
                    T(topLeft.x, topLeft.y), T((topLeft.x + bottomRight.x) / 2, (topLeft.y + bottomRight.y) / 2));
            topLeftTree->insert(node);
        }

        // Indicates Bottom Left Tree 
        else
        {
            if (bottomLeftTree == nullptr)
                bottomLeftTree = new Quadtree<T>(
                    T(topLeft.x,
                    (topLeft.y + bottomRight.y) / 2),
                    T((topLeft.x + bottomRight.x) / 2,
                        bottomRight.y));
            bottomLeftTree->insert(node);
        }
    }
}

template<typename T>
Node<T>* Quadtree<T>::search(T p) const
{
    // Current quad cannot contain it 
        if (!inBoundary(p))
            return nullptr;

        // We are at a quad of unit length 
        // We cannot subdivide this quad further 
        if (n != nullptr && (p.x == n->pos.x && p.y == n->pos.y))
            return n;

        if (((topLeft.x + bottomRight.x) / 2) <= p.x)
        {
            // Indicates topRightTree 
            if ((topLeft.y + bottomRight.y) / 2 <= p.y)
            {
                if (topRightTree == nullptr)
                    return nullptr;
                return topRightTree->search(p);
            }

            // Indicates botRightTree 
            else
            {
                if (bottomRightTree == nullptr)
                    return nullptr;
                return bottomRightTree->search(p);
            }
        }
        else
        {

            // Indicates topLeftTree 
            if ((topLeft.y + bottomRight.y) / 2 <= p.y)
            {
                if (topLeftTree == nullptr)
                    return nullptr;
                return topLeftTree->search(p);
            }

            // Indicates botLeftTree 
            else
            {
                if (bottomLeftTree == nullptr)
                    return nullptr;
                return bottomLeftTree->search(p);
            }
        }

        return nullptr;
}

template<typename T>
bool Quadtree<T>::inBoundary(T p) const
{
    return (p.x >= topLeft.x && p.x <= bottomRight.x && p.y <= topLeft.y && p.y >= bottomRight.y);
}

template<typename T>
T Quadtree<T>::TopLeft() const
{
    return topLeft;
}

template<typename T>
T Quadtree<T>::BottomRight() const
{
    return bottomRight;
}

template class Quadtree<Coordinates>;
template class Quadtree<Bit>;
template class Quadtree<Vec3>;
template class Quadtree<Normal>;
