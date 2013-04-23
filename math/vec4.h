#pragma once
#ifndef MIFFY_VEC4
#define MIFFY_VEC4
#include <cmath>

namespace miffy{
  
	template <typename T>
	struct vec4{//計算用の頂点構造体
		T x;
		T y;
		T z;
		T w;
		vec4():x(0.0),y(0.0),z(0.0),w((T)1.0){}
		vec4(T _x,T _y,T _z):x(_x),y(_y),z(_z),w((T)1.0){}
		vec4(T _x,T _y,T _z,T _w):x(_x),y(_y),z(_z),w(_w){}
		
		void set(T _x,T _y,T _z){x=_x;y=_y;z=_z;w=(T)1.0;}
		void print(const char* _message){printf("%s\n",_message);printf("(%f,%f,%f,%f)\n",x,y,z,w);}
		vec4<T> operator=(const vec4<T>& _in){
			x=_in.x;
			y=_in.y;
			z=_in.z;
			w=_in.w;
			return *this;
		}
		vec4<T> operator=(const float _in){
			x=_in;
			y=_in;
			z=_in;
			w=1.0;
			return *this;
		}
		vec4<T> operator+(const vec4<T>& _in){
			return vec4<T>(x+_in.x,y+_in.y,z+_in.z,w+_in.w);
		}
		vec4<T> operator+(const T  _scalar){
			return vec4<T>(x+_scalar,y+_scalar,z+_scalar);
		}

		vec4<T> operator-(const vec4<T>& _in){
			return vec4<T>(x-_in.x,y-_in.y,z-_in.z);
		}
		vec4<T> operator/(const T _scalar){
			return vec4<T>(x/_scalar,y/_scalar,z/_scalar);
		}
		T operator*(const vec4<T>& _in){//内積
			return (x*_in.x+y*_in.y+z*_in.z);
		}
		T length(){
			T length=(T)0.0;
			length=sqrt(z*z+y*y+x*x);
			return length;
		}
		void normalize(){
			T length=sqrt(x*x+y*y+z*z);
			x/=length;
			y/=length;
			z/=length;
		}
		template <typename T>
		vec4<T> cross(const vec4<T>& _b){
			return vec4<T>(y*_b.z-z*_b.y,z*_b.x-z*_b.z,x*_b.y-y*_b.x);
		}
#ifdef  __FREEGLUT_H__
		void glVertex(){
			glVertex3f(x,y,z);
		}
#endif
#ifdef MIFFY_VEC3
		vec4(vec3<T> _in,T _w):x(_in.x),y(_in.y),z(_in.z),w(_w){}
		void set(vec3<T> _in){x=_in.x;y=_in.y;z=_in.z;w=(T)1.0;}
		vec3<T> toVec3(){return vec3<T>(x,y,z);}
#endif
	};
}
#endif
