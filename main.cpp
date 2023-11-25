#include "tgaimage.h"
#include "model.h"
#include <vector>
#include <limits>
#include <cmath>
#include <cstdlib>
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255,   0,   0, 255);
const TGAColor green = TGAColor(0,   255, 0,   255);
Vec3f light_dir(0,0,-1);
Model *model=NULL;
TGAImage *texture=NULL;
const int width=1600;
const int height=1600;
const float eps=1e-3;
void line(int x0,int y0,int x1,int y1,TGAImage &image,TGAColor color)
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

/*void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) { 
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

Vec3f barycentric(Vec3f AB,Vec3f AC,Vec3f A,Vec2f pix)
{
	Vec3f P=Vec3f(pix.x,pix.y,0.0);
	Vec3f AP=A-P;
	Vec3f u=cross(Vec3f(AB.x,AC.x,AP.x),Vec3f(AB.y,AC.y,AP.y));
	if(std::abs(u[2])>1e-2) return Vec3f(1.f-(u.x+u.y)/u.z,u.x/u.z,u.y/u.z);
	return Vec3f(-1,1,1); 
}
//void triangle(Vec3f *pts,float *zbuffer,TGAImage &image,float intensity,TGAColor CA,TGAColor CB,TGAColor CC)
void triangle(Vec3f *pts,float *zbuffer,TGAImage &image,float intensity,Vec3f *texture_coods)
{ 
	float l=std::numeric_limits<float>::max(),r=-std::numeric_limits<float>::max();
	float b=std::numeric_limits<float>::max(),t=-std::numeric_limits<float>::max();
	for (int i=0;i<3;i++)
	{
		l=std::min(l,pts[i].x);
		r=std::max(r,pts[i].x);
		b=std::min(b,pts[i].y);
		t=std::max(t,pts[i].y);
	}
	Vec3f AB=pts[1]-pts[0],AC=pts[2]-pts[0],A=pts[0];
	for (float i=l;i<=r;i++)
	{
		for (float j=b;j<=t;j++)
		{ 
			Vec3f x=barycentric(AB,AC,A,Vec2f(i,j));
			if(x.x<0||x.y<0||x.z<0) continue;
			int depth=0;
			Vec3f uv=texture_coods[0]*x.x+texture_coods[1]*x.y+texture_coods[2]*x.z;
			TGAColor color=texture->get(uv.x*1024,1024-uv.y*1024);
			for (int k=0;k<3;k++) depth+=pts[k][2]*x[k];
			if(zbuffer[int(i+j*width)]<depth)
			{
				zbuffer[int(i+j*width)]=depth;
				image.set(i,j,color*intensity);
			}
		}
	}
}

Vec3f world2screen(Vec3f v)
{
	int x=(int)((v.x+1.)*width/2.+.5);
	int y=(int)((v.y+1.)*height/2.+.5);
	return Vec3f((float)x,(float)y,v.z);
}

int main(int argc, char** argv)
{
	TGAImage image(width, height, TGAImage::RGB);

	model=new Model("obj/african_head.obj");
	texture=new TGAImage();
	texture->read_tga_file("texture/african_head_diffuse.tga");
	float *zbuffer=new float[width*height];
	for (int i=0;i<width*height;i++) zbuffer[i]=-std::numeric_limits<float>::max();

	for (int i=0;i<model->nfaces();i++)
	{
		std::vector<int> face=model->face(i);
		Vec3f screen_coords[3];
		Vec3f world_coords[3];
		int texindex[3];
		int cntv=0,cntt=0;
		for (int j=0;j<6;j++)
		{
			if((j+1)%2==0)
			{
				texindex[cntt++]=face[j];
			}
			else
			{
				Vec3f v = model->vert(face[j]); 
				screen_coords[cntv++]=world2screen(v);
				world_coords[cntv-1]=v;
			}
		}
		Vec3f n = cross((world_coords[2]-world_coords[0]),(world_coords[1]-world_coords[0])); 
		n.normalize();
		float intensity = n*light_dir; 
		Vec3f texture_coods[3];
		for (int i=0;i<3;i++)
		{
			texture_coods[i]=model->uv(texindex[i]);
		}
		if (intensity>0)
		{
			triangle(screen_coords ,zbuffer,image,intensity,texture_coods);
		}
	}	
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}