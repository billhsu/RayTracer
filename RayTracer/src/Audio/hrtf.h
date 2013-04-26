// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University

#ifndef HRTF_H
#define HRTF_H
#include <string>
#include <vector>
#include "../common.h"
class hrtf
{
public:
    hrtf(void){}
    htrf(std::string dir);
    ~hrtf(void);
    void load(std::string dir);
    short* getHRTF(RayTracer::vector3 direction);
private:
    struct hrtf_data{
        short* ir;
        int h,e;
    };
    std::vector<hrtf_data> hrtf_list;
    void read_hrtf(std::string filename);
};

#endif
