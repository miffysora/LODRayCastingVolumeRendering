#pragma once
#include <windows.h>
#include <GL/glew.h> 
#include <GL/glfw.h>
#include <GL/freeglut.h>
#include <tchar.h>
#include <sstream>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <fstream>
#include <list>
using namespace std;
//視錐台カリング関係
#include <miffy/math/collisiondetection/frustum.h>
#include <miffy/math/vec2.h>
#include <miffy/math/vec3.h>
#include <miffy/math/vec4.h>
#include<miffy/math/matrix.h>
#include <miffy/math/quaternion.h>
#include "Block.h"
#pragma comment(lib,"GLFW.lib")
#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"cg.lib")
#pragma comment(lib,"cgGL.lib")
using namespace miffy;

class Lod
{
public:
	Lod(void);
	~Lod(void);
	void display();
	void RotateFromScreen(int _mx,int _my);
	void reshape(int _width,int _height);
	void renderGeometry(list<Block>& _blocklist);
	void zoom(int _direction);
private:
	void OctreeTraversal(const Block& _parent_block, list<Block>* _dst);
	
private:
	
	list<Block> m_block_list;
	mat4<float> m_modelview;
	mat4<float> m_rotation_matrix;
	mat4<float> m_proj_matrix;
	mat4<float> m_inv_modelview;
	mat4<float> m_inv_next_modelview;
	mat4<float> m_next_modelview;
	vec2<float> m_translate;
	float m_zoom;
	frustum<float> m_render_frsutum;
	const 	vec3<double> m_world_eye_pos;//ワールド座標での視点座標
	const float m_near;
	const float m_far;
	const float m_fovy;
	float m_aspect_ratio;
	Block m_root_block;
	
	
public:
	vec2<int> m_win_size;
	vec2<int> m_last_pushed;
	quat<float> m_current_quaternion;
	quat<float> m_target_quaternion;
};

