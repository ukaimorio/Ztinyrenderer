#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector> 
#include "geometry.h"
#include "tgaimage.h"

class Model{
    private:
    std::vector<Vec3f> verts_;
    std::vector<std::vector<Vec3i>> faces_;
    std::vector<Vec3f> norms_;
    std::vector<Vec2f> uv_;
    TGAImage diffusemap_,normalmap_,specularmap_,glow_;
    void load_texture(std::string filename,const char *suffix,TGAImage &img);
    public:
    Model(const char *nameflie);
    ~Model();
    int nverts();
    int nfaces();
    Vec3f normal(int iface,int nthvert);
    Vec3f normal(Vec2f uvf);
    Vec3f vert(int iface,int nthver);
    Vec3f vert(int i);
    Vec2f uv(int iface,int nthver);
    TGAColor diffuse(Vec2f uvf);
    float specular(Vec2f uvf);
    TGAColor glow(Vec2f uvf); 
};
#endif //__MODEL_H__ 