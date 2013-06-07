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
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

boost::mutex mutex;


void initCalc();
hrtf mhrtf("data\\hrtf");
float rot_x=0.0f,rot_y=0.0f;
DWORD startTime;
RayTracer::Scene scene;
std::vector<RayTracer::Ray> rayListTmp;

wav mWav;
float *response_r;
float *response_l;

float const time441k = 22.675736961451247165532879818594f;
struct respond
{
    RayTracer::vector3 direction;
    float strength;
    int time;
    bool operator<(const respond &rhs) const { return strength > rhs.strength; }
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
struct convData
{
    short* buffer;
    short* buffer_last;
    short* music;
    int dataSize;
    int kernelSize;
    float* response_l;
    float* response_r;
};
const int divide = 2;
short* volatile buffer;
short* volatile buffer_old;
short* volatile buffer_last;
DWORD WINAPI convThread(LPVOID data) {
    boost::lock_guard<boost::mutex> m_csLock(mutex);
    hrtf::convAudio(((convData*)data)->buffer,((convData*)data)->buffer_last,
        ((convData*)data)->music,((convData*)data)->dataSize,
        ((convData*)data)->kernelSize,((convData*)data)->response_l,
        ((convData*)data)->response_r);
    return 0;
}
DWORD WINAPI playwavThread(LPVOID dataSize) {
    mWav.playWave(buffer_old,*(int*)dataSize*4);
    return 0;
}
DWORD WINAPI waveThread(LPVOID data) {
    while( true )
    {
        initCalc();
        //Sleep(20-(end-start)*CLOCKS_PER_SEC/1000.0);
    }
    return 0;
}


void initCalc()
{
    
    long filelen;
    mWav.setMutex(&mutex);
    music = mWav.readWavFileData("Res/tada.wav",filelen);
    rayListTmp.clear();
    respondList.clear();
    response_l=new float[1024];
    response_r=new float[1024];
    memset(response_l,0,1024*sizeof(float));
    memset(response_r,0,1024*sizeof(float));

    mWav.openDevice();
    RayTracer::Scene scene;
    for(int theta=0;theta<60;++theta)
    {
        for(int phi=-30;phi<30;++phi)
        {
            RayTracer::vector3 dir;
            dir.x = cosf(2.0f*PI/60.0f*theta)*sinf(2.0f*PI/60.0f*phi);
            dir.y = sinf(2.0f*PI/60.0f*theta)*sinf(2.0f*PI/60.0f*phi);
            dir.z = cosf(2.0f*PI/60.0f*phi);
            RayTracer::Ray ray(origin, dir);
            ray.strength=1.0f;
            ray.microseconds=0;
            ray.totalDist = 0.0f;
            ray.active=true;
            rayListTmp.push_back(ray);
        }
    }

    //std::cout<<"Sound position:"<<origin<<" Listener position: "<<listener<<std::endl;
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
                if(d2<0.1f)
                {
                    float Thc = sqrt(0.1f-d2);
                    dist_to_listener = Tca-Thc;
                }
            }
            if(dist_>dist_to_listener)
            {
                rayListTmp[i].totalDist+=(dist_to_listener);
                rayListTmp[i].strength-=dist_to_listener/10.0f;
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
                rayListTmp[i].strength-=dist_/10.0f;
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
    //printf("stage 2\n");
    float* hrtf; 
    hrtf::ir_both ir;
    std::sort(respondList.begin(), respondList.end());
    for(unsigned int i=0;i<(respondList.size()>5?5:respondList.size());++i)
    {
        Matrix4 m1;
        m1.rotateY(rot_z);
        Vector3 newDir;
        newDir.x=respondList[i].direction.z;
        newDir.y=respondList[i].direction.y;
        newDir.z=respondList[i].direction.x;
        newDir=m1*newDir;
        respondList[i].direction.x=newDir.x;
        respondList[i].direction.y=newDir.y;
        respondList[i].direction.z=newDir.z;
        ir = mhrtf.getHRTF(respondList[i].direction);
        hrtf = ir.ir_l;
        for(int j=0;j<128;++j)
        {
            if(respondList[i].time+j<1024)response_l[respondList[i].time+j]+=(hrtf[j]*respondList[i].strength);
        }
        hrtf = ir.ir_r;
        for(int j=0;j<128;++j)
        {
            if(respondList[i].time+j<1024)response_r[respondList[i].time+j]+=(hrtf[j]*respondList[i].strength);
        }
        //printf("%d/%d\n",i,respondList.size());
    }

    
    int divide=1;
    int kernelSize=1024;
    int dataSize = 71296/divide;


    buffer = new short[(dataSize+kernelSize)*2];
    buffer_old = new short[(dataSize+kernelSize)*2];
    buffer_last = new short [kernelSize*2];
    hrtf::convAudio(buffer_old,buffer_last,music,dataSize,kernelSize,response_l,response_r,true);

    convData *data = (convData*) malloc(sizeof(convData));
    data->dataSize=dataSize;
    data->kernelSize=kernelSize;
    data->response_l=response_l;
    data->response_r=response_r;
    
    HANDLE convHandle,playwavHandle;
    short* tmpbuf;


    playwavHandle = CreateThread(NULL,0,playwavThread,(LPVOID)&dataSize,0,NULL);
    
    for(int i=1;i<divide;++i)
    {
        data->buffer=buffer;
        data->buffer_last = buffer_last;
        data->music=&music[i*dataSize*2];
        convHandle = CreateThread(NULL,0,convThread,(LPVOID)data,0,NULL);
        //printf("%d CreateThread\n",i);
        
        
        
        WaitForSingleObject(playwavHandle,INFINITE);

        {
            //printf("Swap buffer\n");
            boost::lock_guard<boost::mutex> m_csLock(mutex);
            tmpbuf = buffer_old;
            buffer_old = buffer;
            buffer = tmpbuf;
        }
        
        playwavHandle = CreateThread(NULL,0,playwavThread,(LPVOID)&dataSize,0,NULL);
        //printf("-----------\n");
        //system("pause");
    }
    WaitForSingleObject(playwavHandle,INFINITE);
    free(buffer_old);
    buffer_old = NULL;
    free(buffer);
    buffer = NULL;
    free(music);
    mWav.unprepWave();
    finish = clock();
    mWav.closeDevice();
    double duration = (double)(finish - start) / CLOCKS_PER_SEC;
    //printf( "%f seconds\n", duration );

    /*std::ofstream out("data/response.txt");
    out<<"a =[";
    for(int i=0;i<1024;++i) 
    {
        out<<response_l[i]<<" "<<response_r[i]<<"; ";
    }
    out<<"]"<<std::endl;
    */

}
std::vector<RayTracer::Ray> rayList;

void keyinput(unsigned char key, int x, int y)
{
    Matrix4 m1,m2;
    m1.rotateY(rot_z);
    Vector3 vx=Vector3(0.1,0,0);
    vx=m1*vx;
    m2.rotateY(rot_z+90);
    Vector3 vz=Vector3(0.1,0,0);
    vz=m2*vz;
    HANDLE waveHandle;
    switch(key)
    {
    case 'w':
        listener.x+=vx.x;
        listener.z+=vx.z;
        std::cout<<listener<<std::endl;
        break;
    case 's':
        listener.x-=vx.x;
        listener.z-=vx.z;
        std::cout<<listener<<std::endl;
        break;
    case 'a':
        listener.x+=vz.x;
        listener.z+=vz.z;
        std::cout<<listener<<std::endl;
        break;
    case 'd':
        listener.x-=vz.x;
        listener.z-=vz.z;
        std::cout<<listener<<std::endl;
        break;
    case 'j':
        rot_z+=1.5f;
        break;
    case 'k':
        rot_z-=1.5f;
        break;
    case 'p':
        //waveHandle = CreateThread(NULL,0,waveThread,(LPVOID)NULL,0,NULL);
        break;
    case 27:
        exit(0);
        break;
    }
}


void init(void)
{
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);

