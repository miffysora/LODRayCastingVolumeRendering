//シェーダ言語のvec3みたいなものを目指す。
#ifndef MIFFY_VEC3
#define MIFFY_VEC3
#include <iostream>
using namespace std;
namespace miffy{
  template <typename T>
	class vec3{//計算用の頂点構造体

	public:
		vec3():x(0),y(0),z(0){}
		vec3(T _x,T _y,T _z):x(_x),y(_y),z(_z){}
		//演算子
		vec3<T>& operator=(const vec3<T>& _in){
			x=_in.x;
			y=_in.y;
			z=_in.z;
			return *this;
		}
		vec3<T> operator*(const T& _in)const{
			return vec3<T>(x*_in,y*_in,z*_in);
		}
		vec3<T> operator-(const vec3<T>& _in){
			return vec3<T>(x-_in.x,y-_in.y,z-_in.z);
		}
		vec3<T> operator+(const vec3<T>& _in){
			return vec3<T>(x+_in.x,y+_in.y,z+_in.z);
		}
		vec3<T> operator*(const vec3<T>& _in){
			return vec3<T>(x*_in.x,y*_in.y,z*_in.z);
		}
		bool operator==(const T& _in){
			if(x!=_in){return false;}
			if(y!=_in){return false;}
			if(z!=_in){return false;}
			return true;
		}
		bool operator==(const vec3<T>& _in){
			if(x!=_in.x){return false;}
			if(y!=_in.y){return false;}
			if(z!=_in.z){return false;}
			return true;
		}
		vec3<T> operator+=(const vec3<T>& _in){
			x+=_in.x;
			y+=_in.y;
			z+=_in.z;
			return *this;
		}
		vec3<T> operator-=(const T& _in){
			x-=_in;
			y-=_in;
			z-=_in;
			return *this;
		}
		vec3<T> operator*=(const vec3<T>& _in){
			x*=_in.x;
			y*=_in.y;
			z*=_in.z;
			return *this;
		}
		vec3<T> operator*=(const T _in){
			x*=_in;
			y*=_in;
			z*=_in;
			return *this;
		}
		vec3<T> operator+(const T _in){
			return vec3<T>(x+_in,y+_in,z+_in);
		}
		vec3<T> operator-(const T& _in){
			return vec3<T>(x-_in,y-_in,z-_in);
		}
		vec3<T> operator-(){
			return vec3<T>(-x,-y,-z);
		}
		vec3<T> operator/(const vec3<T>& _in){
			return vec3<T>(x/_in.x,y/_in.y,z/_in.z);
		}
		vec3<T> operator/(const T& _in){
			return vec3<T>(x/_in,y/_in,z/_in);
		}
		bool    operator>(const T _in){//1個でも条件に当てはまっていたらtrueにする
			if(x>_in){return true;}
			if(y>_in){return true;}
			if(z>_in){return true;}
			return false;
		}
		bool    operator<(const T _in){
			if(x<_in){return true;}
			if(y<_in){return true;}
			if(z<_in){return true;}
			return false;
		}
		bool zero(){
			if(x!=0.0f){return false;}
			if(y!=0.0f){return false;}
			if(z!=0.0f){return false;}
			return true;
		}
		vec3<T> cross(const vec3<T>& _in){//外積
			vec3<T> ret(y*_in.z-z*_in.y, z*_in.x-x*_in.z, x*_in.y-y*_in.x);
			//if(ret.length()==0.0){ret.set(0.0,1.0,0.0);}//これはやっちゃダメ！　takasaoレンダリングで変になるから。
			return (ret);
		}
		void set(const vec3<T>& _in){x=_in.x;y=_in.y;z=_in.z;}

		void set(T _x,T _y,T _z){x=_x;y=_y;z=_z;}

		T innerProduct(const vec3<T>& _in){//内積
			return (x*_in.x+y*_in.y+z*_in.z);
		}

		vec3<T> uni(){//単位ベクトルを返す
			T length=sqrt(x*x+y*y+z*z);
			if(length!=0.0){
				return vec3<T>(x/length,y/length,z/length);
			}else{
				return vec3<T>(0.0,0.0,0.0);
			}
		}
		void normalize(){
			T length=sqrt(x*x+y*y+z*z);
			if(length!=0.0){
				x/=length;
				y/=length;
				z/=length;
			}
		}
		vec3<int> toInt(){
			return vec3<int>((int)x,(int)y,(int)z);
		}
		vec3<float> toFloat(){
			return vec3<float>((float)x,(float)y,(float)z);
		}
		T length()const{return (T)sqrt((double)(x*x+y*y+z*z));}
		//vec3が３次元配列のインデックスとして使用されている時のための関数
		unsigned int index(unsigned int _xnum,unsigned int _ynum){return (z*_ynum+y)*_xnum+x;}
		//vec3が３次元配列のインデックスとして使用されている時で正方だった場合の関数
		unsigned int index(unsigned int _size){return (z*_size+y)*_size+x;}

#ifdef  __FREEGLUT_H__
		void glVertex(){}
		void glNormal(){}
		void glTranslate(){}
		void glRasterPos(){}

#endif
#ifdef _IOSTREAM_
		void print()const{std::cout<<x<<","<<y<<","<<z<<std::endl;}
		void print(const string& _message)const{std::cout<<_message<<":"<<x<<","<<y<<","<<z<<std::endl;}
		friend ostream& operator<<<>(std::ostream& _s,const vec3<T>& _in);

#endif
		//このメンバたちはどんな値になってもいいから。
		T x;
		T y;
		T z;
	};//end of class
	//非メンバ関数
	template <typename T>
	ostream& operator<<<>(std::ostream& _s,const vec3<T>& _in)
	{
		_s<<_in.x<<","<<_in.y<<","<<_in.z;
		return _s;
	}

#ifdef  __FREEGLUT_H__
	template<> void vec3<int>::glVertex(){glVertex3i(x,y,z);}
	template<> void vec3<float>::glVertex(){glVertex3f(x,y,z);}
	template<> void vec3<double>::glVertex(){glVertex3d(x,y,z);}

	template<> void vec3<float>::glNormal(){glNormal3f(x,y,z);}
	template<> void vec3<double>::glNormal(){glNormal3d(x,y,z);}

	template<> void vec3<float>::glTranslate(){glTranslatef(x,y,z);}
	template<> void vec3<double>::glTranslate(){glTranslated(x,y,z);}
	template<> void vec3<float>::glRasterPos(){glRasterPos3f(x,y,z);}
	template<> void vec3<double>::glRasterPos(){glRasterPos3f(x,y,z);}
#endif

}//namespace
#endif
