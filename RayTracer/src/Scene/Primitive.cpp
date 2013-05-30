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
        normal.Normalize();
        float D = DOT( normal, _p3 );
        m_Plane.N=normal;
        m_Plane.D=D;
        p1=_p1;
        p2=_p2;
        p3=_p3;
        //std::cout<<normal<<std::endl;
    }
    int Primitive::intersect(Ray& ray, float& distance)
    {
        float d = DOT( m_Plane.N, ray.GetDirection() );
        if(d>0)
        {
            m_Plane.N=-m_Plane.N;
            m_Plane.N.Normalize();
            m_Plane.D=DOT( m_Plane.N, p1 );
        }
        if (d != 0)
        {
            float dist = DOT((p1-ray.GetOrigin()),m_Plane.N)/(DOT(ray.GetDirection(),m_Plane.N));
            if (dist > 0)
            {
                if (dist < distance)
                {
                    if(isInside(ray.GetOrigin()+ray.GetDirection()*dist))
                    {
                        distance = dist;
                        return HIT;
                    }
                }
            }
        }
        return MISS;
    }

    //Check if a point is inside a triangle
    bool Primitive::isInside(vector3 p)
    {
        if (sameSide(p, p1, p2, p3) && sameSide(p,p2, p1, p3) && sameSide(p, p3, p1, p2))
            return true;
        else return false;

    }
    bool Primitive::sameSide(vector3 p1,vector3 p2,vector3 a,vector3 b)
    {
        vector3 cp1 = (b-a).Cross(p1-a);
        vector3 cp2 = (b-a).Cross(p2-a);
        if (DOT(cp1, cp2) >= 0) return true;
        else return false;
    }

    void Primitive::render()
    {
        glColor4f(1,1,1,0.3);
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glBegin(GL_TRIANGLES);
        glVertex3fv(p1.cell);
        glVertex3fv(p2.cell);
        glVertex3fv(p3.cell);
        glEnd();

        glColor4f(1,1,1,0.7);
        glBegin(GL_LINES);
        /*vector3 center=p1+p2+p3;
        center=center/3.0f;
        glVertex3fv(center.cell);
        vector3 end;
        end = center+ m_Plane.N*0.3f;
        glVertex3fv(end.cell);*/
        glVertex3fv(p1.cell);
        glVertex3fv(p2.cell);
        glVertex3fv(p2.cell);
        glVertex3fv(p3.cell);
        glEnd();
    }

};