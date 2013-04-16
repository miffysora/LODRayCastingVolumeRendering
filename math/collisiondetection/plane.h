#ifndef MIFFY_PLANE
#define MIFFY_PLANE

#ifndef MIFFY_VEC3
#include "../vec3.h"
#endif
namespace miffy{
  template <typename T>
	class plane  
	{

	public:

		vec3<T> normal,point;
		float distance;
		//miffy added
		vec3<T> pointA,pointB,pointC,pointD;


		plane( vec3<T> &_v1,  vec3<T> &_v2,  vec3<T> &_v3){set3Points(_v1,_v2,_v3);}
		plane(void){}
		~plane(){}

		void set3Points( vec3<T> &_v1,  vec3<T> &_v2,  vec3<T> &_v3){
			pointA=_v1;
			pointB=_v2;
			pointC=_v3;
			vec3<T> aux1, aux2;

			aux1 = _v1 - _v2;
			aux2 = _v3 - _v2;

			pointD=_v3+aux1;
	

			normal =aux2.cross(aux1);// aux2 * aux1;

			normal.normalize();//これが違う。
			point=_v2;
			distance = -(normal.innerProduct(point));
		}
		void setNormalAndPoint(vec3<T> &_normal, vec3<T> &_point){
			normal.copy(_normal);
			normal.normalize();
			distance = -(normal.innerProduct(_point));
		}
		void setCoefficients(T _a, T _b, T _c, T _d){
			// set the normal vector
			normal.set(_a,_b,_c);
			//compute the lenght of the vector
			float l = normal.length();
			// normalize the vector
			normal.set(_a/l,_b/l,_c/l);
			// and divide d by th length as well
			distance = _d/l;
		}

		//void getNormal(plane &_plane);//deprecated
		float getDistance(vec3<T> &_p) {//ベクトルと平面の距離
			float result = distance + normal.innerProduct(_p);
			return (result);
		}
		void print(){
			printf("Plane(");normal.print();printf("# %f)",distance);
		}
		void renderPlaneQUADS(T _r,T _g,T _b,T _a){
			glColor4f(_r,_g,_b,_a);
			glBegin(GL_QUADS);
			pointA.glVertex();
			pointB.glVertex();
			pointC.glVertex();
			pointD.glVertex();
			glEnd();
		}

	};
}
#endif
