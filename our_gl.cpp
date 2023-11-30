#include <cmath>
#include <limits>
#include <cstdlib>
#include "our_gl.h"

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

IShader::~IShader(){};

void viewport(int x,int y,int w,int h)
{
	Viewport=Matrix::identity();
	Viewport[0][3]=x+w/2.f;
    Viewport[1][3]=y+h/2.f;
    Viewport[2][3]=255.f/2.f;
    Viewport[0][0]=w/2.f;
    Viewport[1][1]=h/2.f;
    Viewport[2][2]=255.f/2.f;
}

void projection(float coeff)
{
	Projection=Matrix::identity();
	Projection[3][2]=coeff;
}

void lookat(Vec3f eye,Vec3f center,Vec3f up)
{
	Vec3f z=(eye-center).normalize();
	Vec3f x=cross(up,z).normalize();
	Vec3f y=cross(z,x).normalize();
	ModelView=Matrix::identity();
	for (int i=0;i<3;i++)
	{
		ModelView[0][i]=x[i];
		ModelView[1][i]=y[i];
		ModelView[2][i]=z[i];
		ModelView[i][3]=-center[i];
	}
}

Vec3f barycentric(Vec2f A,Vec2f B,Vec2f C,Vec2f pix)
{
    Vec2f AB=B-A,AC=C-A,PA=A-pix;
	Vec3f u=cross(Vec3f(AB.x,AC.x,PA.x),Vec3f(AB.y,AC.y,PA.y));
	if(std::abs(u[2])>1e-2) return Vec3f(1.f-(u.x+u.y)/u.z,u.x/u.z,u.y/u.z);
	return Vec3f(-1,1,1); 
}

void triangle(Vec4f *pts, IShader &shader, TGAImage &image, float *zbuffer)
{ 
	float l=std::numeric_limits<float>::max(),r=-std::numeric_limits<float>::max();
	float b=std::numeric_limits<float>::max(),t=-std::numeric_limits<float>::max();
	for (int i=0;i<3;i++)
	{
        for (int j=0;j<2;j++)	
        {
            if(j==0)l=std::min(l,pts[i][j]/pts[i][3]),r=std::max(r,pts[i][j]/pts[i][3]);
            else b=std::min(b,pts[i][j]/pts[i][3]),t=std::max(t,pts[i][j]/pts[i][3]);
        }
	}
    Vec2i P;
    TGAColor color;
    for (P.x=l;P.x<=r;P.x++)
	{
		for (P.y=b;P.y<=t;P.y++)
		{ 
			Vec2f pix(P.x*1.0,P.y*1.0);
            Vec3f c=barycentric(proj<2,4,float>(pts[0]/pts[0][3]),proj<2,4,float>(pts[1]/pts[1][3]),proj<2,4,float>(pts[2]/pts[2][3]),proj<2,2,float>(pix));
            float z=pts[0][2]*c.x+pts[1][2]*c.y+pts[2][2]*c.z;
            float w=pts[0][3]*c.x+pts[1][3]*c.y+pts[2][3]*c.z;
            int frag_depth=z/w;
            if(c.x<0||c.y<0||c.z<0||zbuffer[P.x+P.y*image.get_width()]>frag_depth) continue;
			bool discard=shader.fragment(c,color);
			if(!discard)
			{
				zbuffer[P.x+P.y*image.get_width()] = frag_depth;
				image.set(P.x,P.y,color);
			}
		}
	}
}

/*void line(int x0,int y0,int x1,int y1,TGAImage &image,TGAColor color)
{
	bool steep=false;
	if(std::abs(x0-x1)<std::abs(y0-y1))
	{
		std::swap(x0,y0);
		std::swap(x1,y1);
		steep=true;
	}
	if(x0>x1) std::swap(x0,x1),std::swap(y0,y1);
	int dx=x1-x0,dy=y1-y0;
	int derror=std::abs(dy)*2;
	int error2=0;
	int y=y0;
	for (int x=x0;x<=x1;x++)
	{
		if(steep) image.set(y,x,color);
		else image.set(x,y,color);
		error2+=derror;
		if(error2>dx)
		{
			y+=(y1>y0)?1:-1;
			error2-=2*dx;
		}
	}
}

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(p0.x-p1.x)<std::abs(p0.y-p1.y)) {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        steep = true;
    }
    if (p0.x>p1.x) {
        std::swap(p0, p1);
    }

    for (int x=p0.x; x<=p1.x; x++) {
        float t = (x-p0.x)/(float)(p1.x-p0.x);
        int y = p0.y*(1.-t) + p1.y*t;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) { 
	if (t0.y==t1.y && t0.y==t2.y) return;
	if(t0.y>t1.y) std::swap(t0,t1);
	if(t0.y>t2.y) std::swap(t0,t2);
	if(t1.y>t2.y) std::swap(t1,t2);
	int total_hight=t2.y-t0.y;
	for (int i=0;i<total_hight;i++)
	{
		bool half=i>(t1.y-t0.y)||(t1.y==t0.y);
		float segment_hight=half?(t2.y-t1.y):(t1.y-t0.y);
		float dk1=(float)i/total_hight;
		float dk2=half?(float)(i+t0.y-t1.y)/segment_hight:(float)i/segment_hight;
		Vec2i A=t0+(t2-t0)*dk1;
		Vec2i B=half?t1+(t2-t1)*dk2:t0+(t1-t0)*dk2;
		if(A.x>B.x) std::swap(A,B);
		for (int j=A.x;j<=B.x;j++)
		{
			image.set(j,t0.y+i,color);
		}
	}
}*/