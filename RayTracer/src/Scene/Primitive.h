// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University

//Class for primitive scene object
#ifndef PRIMITIVE_H
#define PRIMITIVE_H
#include "../common.h"

namespace RayTracer{
    class Ray;

    class Primitive
    {
    public:
        Primitive( vector3 _p1, vector3 _p2, vector3 _p3);
        Primitive( vector3 _p1, vector3 _p2, vector3 _p3, vector3 N, float D)
            :p1(_p1),p2(_p2),p3(_p3),m_Plane( plane( N, D ) ){};
        vector3& GetNormal() { return m_Plane.N; }
        float GetD() { return m_Plane.D; }
        int intersect(Ray& ray, float& distance);
        bool isInside(vector3 p0, vector3 p);
        void render();
    private:
        plane m_Plane;
        vector3 p1,p2,p3;
    };
};
#endif