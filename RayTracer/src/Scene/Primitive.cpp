// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University

#include "Primitive.h"
#include "Ray.h"
#include "../common.h"
#include "../glut.h"
#include <iostream>


namespace RayTracer{
    Primitive::Primitive( vector3 _p1, vector3 _p2, vector3 _p3)
    {
        vector3 p12,p23;
        p12=_p1-_p2;
        p23=_p2-_p3;
        vector3 normal = p12.Cross(p23);
        float D = DOT( normal, _p3 ); //P is a point in the plane
        m_Plane.N=normal;
        m_Plane.D=D;
        p1=_p1;
        p2=_p2;
        p3=_p3;
        std::cout<<normal<<std::endl;
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

    void Primitive::render()
    {
        glColor3f(1,1,1);
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glBegin(GL_TRIANGLES);
        glVertex3fv(p1.cell);
        glVertex3fv(p2.cell);
        glVertex3fv(p3.cell);
        glEnd();

        glColor3f(1,0,1);
        glBegin(GL_LINES);
        vector3 center=p1+p2+p3;
        center=center/3.0f;
        glVertex3fv(center.cell);
        vector3 end;
        end = center+ m_Plane.N*0.3f;
        glVertex3fv(end.cell);
        glEnd();
    }

};