#include <vector>
#include <iostream>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "our_gl.h"
#include "geometry.h"
Model *model=NULL;
float *shadowbuffer=NULL;
const int width=800;
const int height=800;
Vec3f light_dir(1,1,1);
Vec3f       eye(1,1,0);
Vec3f    center(0,0,0);
Vec3f        up(0,1,0);
struct Shader:public IShader{
	mat<4,4,float> uniform_M;
	mat<4,4,float> uniform_MIT;
	mat<4,4,float> uniform_Mshadow;
	mat<2,3,float> varying_uv;
	mat<3,3,float> varying_tri;

	Shader(Matrix M,Matrix MIT,Matrix Msh):uniform_M(M),uniform_MIT(MIT),uniform_Mshadow(Msh),varying_uv(),varying_tri(){}

	virtual Vec4f vertex(int iface,int nthvert)
	{
		varying_uv.set_col(nthvert,model->uv(iface,nthvert));
		Vec4f gl_Vertex=Viewport*Projection*ModelView*embed<4,3,float>(model->vert(iface,nthvert));
		varying_tri.set_col(nthvert,proj<3>(gl_Vertex/gl_Vertex[3]));
		return gl_Vertex;
	}

	virtual bool fragment(Vec3f bar,TGAColor &color)
	{
		Vec4f shadow_coods=uniform_Mshadow*embed<4,3,float>(varying_tri*bar);
		shadow_coods=shadow_coods/shadow_coods[3];
		int idx=int(shadow_coods[0])+int(shadow_coods[1])*width;
		float shadow_depth=.3+.7*(shadowbuffer[idx]<shadow_coods[2]);
		Vec2f uv=varying_uv*bar;
		Vec3f n=proj<3,4,float>(uniform_MIT*embed<4,3,float>(model->normal(uv))).normalize();
		Vec3f l=proj<3,4,float>(uniform_M*embed<4,3,float>(light_dir)).normalize();
		Vec3f v=proj<3,4,float>(uniform_M*embed<4,3,float>(eye)).normalize();
		Vec3f h=((v+l)/(v.norm()+l.norm())).normalize();
		float spec=pow(n*h,model->specular(uv)+1000);
		float diff=std::max(0.f,n*l);
		TGAColor amb=TGAColor(20,20,20);
		TGAColor c=model->diffuse(uv);
		color=amb+c*0.5*diff*shadow_depth+c*0.6*spec*shadow_depth;
		return false;
	}
};

struct DepthShader:public IShader{
	mat<3,3,float> varing_tri;
	DepthShader():varing_tri(){}
	virtual Vec4f vertex(int iface,int nthvert)
	{
		Vec4f gl_Vertex=embed<4,3,float>(model->vert(iface,nthvert));
		gl_Vertex=Viewport*Projection*ModelView*gl_Vertex;
		varing_tri.set_col(nthvert,proj<3>(gl_Vertex/gl_Vertex[3]));
		return gl_Vertex;
	}
	virtual bool fragment(Vec3f bar,TGAColor &color)
	{
		Vec3f p=varing_tri*bar;
		color=TGAColor(255,255,255)*(p.z/depth*10);
		return false;
	}
};

int main(int argc, char** argv)
{
	model=new Model("obj/diablo3_pose.obj");
	float *zbuffer=new float[width*height];
	shadowbuffer=new float[width*height];
	for (int i=0;i<width*height;i++) zbuffer[i]=shadowbuffer[i]=-std::numeric_limits<float>::max();
	light_dir.normalize();

	{
		TGAImage depth(width,height,TGAImage::RGB);
		lookat(light_dir,center,up);
		viewport(width/8,height/8,width*3/4,height*3/4);
		projection(0);
		DepthShader depthshader;
		Vec4f screen_coods[3];
		for (int i=0;i<model->nfaces();i++)
		{
			for (int j=0;j<3;j++)
			{
				screen_coods[j]=depthshader.vertex(i,j);
			}
			triangle(screen_coods,depthshader,depth,shadowbuffer);
		}
		depth.flip_vertically();
		depth.write_tga_file("depth.tga");
	}

	Matrix M=Viewport*Projection*ModelView;

	{
		TGAImage frame(width,height,TGAImage::RGB);
		lookat(eye,center,up);
		viewport(width/8,height/8,width*3/4,height*3/4);
		projection(-1.f/(eye-center).norm());
        //逆矩阵//逆矩阵的转置
		Shader shader(ModelView,(Projection*ModelView).invert_transpose(),M*(Viewport*Projection*ModelView).invert());
		Vec4f screen_coords[3];
		for (int i=0;i<model->nfaces();i++)
		{
			for (int j=0;j<3;j++)
			{
				screen_coords[j]=shader.vertex(i,j);
			}
			triangle(screen_coords,shader,frame,zbuffer);
		}
		frame.flip_vertically();
        frame.write_tga_file("framebuffer.tga");
	}
	delete model;
	return 0;
}