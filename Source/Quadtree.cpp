#include "../Headers/Quadtree.h"
#include "../Headers/Coordinates.h"
#include "../Headers/Bit.h"
#include "../Headers/CatchMath.h"
#include "../Headers/FlowClassification.h"
#include "../Headers/Catchment.h"

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
template class Quadtree<FlowDirection>;
template class Quadtree<FlowGeneral>;
