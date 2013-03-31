// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University

#include <stdlib.h>
#include "glut.h"
#include <iostream>
#include <vector>

#include "Scene/Primitive.h"
#include "Scene/Ray.h"
#include "common.h"
std::vector<RayTracer::Ray> rayList;

float rot_x=0.0f,rot_y=0.0f;

void init(void)
{
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel (GL_FLAT);

    for(int theta=0;theta<30;++theta)
    {
        for(int phi=-15;phi<15;++phi)
        {
            RayTracer::vector3 dir;
            dir.x = cosf(2.0f*PI/30.0f*theta)*sinf(2.0f*PI/30.0f*phi);
            dir.y = sinf(2.0f*PI/30.0f*theta)*sinf(2.0f*PI/30.0f*phi);
            dir.z = cosf(2.0f*PI/30.0f*phi);
            RayTracer::Ray ray(RayTracer::vector3(1.5f,0.0f,0.0f), dir);
            ray.strength=1.0f;
            ray.active=true;
            rayList.push_back(ray);
        }
    }
}

void display(void)
{
    glClear (GL_COLOR_BUFFER_BIT);
    glColor3f (1.0, 1.0, 1.0);
    glLoadIdentity ();             /* clear the matrix */
    /* viewing transformation  */
    gluLookAt (3.0, 4.0, -5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glRotatef(rot_x,1.0f,0.0f,0.0f);
    glRotatef(rot_y,0.0f,1.0f,0.0f);
    glPushMatrix();
    glScalef (2.0, 1.0, 1.0);      /* modeling transformation */
    glutWireCube (3.0);
    glPopMatrix();
    glLineWidth(1.0); 
    glColor3f(1.0, 0.0, 0.0);
    for(unsigned int i=0;i<rayList.size();++i)
    {
        if(!rayList[i].active) continue;
        glBegin(GL_LINES);
        RayTracer::vector3 end,dir;
        end=rayList[i].GetOrigin()+rayList[i].GetDirection()*0.1f;
        dir = rayList[i].GetDirection();
        if(fabs(end.x)>=3.0f)
        {
            rayList[i].SetDirection(RayTracer::vector3(-dir.x,dir.y,dir.z));
            glColor3f(1.0, 1.0, 0.0);
            if(rayList[i].strength>=0.25)rayList[i].strength-=0.25f;
        }
        else if(fabs(end.y)>=1.5f)
        {
            rayList[i].SetDirection(RayTracer::vector3(dir.x,-dir.y,dir.z));
            glColor3f(1.0, 1.0, 0.0);
            if(rayList[i].strength>=0.25)rayList[i].strength-=0.25f;
        }
        else if(fabs(end.z)>=1.5f)
        {
            rayList[i].SetDirection(RayTracer::vector3(dir.x,dir.y,-dir.z));
            glColor3f(1.0, 1.0, 0.0);
            if(rayList[i].strength>=0.25)rayList[i].strength-=0.25f;
        }
        else {glColor3f(rayList[i].strength, 0.0, 0.0);}
        glVertex3f(rayList[i].GetOrigin().x, rayList[i].GetOrigin().y, rayList[i].GetOrigin().z);
        glVertex3f(end.x, end.y, end.z);
        glEnd();
        rayList[i].SetOrigin(rayList[i].GetOrigin()+rayList[i].GetDirection()*(0.001f*glutGet(GLUT_ELAPSED_TIME)/10000.0f));
        rayList[i].strength -= 0.00005f*glutGet(GLUT_ELAPSED_TIME)/10000.0f;
        if(rayList[i].strength<=0.0f) rayList[i].active=false;
    }
    
    glutPostRedisplay();
    glFlush ();
}

void reshape (int w, int h)
{
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
    std::cout<<"width: "<<w<<" height:"<<h<<std::endl;
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glFrustum (-1.0*w/h, 1.0*w/h, -1.0, 1.0, 1.5, 20.0);
    glMatrixMode (GL_MODELVIEW);
}

void mouse(int x,int y)
{
    rot_y=(x-300.f)/300.0f*180.0f;
    rot_x=-(y-200.f)/200.0f*180.0f;
}
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize (600, 400);
    glutInitWindowPosition (100, 100);
    glutCreateWindow (argv[0]);
    init ();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutPassiveMotionFunc(mouse);
    glutMainLoop();
    return 0;
}
