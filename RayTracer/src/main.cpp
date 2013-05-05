// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University
#define WIN32_LEAN_AND_MEAN
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
#include "Math/Matrices.h"
#include "SerialPort/SerialPort.h"

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
RayTracer::vector3 origin = RayTracer::vector3(0.0f,0.0f,0.0f);
RayTracer::vector3 listener  = RayTracer::vector3(1.0f,0.0f,1.0f);
float rot_z = 0.0f;

bool start_flag = false;
int start_match_pos = 0;
int recv_cnt = 0;

BYTE recv_data[26]={0};
BYTE start_mark[]={0xa5,0x5a,0x12,0xa1};
int imu_result[14]={0};
float yaw=0.0f,pitch=0.0f,roll=0.0f;



SerialPort serial;

void initCalc()
{
    long filelen;
    music = mWav.readWavFileData("Res/tada.wav",filelen);
    rayListTmp.clear();
    respondList.clear();
    memset(response_l,0,1024);
    memset(response_r,0,1024);
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
            RayTracer::Ray ray(origin, dir);
            ray.strength=1.0f;
            ray.microseconds=0;
            ray.totalDist = 0.0f;
            ray.active=true;
            rayListTmp.push_back(ray);
        }
    }

    std::cout<<"Sound position:"<<origin<<" Listener position: "<<listener<<std::endl;
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

            RayTracer::vector3 L = listener - rayListTmp[i].GetOrigin();
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
                rayListTmp[i].strength-=dist_/40.0f;
                rayListTmp[i].strength/=4;
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
    printf("stage 2\n");
    float* hrtf; 
    hrtf::ir_both ir;
    for(unsigned int i=0;i<respondList.size();++i)
    {
        Matrix4 m1;
        m1.rotateY(rot_z);
        Vector3 newDir;
        newDir.x=respondList[i].direction.x;
        newDir.y=respondList[i].direction.y;
        newDir.z=respondList[i].direction.z;
        newDir=m1*newDir;
        respondList[i].direction.x=newDir.x;
        respondList[i].direction.y=newDir.y;
        respondList[i].direction.z=newDir.z;
        ir = mhrtf.getHRTF(respondList[i].direction);
        hrtf = ir.ir_l;
        for(int j=0;j<128;++j)
        {
            response_l[respondList[i].time+j]+=(hrtf[j]*respondList[i].strength);
        }
        hrtf = ir.ir_r;
        for(int j=0;j<128;++j)
        {
            response_r[respondList[i].time+j]+=(hrtf[j]*respondList[i].strength);
        }
    }
    short* buffer2 = new short[71296*2];
    float response[1024]={0.0};
    response[0] = 1.0f;
    int i, j, k;
    int kernelSize=1024;
    int dataSize = 71296;
    for(i = kernelSize-1; i < dataSize; ++i)
    {
        buffer2[i*2] = 0;                             // init to 0 before accumulate

        for(j = i, k = 0; k < kernelSize; --j, ++k)
            buffer2[i*2] += music[j*2] * response_l[k];
    }

    // convolution from out[0] to out[kernelSize-2]
    for(i = 0; i < kernelSize - 1; ++i)
    {
        buffer2[i*2] = 0;                             // init to 0 before sum

        for(j = i, k = 0; j >= 0; --j, ++k)
            buffer2[i*2] += music[j*2] * response_l[k];
    }
    
    //////////////////////////////////////////////////////////////////////////
    for(i = kernelSize-1; i < dataSize; ++i)
    {
        buffer2[i*2+1] = 0;                             // init to 0 before accumulate

        for(j = i, k = 0; k < kernelSize; --j, ++k)
            buffer2[i*2+1] += music[j*2+1] * response_r[k];
    }

    // convolution from out[0] to out[kernelSize-2]
    for(i = 0; i < kernelSize - 1; ++i)
    {
        buffer2[i*2+1] = 0;                             // init to 0 before sum

        for(j = i, k = 0; j >= 0; --j, ++k)
            buffer2[i*2+1] += music[j*2+1] * response_r[k];
    }


    
    finish = clock();
    mWav.playWave(buffer2,71296*4);
    free(music);
    free(buffer2);
    mWav.closeDevice();
    double duration = (double)(finish - start) / CLOCKS_PER_SEC;
    printf( "%f seconds\n", duration );

    std::ofstream out("data/response.txt");
    out<<"a =[";
    for(int i=0;i<1024;++i) 
    {
        out<<response_l[i]<<" "<<response_r[i]<<"; ";
    }
    out<<"]"<<std::endl;
    

}
std::vector<RayTracer::Ray> rayList;

