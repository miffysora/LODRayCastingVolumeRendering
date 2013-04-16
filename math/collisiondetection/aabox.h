#ifndef MIFFY_AABOX
#define MIFFY_AABOX
#ifndef MIFFY_VEC3
#include "../vec3.h"
#endif
namespace miffy{
template <typename T>
class aabox{
public:
  vec3<T> corner;
	float x,y,z;
	aabox(const vec3<T>& _corner,T _x,T _y,T _z){
		setBox(_corner,_x,_y,_z);
	}
	void setBox(const vec3<T>& _corner,  T _x, T _y, T _z) {
		corner=_corner;//cornerにcornerをコピーする。
		if (_x < (T)0.0) {
			_x = -_x;
			corner.x -= _x;
		}
		if (_y < (T)0.0) {
			_y = -y;
			corner.y -=_y;
		}
		if (_z <(T)0.0) {
			_z = -_z;
			corner.z -= _z;
		}
		x = _x;
		y = _y;
		z = _z;
	}
	//箱が視錐台の中に入ってるかどうか判定するときに使う
	vec3<T> getVertexN(vec3<T> &normal) {//Negative

		vec3<T> res = corner;

		if (normal.x < 0)
			res.x += x;

		if (normal.y < 0)
			res.y += y;

		if (normal.z < 0)
			res.z += z;

		return(res);
	}
	//箱が視錐台の中に入ってるかどうか判定するときに使う
	vec3<T>  getVertexP(vec3<T>  &_normal) {//Positive

		vec3<T> res = corner;

		if (_normal.x > 0)
			res.x += x;

		if (_normal.y > 0)
			res.y += y;

		if (_normal.z > 0)
			res.z += z;
		
		return(res);
	}
};
}
#endif
