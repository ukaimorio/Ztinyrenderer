#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "model.h"

Model::Model(const char *filename):verts_(), faces_()
{
    std::ifstream in;
    in.open(filename,std::ifstream::in);
    if(in.fail()) return;
    std::string line;
    while(!in.eof())
    {
        std::getline(in,line);
        std::istringstream iss(line.c_str());
        char trash;
        if(!line.compare(0,2,"v "))
        {
            iss>>trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss>>v[i];
            verts_.push_back(v);
        }
        else if(!line.compare(0,2,"f "))
        {
            std::vector<int> f;
            int itrash,index,texindex;
            iss>>trash;
            while(iss>>index>>trash>>texindex>>trash>>itrash)
            {
                index--;
                texindex--;
                f.push_back(index);
                f.push_back(texindex);
            }
            faces_.push_back(f);
        }
        else if(!line.compare(0,4,"vt  "))
        {
            Vec3f v;
            iss>>trash>>trash;
            for (int i=0;i<3;i++) iss>>v[i];
            texture_.push_back(v);
        }
    } 
    std::cerr<<"# v# "<<verts_.size()<<" f# "<<faces_.size()<<" vt# "<<texture_.size()<<std::endl;
}

Model::~Model(){}

int Model::nverts(){return (int)verts_.size();}

int Model::nfaces(){return (int)faces_.size();}

int Model::ntexture(){return (int)texture_.size();}

Vec3f Model::vert(int i){return verts_[i];}

std::vector<int> Model::face(int index) {return faces_[index];}

Vec3f Model::uv(int i){return texture_[i];}

