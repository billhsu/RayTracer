// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University
#include <stdio.h>
#include "hrtf.h"


hrtf::hrtf(char* Path)
{
    load(Path);
}
hrtf::~hrtf()
{
    for(unsigned int i=0; i<hrtf_list.size(); ++i)
    {
        free(hrtf_list[i].ir);
    }
}
void hrtf::load(char* Path)
{
    WIN32_FIND_DATA FindData;
    HANDLE hError;
    int FileCount = 0;
    char FilePathName[256];
    char FullPathName[256];
    strcpy(FilePathName, Path);
    strcat(FilePathName, "\\*.*");
    hError = FindFirstFile((LPCSTR)FilePathName, &FindData);
    if (hError == INVALID_HANDLE_VALUE)
    {
        printf("Failed!");
        return;
    }
    while(::FindNextFile(hError, &FindData))
    {
        if (strcmp(FindData.cFileName, ".") == 0 
                || strcmp(FindData.cFileName, "..") == 0 )
        {
            continue;
        }

        wsprintf(FullPathName, "%s\\%s", Path,FindData.cFileName);
        FileCount++;
        //printf("\n%d  %s  ", FileCount, FullPathName);

        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            //printf("<Dir>");
            load(FullPathName);
        }
        else
        {
            read_hrtf(FullPathName);
        }

    }
}

void hrtf::read_hrtf(char* filename)
{
    long fileSize;
    hrtf_data data;
    char drive[16],dir[128],fname[128],ext[16];
    char fname_cpy[128];
    
    data.ir = mWav.readWavFileData(filename, fileSize);
    _splitpath(filename,drive,dir,fname,ext);

    strcpy(fname_cpy, fname);
    char* token = strtok(fname_cpy, "H");
    token = strtok(token, "e");
    data.h = atoi (token);
    token = strtok(NULL, "a");
    data.e = atoi (token);

    hrtf_list.push_back(data);

}
short* hrtf::getHRTF(RayTracer::vector3 direction)
{
    direction=-direction;
    float yaw = atan2(direction.y, direction.x)*180.0f/PI;
    float pitch = atan2(direction.z, 
            sqrt(direction.x*direction.x+direction.y*direction.y))*180.0f/PI;
    float min_dist = 1000.0f;
    int h,e;
    for(int i=0; i<hrtf_list.size(); ++i)
    {
        if(fabs(hrtf_list[i].h-pitch)<min_dist)
        {
            min_dist = fabs(hrtf_list[i].h-pitch);
            h = hrtf_list[i].h;
        }
    }

    min_dist = 1000.0f;
    short* target_ir = NULL;
    for(int i=0; i<hrtf_list.size(); ++i)
    {
        if(hrtf_list[i].h != h) continue;
        if(fabs((hrtf_list[i].e-180)-yaw)<min_dist)
        {
            min_dist = fabs((hrtf_list[i].e-180)-yaw);
            e = hrtf_list[i].e;
            target_ir = hrtf_list[i].ir;
        }
    }

    printf("yaw=%f pitch=%f h=%d e=%d\n",yaw,pitch,h,e);
    return target_ir;


}