    glShadeModel (GL_SMOOTH);

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

    scene.primList.clear();
    
    scene.loadObj("Res/Scene.obj");


    

}
//Compute ray tracing by per frame ray simulation 
void display(void)
{
    glClearColor (0.2f, 0.2f, 0.2f, 0.0f);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
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
    glVertex3f(1,0,0);
    glEnd();
    glColor3f(0.7, 0.5, 0.5);
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
        rayList[i].strength -= 0.00005f;
        if(rayList[i].strength<=0.0f) rayList[i].active=false;
        
    }
    if(active_cnt==0)
    {
        init();
        std::cout<<"New Wave"<<std::endl;
    }
    
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    scene.render();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);


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
    rot_x=-(y-200.f)/200.0f*180.0f/16.0f;
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
            if (recv_cnt == 26)
            {
                for (int i = 0; i < 26; i += 2)
                {
                    imu_result[i / 2] = (recv_data[i] << 8 | recv_data[i + 1]);
                    if (imu_result[i / 2] >= 32768)
                    {
                        imu_result[i / 2] -= 32768;
                        imu_result[i / 2] = -imu_result[i / 2];
                    }
                }
                recv_cnt = 0;

                /*ax = imu_result[0] / 10.0f;
                ay = imu_result[1] / 10.0f;
                az = imu_result[2] / 10.0f;

                gx = imu_result[3] / 10.0f;
                gy = imu_result[4] / 10.0f;
                gz = imu_result[5] / 10.0f;

                mx = imu_result[6] / 10.0f;
                my = imu_result[7] / 10.0f;
                mz = imu_result[8] / 10.0f;
                */
                yaw = imu_result[9] / 10.0f;
                pitch = imu_result[10] / 10.0f;
                roll = imu_result[11] / 10.0f;
                /*IMU.IMU_update(gx, gy, gz, ax, ay, az, mx, my, mz);
                if (cB_PC.Checked)
                {
                    yaw = IMU.yaw;
                    pitch = IMU.pitch;
                    roll = IMU.roll;
                }*/


                /*ry = -yaw;
                rz = pitch;
                rx = roll;
                */
                rot_z=-yaw;
                start_flag = false;
                recv_cnt = 0;
                start_match_pos = 0;
            }
        }
    }

}
int main(int argc, char** argv)
{
    std::cout<<"Select Serial Port(e.g COM1):";
    std::string com_port;
    std::cin>>com_port;

    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize (600, 400);
    glutInitWindowPosition (100, 100);
    glutCreateWindow (argv[0]);
    initCalc();
    init ();
    HANDLE waveHandle = CreateThread(NULL,0,waveThread,(LPVOID)NULL,0,NULL);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutPassiveMotionFunc(mouse);
    glutKeyboardFunc(keyinput);
    
    std::cout<<"Serial Port Status"<<serial.open(com_port);
    std::cout<<serial.set_option(115200,0,8,0,0)<<"\n";
    serial.recv_callback(recv_callback);
    glutMainLoop();
    serial.close();
    return 0;
}
