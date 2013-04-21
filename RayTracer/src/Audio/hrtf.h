// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University

#ifndef HRTF_H
#define HRTF_H
#include <string>
#include <vector>
class hrtf
{
public:
    hrtf(void);
    ~hrtf(void);
    void load(std::string dir);

private:
    struct hrtf_data{
        float ir[128];
    };
    std::vector<hrtf_data> a;
};

#endif