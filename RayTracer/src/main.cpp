// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University

#include <stdlib.h>
#include "glut.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <windows.h>
#include <string>
#include <sstream>
#include "Scene/Scene.h"
#include "Scene/Primitive.h"
#include "Scene/Ray.h"
#include "common.h"
#include "time.h"
#include "Audio/hrtf.h"
#include "Audio/wav.h"

hrtf mhrtf("data\\hrtf");
float rot_x=0.0f,rot_y=0.0f;
DWORD startTime;
RayTracer::Scene scene;
std::vector<RayTracer::Ray> rayListTmp;

wav mWav;
float response_r[1024]={0.0f};
float response_l[1024]={0.0f};

float const time441k = 22.675736961451247165532879818594f;
struct respond
{
    RayTracer::vector3 direction;
    float strength;
    int time;
};
std::vector<respond> respondList;
short* music;
//Compute ray tracing by ray simulation 
void initCalc()
{
    long filelen;
    music = mWav.readWavFileData("Res/default.wav",filelen);
    printf("%d\n",sizeof(music));
    mWav.openDevice();
    RayTracer::Scene scene;
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
            ray.microseconds=0;
            ray.totalDist = 0.0f;
            ray.active=true;
            rayListTmp.push_back(ray);
        }
    }

    glutGet(GLUT_ELAPSED_TIME);
    std::cout<<"Sound position:(1.5,0,0)  Listener position: (-1.5,0,0)"<<std::endl;
    startTime = GetTickCount();
    RayTracer::Primitive p= RayTracer::Primitive(RayTracer::vector3(1,-1.5,-1.5),
        RayTracer::vector3(1,-1.5,1.5),
        RayTracer::vector3(0,1,0));
    //scene.primList.push_back(p);
    scene.loadObj("Res/Scene.obj");
    //Compute delays
    int active_rays = rayListTmp.size();
    clock_t start, finish;
    start = clock();
    while(active_rays>0)
    {
        for(unsigned int i=0;i<rayListTmp.size();++i)
        {
            if(rayListTmp[i].active==false) continue;
            
            float dist_=1000.0f;
            int which = scene.intersect(rayListTmp[i],dist_);
            if(which!=MISS)
            {
                rayListTmp[i].distList.push_back(dist_);
                if(rayListTmp[i].totalDist>=10.0f || rayListTmp[i].strength<=0.0)
                {
                    rayListTmp[i].active=false;
                    active_rays--;
                    continue;
                }
            }

            RayTracer::vector3 L = RayTracer::vector3(-1.5f,0.0f,0.0f) - rayListTmp[i].GetOrigin();
            float Tca = DOT(L,rayListTmp[i].GetDirection());
            float d2;
            float dist_to_listener = 1000.0f;
            if(Tca>0)
            {
                d2 = DOT(L,L) - Tca*Tca;
                if(d2<0.25f)
                {
                    float Thc = sqrt(0.25f-d2);
                    dist_to_listener = Tca-Thc;
                }
            }
            if(dist_>dist_to_listener)
            {
                rayListTmp[i].totalDist+=(dist_to_listener);
                rayListTmp[i].strength-=dist_to_listener/20.0f;
                if(rayListTmp[i].strength<=0.0f)rayListTmp[i].strength=0.0f;
                rayListTmp[i].active=false;
                active_rays--;

                //std::cout<<rayListTmp[i].totalDist/0.000340f<<"¦Ìs, "<<rayListTmp[i].strength<<" "<<
                //    rayListTmp[i].GetDirection()<<std::endl;
                //std::cout<<rayListTmp[i].GetDirection()<<" ";
                //mhrtf.getHRTF(rayListTmp[i].GetDirection());
                respond respnd;
                respnd.strength=rayListTmp[i].strength;
                respnd.time=(int)((rayListTmp[i].totalDist/0.000340f)/time441k);
                respnd.direction = rayListTmp[i].GetDirection();
                respondList.push_back(respnd);
            }
            if(which != MISS)
            {
                rayListTmp[i].totalDist+=dist_;
                rayListTmp[i].strength-=dist_/20.0f;
                rayListTmp[i].strength-=0.25f;
                RayTracer::vector3 end=rayListTmp[i].GetOrigin()+rayListTmp[i].GetDirection()*(dist_*0.999f);
                RayTracer::vector3 dir=-2*DOT(scene.primList[which].GetNormal(),rayListTmp[i].GetDirection())
                    *scene.primList[which].GetNormal()+rayListTmp[i].GetDirection();
                dir.Normalize();
                rayListTmp[i].SetDirection(dir);
                rayListTmp[i].SetOrigin(end);
            }
            
        }
    }

    /*std::ifstream in("data/hrtf_0_0_r.txt");
    float hrtf[128]={0.0f};
    for(int i=0;i<128;++i) in>>hrtf[i];*/
    float* hrtf; 
    float response[2048]={0.0f};
    hrtf::ir_both ir;
    for(int i=0;i<respondList.size();++i)
    {
        ir = mhrtf.getHRTF(respondList[i].direction);
        hrtf = ir.ir_l;
        for(int j=0;j<512;++j)
        {
            response[respondList[i].time+j]+=(hrtf[j]*respondList[i].strength);
        }
    }
    finish = clock();
    double duration = (double)(finish - start) / CLOCKS_PER_SEC;
    printf( "%f seconds\n", duration );

    std::ofstream out("data/response.txt");
    out<<"a =[";
    for(int i=0;i<1024;++i) out<<response[i]<<" ";
    out<<"]"<<std::endl;

}
std::vector<RayTracer::Ray> rayList;

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
            ray.microseconds=0;
            ray.totalDist = 0.0f;
            ray.active=true;
            rayList.push_back(ray);
        }
    }
    /*RayTracer::Ray ray(RayTracer::vector3(1.5f,0.0f,0.0f), RayTracer::vector3(-1.0f,0.0f,0.0f));
    ray.strength=1.0f;
    ray.milliseconds=0;
    ray.active=true;
    ray.GetDirection().Normalize();
    rayList.push_back(ray);*/
    glutGet(GLUT_ELAPSED_TIME);
    std::cout<<"Sound position:(1.5,0,0)  Listener position: (-1.5,0,0)"<<std::endl;
    startTime = GetTickCount();
    RayTracer::Primitive p= RayTracer::Primitive(RayTracer::vector3(1,-1.5,-1.5),
        RayTracer::vector3(1,-1.5,1.5),
        RayTracer::vector3(0,1,0));
    //scene.primList.push_back(p);
    
    scene.loadObj("Res/Scene.obj");

    
    

}
//Compute ray tracing by per frame ray simulation 
void display(void)
{
    glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
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

    glLineWidth(2.0); 
    glColor3f(1.0, 0.0, 0.0);

    int active_cnt=0;
    glColor3f(1.0, 0.0, 1.0);

    for(unsigned int i=0;i<rayList.size();++i)
    {
        if(!rayList[i].active) continue;
        active_cnt++;
        glBegin(GL_LINES);
        RayTracer::vector3 end,dir;
        end=rayList[i].GetOrigin()+rayList[i].GetDirection()*0.1f;
        float dist_=0.1f;
        int which = scene.intersect(rayList[i],dist_);
        rayList[i].microseconds++;
        if(which!=MISS)
        {
            end=rayList[i].GetOrigin()+rayList[i].GetDirection()*dist_;
            RayTracer::vector3 dir=-2*DOT(scene.primList[which].GetNormal(),rayList[i].GetDirection())
                *scene.primList[which].GetNormal()+rayList[i].GetDirection();
            dir.Normalize();
            rayList[i].SetDirection(dir);
            rayList[i].SetOrigin(end);
            rayList[i].strength-=0.25f;
            glColor3f(1.0f,1.0f,0.0f);
        }
        else glColor3f(rayList[i].strength,0.0f,0.0f);
        dir = rayList[i].GetDirection();
        RayTracer::vector3 dist = end-RayTracer::vector3(-1.5f,0.0f,0.0f);
        if(dist.Length()<=0.5f)
        {
            rayList[i].active=false;
            std::cout<<"Hit# "<<rayList[i].microseconds<<"¦Ìs, Strength:"<<
                rayList[i].strength<<", Direction:"<<rayList[i].GetDirection()<<std::endl;
            glColor3f(1.0, 1.0, 0.0);
        }
        
        glVertex3f(rayList[i].GetOrigin().x, rayList[i].GetOrigin().y, rayList[i].GetOrigin().z);
        glVertex3f(end.x, end.y, end.z);
        glEnd();
        rayList[i].SetOrigin(rayList[i].GetOrigin()+rayList[i].GetDirection()*0.000340f);
        rayList[i].strength -= 0.000005f;
        if(rayList[i].strength<=0.0f) rayList[i].active=false;
        
    }
    if(active_cnt==0)
    {
        init();
        std::cout<<"New Wave"<<std::endl;
    }
    scene.render();
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
    initCalc();
    init ();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutPassiveMotionFunc(mouse);

    glutMainLoop();
    return 0;
}
