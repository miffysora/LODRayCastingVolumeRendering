#include "Lod.h"

Lod::Lod(void)
	:m_world_eye_pos(vec3<double>(0.0, 0.0, 9.0))
	,m_near(0.1f)
	,m_far(100.0f)
	,m_fovy(30.0f)
	,m_zoom(0.0f)
{
	Block::Init(1024,64,1.0);
	
}
void Lod::display(){
	//描画
	glClearColor(0.5,0.5,0.5,1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//ここから描きたいものを描画
	//読み取りよう視錐台のモデルビュー行列作成
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();

	glLoadIdentity();
	gluLookAt(m_world_eye_pos.x, m_world_eye_pos.y, m_world_eye_pos.z,0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	glTranslated(m_translate.x,m_translate.y,m_zoom);/* 平行移動(奥行き方向) 　　*/
	m_target_quaternion.toMat4(const_cast<float*>(m_rotation_matrix.m));
	glMultMatrixf(m_rotation_matrix.m);//クォータニオンによる回転
	OctreeTraversal(Block::ROOT_BLOCK,&m_block_list);
	renderGeometry(m_block_list);//ボリュームデータとかを描画。

	glPopMatrix();



}//for display
void Lod::OctreeTraversal(const Block& _parent_block, list<Block>* _dst){
	if(_parent_block.IsBestResolution(m_modelview,m_proj_matrix,m_win_size)){
		Block children;
		for(int z=0;z<2;z++){
			for(int y=0;y<2;y++){
				for(int x=0;x<2;x++){
					//_parent_block.info("root");
					//Block child=_parent_block.getChildren(x,y,z);
					OctreeTraversal(_parent_block.getChildren(x,y,z),_dst);
				}
			}
		}
	}else{
		//_parent_block.info("push");
		_dst->push_back(_parent_block);
	}
	
}
void Lod::RotateFromScreen(int _mx,int _my){
	float dx=(float)(_mx-m_last_pushed.x)/(float)m_win_size.x;
	float dy=(float)(_my-m_last_pushed.y)/(float)m_win_size.y;
	//過去との回転の合成をどうやっていいかわからない。やっぱクオータニオンが必要
	vec3<float> rotate_axis=vec3<float>(dy,dx,0.0);
	float axislength=rotate_axis.length();
	if(axislength!=0.0){
		float radian=(float)fmod(axislength*(float)M_PI,360.0);//画面いっぱいで調度一周になるようにする。
		rotate_axis.normalize();//軸の長さを1にする。
		quat<float> difq(cos(radian),rotate_axis.x*sin(radian),rotate_axis.y*sin(radian),0.0);
		m_target_quaternion=difq*m_current_quaternion;
		
	}
}
void Lod::renderGeometry(list<Block>& _blocklist){
	//描画
	list<Block>::iterator it=_blocklist.begin();
	while(it!=_blocklist.end()){
		it->renderBlockLines();
		it++;
	}
}
void Lod::reshape(int _width, int _height){
	m_win_size.set(_width,_height);
	m_translate.x = 0.0;
	m_translate.y = 0.0;
	
	
	glViewport(0, 0, (GLsizei) _width, (GLsizei) _height);

	
	m_aspect_ratio = (float)_width/(float)_height;

	//ブロック前後クリッピング初期化
	glMatrixMode(GL_PROJECTION);  /* 投影変換の設定 */
	glLoadIdentity(); 
	gluPerspective(m_fovy, m_aspect_ratio, m_near, m_far);//original
	glGetFloatv(GL_PROJECTION_MATRIX, m_proj_matrix.m);	
	m_render_frsutum.setFromPerspective(m_proj_matrix,m_fovy,m_aspect_ratio,m_near,m_far);	
	glMatrixMode(GL_MODELVIEW);


}
void Lod::zoom(int _direction){
	m_zoom=(float)-_direction*0.4f;
	
}

Lod::~Lod(void)
{
}
