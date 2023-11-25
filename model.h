#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector> 
#include "geometry.h"

class Model{
    private:
    std::vector<Vec3f> verts_;
    std::vector<std::vector<int>> faces_;
    std::vector<Vec3f> texture_;
    public:
    Model(const char *nameflie);
    ~Model();
    int nverts();
    int nfaces();
    int ntexture();
    Vec3f vert(int i);
    std::vector<int> face(int index);
    Vec3f uv(int i);
};
#endif //__MODEL_H__ 