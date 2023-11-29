#include <vector>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "our_gl.h"
#include "geometry.h"
Model *model=NULL;
const int width=800;
const int height=800;
Vec3f light_dir(1,1,1);
Vec3f       eye(1,1,3);
Vec3f    center(0,0,0);
Vec3f        up(0,1,0);
struct GouraudShader:public IShader{
	mat<2,3,float> varying_uv;
	
	virtual Vec4f vertex(int iface,int nthvert)
	{
		varying_uv.set_col(nthvert,model->uv(iface,nthvert));
		Vec4f gl_Vertex=embed<4,3,float>(model->vert(iface,nthvert));
		return Viewport*Projection*ModelView*gl_Vertex;
	}

	virtual bool fragment(Vec3f bar,TGAColor &color)
	{
		Vec2f uv=varying_uv*bar;
		Vec3f n=proj<3,4,float>((Projection*ModelView).invert_transpose()*embed<4,3,float>(model->normal(uv))).normalize();
		Vec3f l=proj<3,4,float>(Projection*ModelView*embed<4,3,float>(light_dir)).normalize();
		Vec3f v=proj<3,4,float>(Projection*ModelView*embed<4,3,float>(eye)).normalize();
		Vec3f h=((v+l)/(v.norm()+l.norm())).normalize();
		float spec=pow(n*h,model->specular(uv));
		float diff=std::max(0.f,n*l);
		TGAColor amb=TGAColor(5,5,5);
		TGAColor c=model->diffuse(uv);
		color=amb+c*diff+c*.55*spec;
		return false;
	}
};

int main(int argc, char** argv)
{
	model=new Model("obj/african_head.obj");
	lookat(eye,center,up);
	viewport(width/8,height/8,width*3/4,height*3/4);
	projection(-1.f/(eye-center).norm());
	light_dir.normalize();
	TGAImage image(width,height,TGAImage::RGB);
	TGAImage zbuffer(width,height,TGAImage::GRAYSCALE);

    GouraudShader shader;
	for (int i=0;i<model->nfaces();i++)
	{
		Vec4f screen_coords[3];
		for (int j=0;j<3;j++)
		{
			screen_coords[j]=shader.vertex(i,j);
		}
		triangle(screen_coords,shader,image,zbuffer);
	}
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	zbuffer.flip_vertically();
	image.write_tga_file("output.tga");
	zbuffer.write_tga_file("zbuffer.tga");
	delete model;
	return 0;
}