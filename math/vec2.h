#pragma once
#ifndef MIFFY_VEC2
#define MIFFY_VEC2
#define _USE_MATH_DEFINES
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>
namespace miffy{
  template <typename T>
	struct vec2{
		T x;
		T y;
		vec2():x(0),y(0){}
		vec2( T _x, T _y):x(_x),y(_y){}
		void set( T _x, T _y){x=_x;y=_y;}
		void glVertex(){}
		void glTexCoord(){}
		vec2<T>& operator=(const vec2<T>& _in){
			x=_in.x;
			y=_in.y;
			return *this;
		}
		vec2 operator*(const T& _in){return vec2(x*_in,y*_in);}
		vec2 operator/(const T& _in){return vec2(x/_in,y/_in); }
		vec2 operator-(const vec2<T>& _in){
			return vec2(x-_in.x,y-_in.y);
		}
		vec2 operator+(const vec2<T>& _in){
			return vec2(x+_in.x,y+_in.y);
		}
		float length(){return sqrt((float)(x*x+y*y));}
		float pow(){return ((float)(x*x+y*y));}
		void Print(const char* _message){}
#ifdef _IOSTREAM_
		friend std::ostream& operator<<<>(std::ostream& _s,const vec2<T>& _in);
#endif
	};
	//非メンバ関数
#ifdef _IOSTREAM_
	template <typename T>
	std::ostream& operator<<<>(std::ostream& _s,const vec2<T>& _in)
	{
		_s<<_in.x<<","<<_in.y;
		return _s;
	}
#endif
	template<> void vec2<float>::glTexCoord(){glTexCoord2f(x,y);}
	template<> void vec2<int>::glTexCoord(){glTexCoord2i(x,y);}
	template<> void vec2<float>::glVertex(){glVertex2f(x,y);}
	template<> void vec2<int>::glVertex(){glVertex2i(x,y);}
	template<> void vec2<int>::Print(const char* _message){
		printf("%s:",_message);
		printf("(%d,%d)\n",x,y);
	}
	template<> void vec2<float>::Print(const char* _message){
		printf("%s:",_message);
		printf("(%f,%g)\n",x,y);
	}
	/*
	#ifdef __FREEGLUT_H__

	void drawVector(float posx,float posy,float size){
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	//矢印の平行移動
	glTranslatef(posx,posy,0.0f);
	//Print("vec2");
	float degree=atan2((float)y,(float)x)* 180.0f / 3.14f;//矢印の回転 M_PIがエラーになる。。！！
	glRotatef(degree,0.0f,0.0f,1.0f);
	glBegin(GL_LINES);
	glVertex2f(-size/2.0f,0.0f);
	glVertex2f(size/2.0f,0.0f);
	glVertex2f(size/2.0f,0.0f);
	glVertex2f(size/6.0f,size/(3.0f*1.73f));//1.73はルート３．30度ぐらいの矢印角度にしようと思った。
	glVertex2f(size/2.0f,0.0f);
	glVertex2f(size/6.0f,-size/(3.0f*1.73f));
	glEnd();

	glPopMatrix();
	}

	#endif*/

}//end of miffy
#endif
