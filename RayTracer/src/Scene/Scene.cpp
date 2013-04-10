// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University

#include "Scene.h"
#include "Primitive.h"
#include "Ray.h"
#include <iostream>
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
        float min_dist=1000.0f;
        int collisionID=-1;
        for(i=0;i<primList.size();++i)
        {
            float dist_ = dist;
            if(primList[i].intersect(ray, dist_)==HIT) 
            {
                if(min_dist>dist_){
                    min_dist = dist_;
                    collisionID = i;
                }
            }
        }
        if(collisionID==-1)return MISS;
        else {
            dist = min_dist;
            return collisionID;
        }
    }
};