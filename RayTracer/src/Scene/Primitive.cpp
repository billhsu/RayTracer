// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University

#include "Primitive.h"
#include "Ray.h"
#include "../common.h"

namespace RayTracer{
    Primitive::Primitive( vector3 _p1, vector3 _p2, vector3 _p3)
    {
        vector3 normal = _p1.Cross(_p2);
        float D = DOT( normal, _p3 ); //P is a point in the plane
        m_Plane.N=normal;
        m_Plane.D=D;
        p1=_p1;
        p2=_p2;
        p3=_p3;
    }
    int Primitive::intersect(Ray& ray, float& distance)
    {
        float d = DOT( m_Plane.N, ray.GetDirection() );
        if (d != 0)
        {
            float dist = -(DOT( m_Plane.N, ray.GetOrigin() ) + m_Plane.D) / d;
            if (dist > 0)
            {
                if (dist < distance && isInside(ray.GetOrigin(),ray.GetOrigin()+ray.GetDirection()*dist))
                {
                    distance = dist;
                    return HIT;
                }
            }
        }
        return MISS;
    }

    //Check if a point is inside a triangle
    bool Primitive::isInside(vector3 p0, vector3 p)
    {
        vector3 v1,v2,n1;
        float d1;

        v1=p1-p;
        v2=p2-p;
        n1=v2.Cross(v1);
        n1.Normalize();
        d1=-DOT(p0,n1);
        if((DOT(p,n1)+d1)<0)
            return false;

        v1=p2-p;
        v2=p3-p;
        n1=v2.Cross(v1);
        n1.Normalize();
        d1=-DOT(p0,n1);
        if((DOT(p,n1)+d1)<0)
            return false;

        v1=p3-p;
        v2=p1-p;
        n1=v2.Cross(v1);
        n1.Normalize();
        d1=-DOT(p0,n1);
        if((DOT(p,n1)+d1)<0)
            return false;

        return true;

    }

};