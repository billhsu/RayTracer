// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University

#include <stdlib.h>
#include "glut.h"
#include <iostream>
#include <vector>
#include <windows.h>

#include "Scene/Primitive.h"
#include "Scene/Ray.h"
#include "common.h"
std::vector<RayTracer::Ray> rayList;

float rot_x=0.0f,rot_y=0.0f;
DWORD startTime;
RayTracer::Primitive *p;
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
            ray.milliseconds=0;
            ray.active=true;
            rayList.push_back(ray);
        }
    }
    glutGet(GLUT_ELAPSED_TIME);
    std::cout<<"Sound position:(1.5,0,0)  Listener position: (-1.5,0,0)"<<std::endl;
    startTime = GetTickCount();
    p= new RayTracer::Primitive(RayTracer::vector3(0,0,0),
        RayTracer::vector3(0,0,1),
        RayTracer::vector3(0,1,-1));
}

void display(void)
{
    glClearColor (0.5f, 0.5f, 0.5f, 0.0f);
    glClear (GL_COLOR_BUFFER_BIT);
    glColor3f (1.0, 1.0, 1.0);
    glLoadIdentity ();             /* clear the matrix */
    /* viewing transformation  */
    gluLookAt (3.0, 4.0, -5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glRotatef(rot_x,1.0f,0.0f,0.0f);
    glRotatef(rot_y,0.0f,1.0f,0.0f);
    
    glPushMatrix();
    glTranslatef (-1.5f, 0.0f, 0.0f);
    glutWireSphere (0.5f, 10.0f, 10.0f);
    glPopMatrix();

    glPushMatrix();
    glScalef (2.0, 1.0, 1.0);      /* modeling transformation */
    glutWireCube (3.0);
    glPopMatrix();

    glLineWidth(1.0); 
    glColor3f(1.0, 0.0, 0.0);
    int milliseconds = glutGet(GLUT_ELAPSED_TIME);
    int active_cnt=0;
    for(unsigned int i=0;i<rayList.size();++i)
    {
        if(!rayList[i].active) continue;
        active_cnt++;
        glBegin(GL_LINES);
        RayTracer::vector3 end,dir;
        end=rayList[i].GetOrigin()+rayList[i].GetDirection()*0.1f;
        dir = rayList[i].GetDirection();
        RayTracer::vector3 dist = end-RayTracer::vector3(-1.5f,0.0f,0.0f);
        if(dist.Length()<=0.5f)
        {
            rayList[i].active=false;
            rayList[i].milliseconds = GetTickCount()-startTime;
            std::cout<<"Hit# "<<rayList[i].milliseconds<<"ms, Strength:"<<
                rayList[i].strength<<", Direction:"<<rayList[i].GetDirection()<<std::endl;
            glColor3f(1.0, 1.0, 0.0);
        }
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
        rayList[i].SetOrigin(rayList[i].GetOrigin()+rayList[i].GetDirection()*(0.001f*milliseconds/10000.0f));
        rayList[i].strength -= 0.00005f*milliseconds/10000.0f;
        if(rayList[i].strength<=0.0f) rayList[i].active=false;
        
    }
    if(active_cnt==0)
    {
        init();
        std::cout<<"New Wave"<<std::endl;
    }
    p->render();
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
