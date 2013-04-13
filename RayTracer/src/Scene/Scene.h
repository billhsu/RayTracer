// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University

#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>
namespace RayTracer{
    class Primitive;
    class Ray;
    class Scene
    {
    public:
        bool loadObj(std::string filename);
        void render();
        int intersect(Ray& ray, float& dist);
        std::vector<Primitive> primList;
    };

};

#endif