// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University

//Defines a ray object
#ifndef RAY_H
#define RAY_H

#include "../common.h"

namespace RayTracer{

    class Ray
    {
    public:
        Ray() : m_Origin( vector3( 0, 0, 0 ) ), m_Direction( vector3( 0, 0, 0 ) ) {};
        Ray( vector3& a_Origin, vector3& a_Dir );
        void SetOrigin( vector3& a_Origin ) { m_Origin = a_Origin; }
        void SetDirection( vector3& a_Direction ) { m_Direction = a_Direction; }
        vector3& GetOrigin() { return m_Origin; }
        vector3& GetDirection() { return m_Direction; }
        float strength;
        bool active;
        int microseconds;
    private:
        vector3 m_Origin;
        vector3 m_Direction;
    };
};
#endif
