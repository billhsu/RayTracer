// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University

#include "Ray.h"

namespace RayTracer{

    Ray::Ray( vector3& a_Origin, vector3& a_Dir ) : 
        m_Origin( a_Origin ), 
        m_Direction( a_Dir )
    {
    }

};