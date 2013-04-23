//目的：ブロックの８つの頂点情報を溜め込む。
//テクスチャとはテクスチャ番号でつながれてる。
//テクスチャはブロックオブジェクトの配列のインデックス番号でもある。
#pragma once
#include <stdio.h>
#include <math.h> 
#include <assert.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <sstream>
#include <string>
#include <deque>
#include <list>
#ifndef MIFFY_VEC2
#include <miffy/math/vec2.h>
#endif
#ifndef MIFFY_VEC3
#include <miffy/math/vec3.h>
#endif
#ifndef MIFFY_VEC4
#include <miffy/math/vec4.h>
#endif
#ifndef MIFFY_FRUSTUM
#include <miffy/math/collisiondetection/frustum.h>
#endif
#include <miffy/math/cube.h>
using namespace miffy;
using namespace std;
class Block
{
public:
	//いつも必要
	int m_x,m_y,m_z;//ブロックインデックス
	cube<float> m_cube;
	int m_level;//詳細度レベル 0=オリジナル解像度
	float m_size;//1辺のサイズ
	//ブロックが視錐台に入ってるかどうか判定するときに使う
	int m_block_num;
	enum blockstate{CHILDREN,PARENT,SAME};
	float distanceFromCamera;
	Block();
	Block( int _x, int _y, int _z, int _level, int _blockNum,  float _blockLength);
	const Block getChildren(int _x,int _y,int _z)const;
	//Block& operator=(const Block& _in);
	//destructor
	~Block(void);
	//settings
	void set(const int _x,const int _y,const int _z,const int _level,const int _blockNum,const  float _blockLength);
	void setMySerialNumber(int id);
	//calculation
	void convertTexCoordToworldCoord(int indexX,int indexY, int indexZ, int bnum);
	//レンダリング
	void renderBlockQUADS(float times=0.5f);
	void renderBlockLines();
	void info(const char* _message)const;
	int IsInFrustum(const frustum<float>& _frustum);
	bool IsBestResolution(const mat4<float>& _modelMatrix,const mat4<float>& _projmatrix,const vec2<int>& _winsize)const;

	//視錐台カリング
	vec3<float> getVertexP(vec3<float> &normal) ;
	vec3<float> getVertexN(vec3<float> &normal) ;
	static  unsigned int ORIGINAL_VOXEL_NUM;//2のべき乗である必要がある。
	static  unsigned int BOXEL_PER_BLOCK;
	static  unsigned int LEVEL_NUM;
	static Block ROOT_BLOCK;
	static void Init(int _original_voxel_num,int _voxel_per_block,float _root_length);
private:

};
