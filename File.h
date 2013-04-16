/* ------------------------------------------------------

 View Frustum - Lighthouse3D

  -----------------------------------------------------*/

#ifndef MIFFY_FRUSTUM
#include <miffy/math/collisiondetection/frustum.h>
#endif
#ifndef _FILE_
#define _FILE_


#ifndef _BLOCK_
#include "Block.h"
#endif
class Block;
using namespace std;
using namespace miffy;
/// メインブロックリクエストキュー管理構造体
typedef struct _RequestQueue{
  ULONGLONG needflag;           /// そのブロックが必要か否か　優先度スコア
	Block* block;
}RequestQueue;

/// メインブロック管理構造体　ブロックの数だけ作る
typedef struct _MainMemManage{
	ULONGLONG needflag;/// 優先度スコア
	Block* block;
	int FromLoadTime;
	float* data;                     //texSubImage3Dに渡す、GPUに渡すデータのポインタ。実際の場所を指すアドレス
	bool rockflag;                   //上書きを禁止するかフラグ
	bool loadstartflag;       //そのブロックをロードしはじめたか否か
	LPOVERLAPPED ol;//winAPI
	HANDLE Fl;
}MainMemManage;


/// 全メインブロック管理用構造体
typedef struct _bstruct{
	MainMemManage* dataIndex;  //メインメモリ上のどこに存在するかのアドレス
	RequestQueue*  queIndex;
}bstruct;

//// テクスチャブロック管理用構造体*/
typedef struct _TexMemManage{
    ULONGLONG needflag;           //そのブロックが必要か否か
	GLuint* texdata;                     //実際の場所を指すアドレス
	Block* block;
}TexMemManage;

/*全テクスチャブロック管理用構造体*/
typedef struct _btexstruct{
	//テクスチャメモリ上のどこに存在するかのアドレス
	TexMemManage* texdataIndex;
}btexstruct;

class File 
{

private:
	/*visible index(全階層分持つ)*/
   unsigned int* indexblock[NUMLEVEL];
   unsigned int* texCompressInfo[NUMLEVEL-1];
   string m_Path;//多解像度ボリュームデータの保存してあるディレクトリ
   string m_DataName;//ボリュームデータセットの名前
   /*メインメモリ制御用*/
	bstruct**** bs;//MainMemManageなどを入れておく4次元配列                   
	MainMemManage*  memblock;
	float*    datapool;
	//リクエストキュー
	RequestQueue* reqque;

	/*テクスチャメモリ制御用*/
	btexstruct**** bstex;
	TexMemManage* texblock;
	GLuint*    texName;

	float loadTime;
	float texLoadTime;
	bool  texLoadflag;

	/*ハードディスク読込み時間測定用*/
	LARGE_INTEGER* mTimeStart;
	LARGE_INTEGER* mTimeEnd;
	LARGE_INTEGER freq;
	float temp_sum;
	int temp_count;

	/*ハードディスク読込み時間測定用*/
	LARGE_INTEGER fTimeprev,before,after,freq2;
	LARGE_INTEGER fTime;
	float fileThreadTime;
	double maintotextime;

public:
	 
	File();
	~File();
	void Init(string path,string dataname);
	void deleteMemory(btexstruct* bstex);
	void deleteMemory(bstruct* bs);
	LARGE_INTEGER getLaddress(long long address);
	void loadFile(string file,float* data,int header,int fx, int fy, int fz,MainMemManage* mmm);
	void loadIndexFile(string file,unsigned int* data,int header,int fx, int fy, int fz);
	GLuint* getTexaddress(int index);
	
	void loadHDToMain(void);
	void loadMainToTex(Block bl,Cg* Cg,CGparameter decal);
	void setTexBlock(Cg* Cg,CGparameter decal,Block block);

	void countMainLRU(void);
	void countTexLRU(void);

	int getIndexblock(Block bl);
		
	bool blockExist(Block bl);
	Block lowBlockExist(Block bl);
	Block lowBlockExistFromRoot(Block bl);
	bool highBlockExist(Block bl);
	int getTexCompressInfo(Block bl);
	bool texBlockExist(Block bl);
	Block texLowBlockExist(Block bl);
		
	GLuint getTexName(int i);
	void RockBlock(Block bl);
	void UnRockBlock(Block bl);
	/*引数blがまだreqqueに入れられてないなら空いてる場所orreqqueが満杯なら優先度の一番低いところに入れる。blがもうreqque、またはメインメモリに転送済みにあるなら優先度を更新する*/
	void mainBlockRequest(double* modelmatrix,double* projmatrix,Block bl,frustum<float> _frustum,frustum<float>::FrustumName fname);
	void texBlockRequest(double* modelmatrix,double* projmatrix,Block bl,frustum<float> _frustum,frustum<float>::FrustumName fname);
	

	ULONGLONG getReqbit(double* modelmatrix,double* projmatrix,Block bl,frustum<float> _frustum,frustum<float>::FrustumName fname);

	float getTexLoadTime();
	bool  getTexLoadflag();


	void CheckLoadComplete();
	void CheckLoadComplete(int i);

	float getMainLoadTime();
	
	void countThreadTime();/*ハードディスク読込み時間測定用*/
	float getThreadTime();
	double getMainToTexTime();
	void deleteMemory();
	int returnMainMemInfo(int _i);
	void initTexLoad();
	int returnTexMemInfo(int _i);
	void initMainLoad();
};


#endif
