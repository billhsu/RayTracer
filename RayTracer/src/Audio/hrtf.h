// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University

#ifndef HRTF_H
#define HRTF_H
#include <string>
class hrtf
{
public:
    hrtf(void);
    ~hrtf(void);
    void load(std::string dir);
};

#endif