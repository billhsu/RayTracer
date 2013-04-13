// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University

#include "Scene.h"
#include "Primitive.h"
#include "Ray.h"
#include <iostream>
#include <sstream>
#include <fstream>

namespace RayTracer{

    bool Scene::loadObj(std::string filename)
    {
        std::ifstream in;
        in.open(filename, std::ifstream::in);
        char line[256];

        std::vector<RayTracer::vector3> vList;
        while(in.getline(line,256))
        {
            std::istringstream stream;
            char mark;
            float v1,v2,v3;
            int i1,i2,i3;
            stream.str(line);
            stream>>mark;
            if(mark=='v') 
            {
                stream>>v1>>v2>>v3;
                vList.push_back(RayTracer::vector3(v1,v2,v3));
            }
            else if(mark=='f') 
            {
                stream>>i1>>i2>>i3;
                Primitive p = Primitive(vList[i1-1],vList[i2-1],vList[i3-1]);
                primList.push_back(p);
            }
        }
        return true;
    }
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