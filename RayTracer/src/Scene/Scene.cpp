// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University

#include "Scene.h"
#include "Primitive.h"
#include "Ray.h"

namespace RayTracer{
    void Scene::render()
    {
        for(unsigned int i=0;i<primList.size();++i)
        {
            primList[i].render();
        }
    }

    int Scene::intersect(Ray& ray, float& dist)
    {
        unsigned int i;
        for(i=0;i<primList.size();++i)
        {
            if(primList[i].intersect(ray, dist)==HIT) break;
        }
        if(i==primList.size())return MISS;
        else return i;
    }
};