#ifndef __our_gl_h__
#define __our_gl_h__

#include "tgaimage.h"
#include "geometry.h"

extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;


void viewport(int x,int y,int w,int h);
void lookat(Vec3f eye,Vec3f center,Vec3f up);
void projection(float coeff=0);

struct IShader{
    virtual ~IShader();
    virtual Vec4f vertex(int iface,int nthvert)=0;
    virtual bool fragment(Vec3f vec,TGAColor &color)=0;
};

void triangle(Vec4f *pts,IShader &shader,TGAImage &image,TGAImage &zbuffer);

#endif