void keyinput(unsigned char key, int x, int y)
{
    Matrix4 m1,m2;
    m1.rotateY(rot_z);
    Vector3 vx=Vector3(-0.1,0,0);
    vx=m1*vx;
    m2.rotateY(rot_z+90);
    Vector3 vz=Vector3(-0.1,0,0);
    vz=m2*vz;
    switch(key)
    {
    case 'w':
        listener.x+=vx.x;
        listener.z+=vx.z;
        break;
    case 's':
        listener.x-=vx.x;
        listener.z-=vx.z;
        break;
    case 'a':
        listener.x+=vz.x;
        listener.z+=vz.z;
        break;
    case 'd':
        listener.x-=vz.x;
        listener.z-=vz.z;
        break;
    case 'j':
        rot_z+=1.5f;
        break;
    case 'k':
        rot_z-=1.5f;
        break;
    case 'p':
        initCalc();
        break;
    case 27:
        exit(0);
        break;
    }
}


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
            RayTracer::Ray ray(origin, dir);
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
    //std::cout<<"Sound position:(1.5,0,0)  Listener position: (-1.5,0,0)"<<std::endl;
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
    
    glTranslatef (listener.x, listener.y, listener.z);
    glRotatef(rot_z,0.0f,1.0f,0.0f);
    glBegin(GL_LINES);
    glColor3f(1.0, 1.0, 0.0);
    glVertex3f(0,0,0);
    glVertex3f(-1,0,0);
    glEnd();
    glColor3f(0.5, 0.5, 0.5);
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
        RayTracer::vector3 dist = end-listener;
        if(dist.Length()<=0.5f)
        {
            rayList[i].active=false;
            //std::cout<<"Hit# "<<rayList[i].microseconds<<"¦Ìs, Strength:"<<
            //    rayList[i].strength<<", Direction:"<<rayList[i].GetDirection()<<std::endl;
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
void recv_callback(int length, BYTE* recv)
{
    for(int i=0;i<length;++i)
    {

        BYTE b = recv[i];
        if (b<0) b+= 256;
        if (!start_flag)
        {
            if (b == start_mark[start_match_pos])
            {

                start_match_pos++;
                if (start_match_pos == 4)
                {
                    start_flag = true; 
                    recv_cnt = 0;
                    start_match_pos = 0;
                }
            }
            else start_match_pos = 0;
        }
        else
        {

            recv_data[recv_cnt] = b;
            ++recv_cnt;
            if (recv_cnt == 16) 
            {
                for (int ia = 0; ia < 6; ia+=2)
                {
                    imu_result[ia / 2] = (recv_data[ia] << 8 | recv_data[ia + 1]);
                    if (imu_result[ia / 2] >= 32768)
                    {
                        imu_result[ia / 2] -= 32768;
                        imu_result[ia / 2] = -imu_result[ia / 2];
                    }
                }
                recv_cnt = 0;
                yaw = imu_result[0] / 10.0;
                pitch = imu_result[1] / 10.0;
                roll = imu_result[2] / 10.0;
                start_flag = false;
                rot_y=yaw;

            }
        }
    }


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
    glutKeyboardFunc(keyinput);
    std::cout<<"Serial Port Status"<<serial.open("COM22");
    std::cout<<serial.set_option(115200,0,8,0,0)<<"\n";
    serial.recv_callback(recv_callback);
    glutMainLoop();
    serial.close();
    return 0;
}
