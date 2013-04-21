#include "Block.h"
vec3<float> texCoord[8]={
	vec3<float>(0,0,0),
	vec3<float>(1,0,0),
	vec3<float>(1,1,0),
	vec3<float>(0,1,0),	
	vec3<float>(0,0,1),
	vec3<float>(1,0,1),
	vec3<float>(1,1,1),
	vec3<float>(0,1,1),
};
vec3<float> blsign[8]={
	vec3<float>(0,0,0),
	vec3<float>(-1.0f,0,0),
	vec3<float>(-1.0f,-1.0f,0),
	vec3<float>(0,-1.0f,0),	
	vec3<float>(0,0,-1.0f),
	vec3<float>(-1.0f,0,-1.0f),
	vec3<float>(-1.0f,-1.0f,-1.0f),
	vec3<float>(0,-1.0f,-1.0f),
};
 unsigned int Block::LEVEL_NUM=0;//実態
/*!
@brief デフォルトコンストラクタはルートのブロック
*/
Block::Block()
	:m_x(0),m_y(0),m_z(0),level(Block::LEVEL_NUM),m_block_num(1),m_size(1.0){
		m_cube.setFromCorner(vec3<float>(-0.5f,-0.5f,-0.5f),1.0f);
}
Block::Block(const int _x,const int _y,const int _z,const int _level,const int _blockNum,const  float _blockLength)
	:m_x(_x),m_y(_y),m_z(_z),level(_level),m_block_num(_blockNum),m_size(_blockLength)
{
	m_cube.setFromCorner(vec3<float>(0,0,0),1.0);
}
const Block Block::getChildren(int _x,int _y,int _z){
	return Block(_x,_y,_z,m_level++,m_block_num*2,m_size*0.5f);
}
//Block& Block::operator=(const Block& _in){}
 void Block::Init(int _original_voxel_num,int _voxel_per_block,float _root_length){
	Block::LEVEL_NUM=0;
	Block::ORIGINAL_VOXEL_NUM=_original_voxel_num;
	Block::BOXEL_PER_BLOCK=_voxel_per_block;
	int voxel=Block::ORIGINAL_VOXEL_NUM;
	while(voxel!=Block::BOXEL_PER_BLOCK){
		voxel=voxel>>1;
		Block::LEVEL_NUM++;
	}
	ROOT_BLOCK.set(0,0,0,Block::LEVEL_NUM,1,_root_length);
}
 void set(const int _x,const int _y,const int _z,const int _level,const int _blockNum,const  float _blockLength){
 }
void Block::setMySerialNumber(int id){
	this->mySerialNumber=id;
}
/*!
@brief frustum<T>の汎用性を保持したかったのでBlockのほうに判定を作った。
*/
int Block::IsInFrustum(const frustum<float>& _frustum){
	//もしビューボリュームにblockが入っていれば
	aabox<float> abox(m_cube.corner[0],m_size,m_size,m_size);//ブロックの情報をAxis Aligned Boxで表現している。AABoxのx,y,zはブロックの辺の長さ。
    return _frustum.boxInFrustum(abox);
}
Block::~Block(void){}

void Block::renderBlockLines(){

	glColor3f(1.0,1.0,1.0);
	m_cube.DrawWireCube();
}

void Block::renderBlockQUADS(float times){
	glColor4f(this->x*times,this->y*times,this->z*times,0.5);
	m_cube.DrawQuads();
}
/*!
@return false:これ以上詳細にする必要なし　true:もっと詳細にしてよし
*/bool Block::IsBestResolution(const mat4<float>& _modelMatrix,const mat4<float>& _projmatrix,const vec2<int>& _winsize)const
{//最適レベルの値を返す
	if( m_level==0){//オリジナル解像度なので無理。
		return false;
	}
	//まずは一番遠い頂点インデックスを求める
	vec3<float> viewvec(-_modelMatrix.m[2],-_modelMatrix.m[6],-_modelMatrix.m[10]);
	float maxdist = viewvec.innerProduct(m_cube.corner[0]);//内積を求める
    float mindist = maxdist;
    int far_id = 0;//nMaxIdxは、m_pEdgeListのためのインデックス
    for(int i = 1; i < 8; ++i) {
		float dist = viewvec.innerProduct(m_cube.corner[i]);//各頂点との内積を求める
        if ( dist > maxdist) {
            maxdist = dist;
            far_id = i;//マウスで箱を動かすとここの値が変わる。初期値では0
        }
    }//一番遠い頂点調べるの終わり
	//int=0 粗くする 1=stay same 2=詳細にする

	cube<float> fartherest_voxel;
	fartherest_voxel.setFromCorner(
		vec3<float>(m_size/BOXEL_PER_BLOCK,m_size/BOXEL_PER_BLOCK,m_size/BOXEL_PER_BLOCK),
		(float)m_size/(float)BOXEL_PER_BLOCK*0.5f);//*0.5fにしたのは、仮にもう一段階したとき、１ピクセルより小さくなるかどうかってやり方にしたいから
	vec2<float> projected_size=fartherest_voxel.projectedsize(_modelMatrix,_projmatrix,_winsize);
	if(projected_size.x<1.0f || projected_size.y<1.0f){return false;}//これ以上詳細にしなくてもいい					
	return true;

		
}

