// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University

//Class for primitive scene object
#ifndef PRIMITIVE_H
#define PRIMITIVE_H
namespace RayTracer{

class Ray;

class Primitive
{
public:
    Primitive(void);
    ~Primitive(void);

    bool intersect(Ray & ray, float const distance);
};


};
#endif