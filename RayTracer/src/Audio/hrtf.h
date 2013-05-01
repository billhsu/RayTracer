// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University

#ifndef HRTF_H
#define HRTF_H
#include <string>
#include <vector>
#include "../common.h"
#include "wav.h"

class hrtf
{
public:
    hrtf(){}
    hrtf(char* Path);
    ~hrtf(void);
    struct ir_both 
    {
        float* ir_r;
        float* ir_l;
    };
    wav mWav;
    void load(char* Path);
    ir_both getHRTF(RayTracer::vector3 direction);
private:
    struct hrtf_data{
        float ir[512];
        int a,e;
    };
    
    std::vector<hrtf_data> hrtf_list_r;
    std::vector<hrtf_data> hrtf_list_l;
    void read_hrtf(char* filename);
};

#endif
