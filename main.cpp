// RayCasting.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include <GL/glew.h>
#include "Block.h"
#include "../Matrix3X3.h"
#include "../App.h"

#include "../FrustumG.h"
#include "../Vec3.h"
#include "Cg.h"
#include "File.h"
//#include "../../Color.h"
#include <math.h>
#include <fstream>
#include "../Matrix4X4.h"
#include "../Vec4.h"
#include "../Mathematics.h"
#include "../Color.h"
#include <queue>//<queue>ヘッダの内部で<deque>ヘッダを呼んでいる。dequeはdouble-ended-queue（両頭キュー）の略。
#include <filesystem>//カレントディレクトリを知るのに必要
using namespace std;
using namespace Mahbub;

int maxRenderNum;//1度に描画してもよい数
int MAXBNUM;  //メモリに格納できる最大ブロック数(一つはファイルロード用) raycasting.cpp
int QSIZE;
//int MAXTEXNUM;
int BACKUPTHRE; //ファイルのwhileループ1回あたりに読み込んでもよいブロックの最大数
GLuint preintName;

float nearSlice;
float farSlice;
float eyeInitPosition[4] = {0.0, 0.0, 9.0, 1.0};

int order=0;//描かれた順
int winwidth = 720;
int winheight = 576;


//パラ5
float NFrustumSize = INITNFRUSTUMSIZE; //normalfrustumのサイズはAPIビューイングフラスタムの何倍？(台形の体積で考えてます)

void DDAblock(Vec3 viewvec, Vec3 abview,Block bl,int* normalReso,int* backupReso,int* existblcount,deque<Block>* blockQue);
void DDAblockFile(Vec3 viewvec, Vec3 abview,Block bl,int* _reso,int* _exist_in_main_count,unsigned int* _blockstate,bool ****needCountMap,bool **** existInMainMap,deque<Block>* blockQue);
void testLoadFrustum(Block block,int* octreeReso,int* existblcount,unsigned int** renderblock,bool **** needCountflag,bool **** existCountflag, deque<Block>* blockQue);
void initCountflag(int diff,bool **** needCountflag,bool **** existCountflag);
void SetFrustum();//idle,reshape,motionで使われる。displayでは使用されない。
void SetLoadFrustum();//setFrustumで使われるだけ
void SetRenderFrustum();//setFrustumで使われるだけ
void createTF();
void exit_func(void);
Vec3 getViewvector(Vec3* view_z);
void testRenderFrustum(Block block,int* Preso,int* Sreso,int* NeedInTex_ExistInMainBlcount,deque<Block>* blockQue);
/*/////////////////////////////////////////////////////////////////////
CG関係
//////////////////////////////////////////////////////////////////////*/
Cg Cg;


/*///////////////////////////////////////////////////////////////////////////////
ファイル管理
//////////////////////////////////////////////////////////////////////////////*/
File file;
//static string path =  "C:/takasao/Largedata_rendering/LargeData/";
static string path =  "../../";
static string dataname = "VertebralBody_Brick";


/*///////////////////////////////////////////////////////////////////////////////
シェーディング関係
////////////////////////////////////////////////////////////////////////////////*/
float eyePosition[4];
float eyeDis;


float globalAmbient[3] = {0.5f, 0.5f, 0.5f};
float lightAPosition[4] = {0.0f, 0.0f, 20.0f, 1.0f};
float lightPosition[4];
float lightColor[3] = {1.0f, 1.0f, 1.0f};

float Ke[3] = {0.0f, 0.0f, 0.0f};
float Ka[3] = {0.2f, 0.2f, 0.2f};
float Kd[3] = {0.7f, 0.7f, 0.7f};
float Ks[3] = {0.9f, 0.9f, 0.9f};
float shininess = 50.0f;

Matrix4X4 modelViewMatrix;
Matrix4X4 inv_modelView;

Matrix4X4 Next_modelViewMatrix;
Matrix4X4 Next_inv_modelView;
Vec4 Next_translate;
double Next_modelMatrix[16];
const double IdleTranslateRatio = 0.25;      /* idle時の平行移動の比　*/
const double TranslateEpsilon = 1.0e-6;      /* 平行移動の閾値          */

/*///////////////////////////////////////////////////////////////////////////////
ブロッククリッピング関係
////////////////////////////////////////////////////////////////////////////////*/
FrustumG frustum;
FrustumG renderfrustum;

float aspect;
float thetaw,thetah;
float Bclipresio = INITFAR / INITNEAR;
float Bclipeyeresio;



/*///////////////////////////////////////////////////////////////////////////////
マルチスレッド関係
////////////////////////////////////////////////////////////////////////////////*/
/*スレッド関数の引数データ構造体*/
typedef struct _thread_arg {
  int thread_no;
	int *data;
} thread_arg_t;


//時間測定用ファイル
LARGE_INTEGER before,after,freq;
 double frameTime;
 double maintotextime;

/*//////////////////////////////////////////////////////////////////////////////////
その他
///////////////////////////////////////////////////////////////////////////////*/

 int BBLoadMargin;
const double AngleRatio = 0.25;          /* マウスの移動と回転角の比 */
const double IdleAngleRatio = 0.25;      /* idle時の回転角の比　*/
const double AngleEpsilon = 1.0e-6;      /* 回転の閾値          */
int buttondown = -1;                     /* ボタン状態 up(-1)/left(0) */
int beginX, beginY;                      /* マウスカーソルの位置 */
double angleDelta = 0.0;                 /* idle時の回転角(角速度)   */
double originX, originY, originZ;        /* 原点位置　　　　　　*/
double axisX, axisY, axisZ;              /* idle時の回転角(方向ベクトル) */
double modelMatrix[16];
double projMatrix[16];
int viewport[4];


void multMatrix(double matrix[], float org[], float dst[]);

/*////////////////////////////////////////////////////////////////////////////////////
計測
////////////////////////////////////////////////////////////////////////////////////*/
/*FPS測定・表示*/
int frame,miffytime,timebase=0;
void* font=GLUT_BITMAP_HELVETICA_10;

stringstream floadtime;
stringstream texloadtime;
stringstream oquerytime;
stringstream tptime;
stringstream blockrendertime;
stringstream trtime;
stringstream mrtime;

stringstream fps;

stringstream normalnbnstr;
stringstream normalebnstr;
stringstream normalresostr;
stringstream backnbnstr;
stringstream backebnstr;
stringstream backresostr;
stringstream rendernbnstr;
stringstream renderebnstr;
stringstream renderresostr;

stringstream occlutionnum;
stringstream renderblocknum;
stringstream loadtexblocknum;
stringstream maxloadtexblocknum;
stringstream maxloadmainblocknum;
stringstream filethreadtime;

stringstream widthstring;
stringstream heightstring;

//stringstream mainmemlog;//メインメモリの変遷を記録
//stringstream mainmemParameterLog;//メインメモリに関するパラメータの記録
stringstream timelog;//1フレーム内でどんな処理にどのくらいの時間がかかったか記録する
/*レンダリング時間計測構造体*/
typedef struct _TotalRendering {
	float totalrendertime;
	bool flag;
} TotalRendering;

TotalRendering totalrender;


int normalnbn = 0;//描画に必要な現解像度ブロックの数
int normalebn = 0;//既に存在している現解像度ブロックの数
int backnbn = 0 ;
int backebn = 0;
int idealBlockNum=0;//最適解像度ブロックの数
bool backupOrNot=false;
int renderBlockNum=0;//実際にレンダリングしているブロックの数
float fpsresult;
float raycasttime;
int frame_special=0;
int frame_special_occ=0;
int frame_tex_pro=0;
int tex_req_frame=0;
float rbn;
float obn;
float ltn;
float occlutiontime;
float texprocesstime;
float texrequesttime;
float memrequesttime;
float totalrendertime_sum=0;
 int blockStateCount[10];//miffy original

int renderblockcount = 0;//レンダリングしてるブロックの数をカウント
int loadtexblockcount = 0;

double bltime_sum=0;//1フレームごとの足し算
double blcount=0;//1フレームごと
double bltime_perframe;//1フレームの、ブロックの各処理にかかった時間の平均
int MAXTEXLOAD;


bool subwindowflag = true;
bool showGridFlag=false;
FrustumG::FrustumName frustumstate = FrustumG::render;//normal;
bool infoflag = false;


float dPlaneStart;     //スライス開始位置


GLuint pbo;//使ってない。
GLuint fb = 0;
GLuint fbTex = 0;
GLuint fbDep = 0;


bool**** n_needInMain;
bool**** n_existInMain;
bool**** b_needInMain;
bool**** b_existInMain;
bool**** render_normal_blocks;//どのメモリにあるなし関係なく今描画に必要なブロックリスト
int render_normal_num=0;
bool**** r_existInMainMemNotInTexMem;//テクスチャメモリにはないけどメインメモリにはある
unsigned int **** blockQueueMap;//キューの何番目に何のブロックが入ってるかマップ
int pinchBlockCount=0;
HANDLE sem;
Vec4 ini_cam(0.0f,0.0f,0.0f,1.0f);
Vec3 ini_l(0.0,0.0,-1.0);
Vec3 ini_u(0.0,1.0, 0.0);
bool zoomflag = false;
bool nowprocessingflag=false;//結果をファイル出力するための文字列作成のため
deque<Block> bRequestQueue;//requestNormalFrustumで使う
deque<Block> nRequestQueue;//requestNormalFrustumで使う
deque<Block> rnRequestQueue;//バックアップと現解像度の中から選ばれたやつ（だと思う）
deque<Block> rbRequestQueue;//バックアップと現解像度の中から選ばれたやつ（だと思う）MAXTEXLOADを超えたときはこっちを使う。（しかし、このマシンのスペックならこのループに入ることはまずないっぽい）

int occblockcount;
int normalreso;
int backupreso;

void destroyFBO()
{
	glDeleteFramebuffers(1,&fb);
	glDeleteTextures(1, &fbTex);
	glDeleteTextures(1, &fbDep);
	fb = 0;
	fbTex = 0;
	fbDep = 0;
}
void check_framebuffer_status() 
{ 
	GLenum status; 
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER); 
	switch(status) { 
  case GL_FRAMEBUFFER_COMPLETE_EXT: 
	  break; 
  case GL_FRAMEBUFFER_UNSUPPORTED_EXT: 
	  fprintf(stderr, "choose different formats\n"); 
	  break; 
  default: 
	  fprintf(stderr, "programming error; will fail on all hardware: %04x\n", status); 
	} 
}
void createFBO(){
	if(fb > 0)
		destroyFBO();

 /* フレームバッファオブジェクトを生成 */
	glGenFramebuffersEXT(1,&fb);//1=フレームバッファの名前(0はウィンドウシステムから与えられた名前だから使っちゃダメ),fb=フレームバッファの名前を入れるところ。つまり、1という値をfbに入れるところ。
	//書き込まれるテクスチャの設定
	glGenTextures(1, &fbTex);
	 /* フレームバッファオブジェクトを結合 */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);//fbという名前のフレームバッファにGL_FRAMEBUFFERというフレームバッファオブジェクトをboundする。たいていGL_FRAMEBUFFERらしい。
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB,fbTex);

	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB,0,GL_RGBA,winwidth,winheight,0,GL_RGBA,GL_FLOAT,NULL);//これがないと画面が白くなる。
	 /* フレームバッファオブジェクトに２Dのテクスチャオブジェクトを結合する */
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB,fbTex, 0 );//これがないとボリュームがなくなる。
	glBindTexture(GL_TEXTURE_2D,0);
	  /* フレームバッファオブジェクトの結合を解除する */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	cgGLSetTextureParameter(Cg.fdecalParam,fbTex);
	cgGLSetTextureParameter(Cg.fdecal2Param,fbTex);//なくても問題ない。


	glGenTextures(1, &fbDep);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB,fbDep);

	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	//use bilinear filtering
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB,0,GL_DEPTH_COMPONENT24,winwidth,winheight,0,GL_DEPTH_COMPONENT,GL_INT,NULL);//これがないとボリュームがなくなる。
	//テクスチャをフレームバッファにアタッチします。
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB,fbDep, 0 );

	glBindTexture(GL_TEXTURE_2D,0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	check_framebuffer_status();
}
/*
** 初期化
*/
void myInit()
{	
	/*//////////////////////////////////////////////////////////////
	テクスチャ1(高解像度ブロック×MAXTEXNUM）
	//////////////////////////////////////////////////////////////*/
	
	for(int i=0;i < MAXTEXNUM;i++)
	{
		glGenTextures(1, file.getTexaddress(i));
		glBindTexture(GL_TEXTURE_3D, file.getTexName(i));
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
		glTexImage3D(GL_TEXTURE_3D,0,GL_ALPHA,BLX+2,BLY+2,BLZ+2,0,GL_ALPHA, GL_FLOAT,NULL);//これがないとボリュームがなくなる。
		
	}//なぜ２のべき乗でなくて平気なのか？
//	glBindTexture(GL_TEXTURE_2D, 0);//これがなくても支障はない。
	

	const GLubyte *i_vendor, *i_renderer, *i_version;
	i_vendor = glGetString(GL_VENDOR);
	i_renderer = glGetString(GL_RENDERER);
	i_version = glGetString(GL_VERSION);
	cout <<"OpenGL information : "<<endl;
	cout <<"  vendor   : "<< i_vendor << endl;
	cout <<"  renderer : "<< i_renderer<<endl;
	cout <<"  version  : "<< i_version<<endl;



	/* 初期設定 */
	//glClearColor(1.0,1.0,1.0,1.0);//印刷用
	glClearColor(0.0, 0.0, 0.0, 0.0);//フレームバッファを使ってオクルージョンテストするなら背景は黒にすべき
	glClearDepth(1.0);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);


	//トータルレンダリング時間計測構造体初期化
	totalrender.totalrendertime = 0.0;
	totalrender.flag = false;

}
void lighting(void) {                      /* ライトの指定 　　　　　　*/
	Cg.SetMatrixParameter(Cg.matModelViewNormalParam,CG_GL_MODELVIEW_MATRIX,CG_GL_MATRIX_INVERSE_TRANSPOSE);
	Cg.SetParameter(Cg.lightColorParam,lightColor);
	Cg.SetParameter(Cg.lightPositionParam,lightPosition);
	Cg.SetParameter(Cg.globalAmbientParam,globalAmbient);
	Cg.SetParameter(Cg.eyePositionParam,eyePosition);
	Cg.SetParameter(Cg.KeParam,Ke);
	Cg.SetParameter(Cg.KaParam,Ka);
	Cg.SetParameter(Cg.KdParam,Kd);
	Cg.SetParameter(Cg.KsParam,Ks);
	Cg.SetParameter(Cg.shininessParam,shininess);
}
Vec3 getViewvector(Vec3* view_z){

	
	*view_z = inv_modelView.vec_maltiply3(view_z);

	if(view_z->x == 0.0f)
		view_z->x = 0.0001f;
	if(view_z->y == 0.0f)
		view_z->y = 0.0001f;
	if(view_z->z == 0.0f)
		view_z->z = 0.0001f;



	//front_to_back だからベクトル逆にしている
	view_z->x = -view_z->x;
	view_z->y = -view_z->y;
	view_z->z = -view_z->z;


	Vec3 abview;

	abview.x = 1.0f/(abs(view_z->x)*SPX);
	abview.y = 1.0f/(abs(view_z->y)*SPY);
	abview.z = 1.0f/(abs(view_z->z)*SPZ);

	abview.normalize();
	return abview;
}
void ProcessNextResoRender(Block bl,int* countA,int* countB,int* countC,deque<Block>* blockQue)//1段階高解像度のブロックのノードを探索
{
	Block nextbl = bl.getHighblock();//仮に1段階高解像度のブロックのノードを探索
	Vec3 vz = nextbl.getBlockVec(modelViewMatrix);
	Vec3 ab = getViewvector(&vz);
	DDAblock(vz,ab,nextbl,countA,countB,countC,blockQue);
	bl.setBlockState(Block::nextreso,Block::rRequestBlock);
}
void ProcessNextResoLoad(Block bl,int* countA,int* countB,unsigned int** rblock,bool **** needCountflag,bool **** existCountflag,deque<Block>* blockQue)//NExt->次のより高解像度のやつって意味。
{//Block blを与えられ、詳細度レベル一個上のを作ってる。bl一個につき一つ高解像度のやつを８個作ってる。
	for(int i=0;i<2;i++)//x loop
	{
		for(int j=0;j<2;j++)
		{
			for(int k=0;k<2;k++)
			{//countAは0～4の値。詳細度レベルを示していると思われる。
				testLoadFrustum(Block(2*bl.x+i,2*bl.y+j,2*bl.z+k,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2),countA,countB,rblock,needCountflag,existCountflag,blockQue);
			}//Block(2*bl.x+i,2*bl.y+j,2*bl.z+k,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2).getMultiLowblock(countA)という処理をブロックがblockQueに登録される。
		}
	}//x loop
	bl.setBlockState(Block::nextreso,rblock);
}
void renderBlock(Block block)
{
	//block.printBlockInfo("ryacasting",0);
	//Occlusion Culling判定
	bool occludeState=block.testOcclusion(&Cg);
	block.setOccludeState(!occludeState);//Block::occludedIndex[block.level][block.x][block.y][block.z]=!occludeState;
	if(occludeState)//もしアルファ値がthresholdよりも小さいフラグメントが存在するならtrue,存在しないならflaseが変える
	{
		Block lowResoblock = block;
		do{//do-whileループを外すとちらちら見えなくなるブロックが増える
			//初めにテクスチャメモリを探す
			if(file.texBlockExist(lowResoblock))
			{				

				file.setTexBlock(&Cg,Cg.vdecalParam,lowResoblock);//シェーダにテクスチャ名を渡したりする。setTextureParameterする。
				file.texBlockRequest(modelMatrix,projMatrix,lowResoblock,renderfrustum,FrustumG::render);
				int dif = lowResoblock.level - block.level;
			
				block.RayCast(&Cg,dif);//ここでボリューム描画
				//block.renderNumber(order,0.83,0.9,0.0);
				//order++;//描画順を調べるためのもの
				Block::blockState bstate = block.getLowState(lowResoblock);
				//printf("bstate=%d\n",bstate);
				block.setBlockState(bstate,Block::rRequestBlock);
				renderblockcount += 1;	//レンダリングしてるブロックの数をカウント

				break;//無事にレンダリング出来たらループを抜ける
			}
			else
			{block.renderBlockLines(0.0,1.0,0.0,0.85);//緑はテクスチャメモリのロード町
				block.setBlockState(Block::waitblock,Block::rRequestBlock);}//テクスチャメモリへのロード待ち


			if(lowResoblock.level == NUMLEVEL -1)
			{	lowResoblock.renderBlockLines(1.0,0.0,0.0,0.85);//level4すらない、（赤）
			break;
			}else{//if(lowResoblock.level==3){block.printBlockInfo("最低段階行？",0);}
				lowResoblock = lowResoblock.getLowblock();//1段階下にしてやりなおし。ｓ
			}
		}while(1);
	}else//オクルージョンカリングされるもの
	{
		block.setBlockState(Block::occlusionculling,Block::rRequestBlock);
		occblockcount += 1;//オクルージョンカリングされたブロックのカウント
		block.renderBlockLines(1.0,1.0,0.0,0.85);//黄色
	}
}

void DDAblock(Vec3 viewvec, Vec3 abview,Block bl,int* normalreso,int* backupreso,int* existblcount,deque<Block>* blockQue)
{// abviewとviewvecは対の関係みたいになってる。
	int DDAb[3];
	if(bl.level == NUMLEVEL-1)
	{
		DDAb[0] = INIBLX;//2
		DDAb[1] = INIBLY;//2
		DDAb[2] = INIBLZ;//2
	}
	else
	{
		DDAb[0] = 2;
		DDAb[1] = 2;
		DDAb[2] = 2;
	}

	int deltal;
	int deltam;                  
	int deltas;                  
	int num;
	int llength;
	int mlength;
	int slength;
	//abview.print("abview");//abviewの成分は0か1
	if(abview.x >= abview.y && abview.y >= abview.z)
	{
		deltam = (int)(DDAb[2]*abview.y/abview.z);
		deltas =  DDAb[2];
		deltal = (int)(DDAb[2]*abview.x/abview.z);
		llength = DDAb[0];
		mlength = DDAb[1];
		slength = DDAb[2];
		num = 0;
	}
	else if(abview.x >= abview.z && abview.z >= abview.y)
	{

		deltam = (int)(DDAb[1]*abview.z/abview.y);
		deltas = DDAb[1];
		deltal = (int)(DDAb[1]*abview.x/abview.y);
		llength = DDAb[0];
		mlength = DDAb[2];
		slength = DDAb[1];
		num = 1;

	}
	else if(abview.y >= abview.z && abview.z >= abview.x)
	{


		deltam = (int)(DDAb[0]*abview.z/abview.x);
		deltas = DDAb[0];
		deltal = (int)(DDAb[0]*abview.y/abview.x);
		llength = DDAb[1];
		mlength = DDAb[2];
		slength = DDAb[0];
		num = 2;
	}
	else if(abview.y >= abview.x && abview.x >= abview.z)
	{

		deltam = (int)(DDAb[2]*abview.x/abview.z);
		deltas = DDAb[2];
		deltal = (int)(DDAb[2]*abview.y/abview.z);
		llength = DDAb[1];
		mlength = DDAb[0];
		slength = DDAb[2];
		num = 3;
	}
	else if(abview.z >= abview.x && abview.x >= abview.y)
	{

		deltam = (int)(DDAb[1]*abview.x/abview.y);
		deltas = DDAb[1];
		deltal = (int)(DDAb[1]*abview.z/abview.y);
		llength = DDAb[2];
		mlength = DDAb[0];
		slength = DDAb[1];
		num = 4;
	}
	else
	{

		deltam = (int)(DDAb[0]*abview.y/abview.x);
		deltas = DDAb[0];
		deltal = (int)(DDAb[0]*abview.z/abview.x);
		llength = DDAb[2];
		mlength = DDAb[1];
		slength = DDAb[0];
		num =5;
	}




	int a;
	int* x;
	int* y;
	int* z;
	int i,j;
	int count = 0;
	double temp_sum= 0.0;

	//QueryPerformanceCounter(&temp1);
	//					QueryPerformanceCounter(&temp2);
	//			QueryPerformanceFrequency(&freq);					
	//			temp_sum += (double)(temp2.QuadPart-temp1.QuadPart)/(double)freq.QuadPart;
	//		count++;


	int sg[3];
	int vol[3];
	int vec_num;
	//viewvec.print("viewvec");//viewvecの成分は、0か1か-1 8通りの組み合わせ。
	if(viewvec.x >= 0.0 && viewvec.y >= 0.0 && viewvec.z >= 0.0)
	{
		sg[0]=1;sg[1]=1;sg[2]=1;
		vol[0]=0;vol[1]=0;vol[2]=0;
		vec_num = 0;
	}
	else if(viewvec.x < 0.0 && viewvec.y < 0.0 && viewvec.z < 0.0)
	{
		sg[0]=-1;sg[1]=-1;sg[2]=-1;
		vol[0]=DDAb[0]-1;vol[1]=DDAb[1]-1;vol[2]=DDAb[2]-1;
		vec_num = 1;
	}
	else if(viewvec.x >= 0.0 && viewvec.y >= 0.0 && viewvec.z < 0.0)
	{
		sg[0]=1;sg[1]=1;sg[2]=-1;
		vol[0]=0;vol[1]=0;vol[2]=DDAb[2]-1;
		vec_num = 2;
	}
	else if(viewvec.x < 0.0 && viewvec.y < 0.0 && viewvec.z >= 0.0)
	{
		sg[0]=-1;sg[1]=-1;sg[2]=1;
		vol[0]=DDAb[0]-1;vol[1]=DDAb[1]-1;vol[2]=0;
		vec_num = 3;
	}
	else if(viewvec.x < 0.0 && viewvec.y >= 0.0 && viewvec.z < 0.0)
	{
		sg[0]=-1;sg[1]=1;sg[2]=-1;
		vol[0]=DDAb[0]-1;vol[1]=0;vol[2]=DDAb[2]-1;
		vec_num = 4;
	}
	else if(viewvec.x >= 0.0 && viewvec.y < 0.0 && viewvec.z >= 0.0)
	{
		sg[0]=1;sg[1]=-1;sg[2]=1;
		vol[0]=0;vol[1]=DDAb[1]-1;vol[2]=0;
		vec_num = 5;
	}
	else if(viewvec.x < 0.0 && viewvec.y >= 0.0 && viewvec.z >= 0.0)
	{
		sg[0]=-1;sg[1]=1;sg[2]=1;
		vol[0]=DDAb[0]-1;vol[1]=0;vol[2]=0;
		vec_num = 6;
	}
	else
	{
		sg[0]=1;sg[1]=-1;sg[2]=-1;
		vol[0]=0;vol[1]=DDAb[1]-1;vol[2]=DDAb[2]-1;
		vec_num = 7;
	}



	if(deltam*llength < deltal && deltam <= deltas*llength && deltas <= deltam )
	{


		int end_s = 0;
		int sum = -deltam;
		int *m_value;
		m_value = new int[slength];
		//初期化
		for(i=0;i<slength;i++){
			m_value[i] = 0;
		}
		int d = deltas;
		int start_s = 0;



		switch(num) 
		{
		case 0:
			x = &j;y = &a;z = &i;
			break;
		case 1:
			x = &j;y = &i;z = &a;
			break;
		case 2:
			x = &i;y = &j;z = &a;
			break;
		case 3:
			x = &a;y = &j;z = &i;
			break;
		case 4:
			x = &a;y = &i;z = &j;
			break;
		default:
			x = &i;y = &a;z = &j;
			break;
		}

//最も離れているボクセル面をPに入れる
		do{
			for(i = start_s ;i<=end_s;i++){//P内の全各ボクセル面index
				int &vl = m_value[i];
				a = vl;
				for(j = 0;j < llength ; j++)//s[index]内の全各ボクセル列
				{
					Block block(vol[0]+sg[0]**x+bl.x,vol[1]+sg[1]**y+bl.y,vol[2]+sg[2]**z+bl.z,bl.level,bl.bnumx,bl.bnumy,bl.bnumz);
					testRenderFrustum(block,normalreso,backupreso,existblcount,blockQue);
					//視点から最も離れている未描画ボクセル１つを描画する。
				}
				vl++;
				if(vl == mlength)
					start_s++;
			}	
			sum += d;//△y/△z
			if(sum >0 && end_s<slength-1){//Pの先頭ボクセル面内の全ボクセルが描画済み。
				end_s++;
				sum -= deltam;//先頭ボクセル面をPから外す
			}
		}while(start_s <= slength-1);	//Pが空ではない。

		delete []m_value;



	}
	else
	{   


		int ms = deltam*deltas;//△y/△z
		int ls = deltal*deltas;
		int lm = deltal*deltam;


		int end_s = 0;
		int sum_s = -lm;//
		int start_s = 0;

		int *end_m;
		end_m = new int[slength];	
		int** l_value;
		l_value = new int*[slength];


		int* sum_m;
		sum_m = new int[slength];
		int* start_m;
		start_m = new int[slength];




		for(i = 0;i<slength;i++)
		{
			start_m[i] = 0;
			sum_m[i] = -ls;
			end_m[i] = 0;

			l_value[i] = new int [mlength];

			for(j = 0;j < mlength;j++)
				l_value[i][j] = 0;	
		}


		switch(num) 
		{
		case 0:
			x = &a;y = &j;z = &i;
			break;
		case 1:
			x = &a;y = &i;z = &j;
			break;
		case 2:
			x = &i;y = &a;z = &j;
			break;
		case 3:
			x = &j;y = &a;z = &i;
			break;
		case 4:
			x = &j;y = &i;z = &a;
			break;
		default:
			x = &i;y = &j;z = &a;
			break;
		}
//最も離れているボクセル面をPに入れる
		do
		{
			for(i = start_s;i<= end_s;i++)//P内の全各ボクセル面index
			{   
				for(j = start_m[i];j <= end_m[i]; j++)//s[index]内の全各ボクセル列
				{	
					int &vl = l_value[i][j];
					a = vl;
					Block block(vol[0]+sg[0]**x+bl.x,vol[1]+sg[1]**y+bl.y,vol[2]+sg[2]**z+bl.z,bl.level,bl.bnumx,bl.bnumy,bl.bnumz);
					testRenderFrustum(block,normalreso,backupreso,existblcount,blockQue);
				//もっとも遠いボクセルを一つ描画する。
					vl++;
				}
				sum_m[i] += ms;//ds[index]+=△y/△z
				if(sum_m[i] >0 && end_m[i]<mlength-1){//ds[index]>=1.0
					end_m[i]++;//視点から最も離れている未描画のボクセル列をS[index]の最後に加える。
					sum_m[i] -= ls;//ds[index]-=1.0;
				}
				if(l_value[i][start_m[i]] == llength )//S[index]の先頭ボクセル列内の全ボクセルが描画済み。
					start_m[i]++;//先頭ボクセル列をS[index]から外す。
			}	
			sum_s += ms;//dp+=△x/△z
			if(sum_s >0 && end_s<slength-1){
				end_s++;
				sum_s -= lm;
				sum_m[end_s] = sum_s;
			}
			if(start_m[start_s] >  mlength-1)//Pの先頭ボクセル面内の全ボクセルが描画済み。
				start_s++;//先頭ボクセル面をPから外す
		}while(start_s <= slength - 1);	//Pが空ではない。

		delete []end_m;
		for(i=0;i < slength;i++)
			delete []l_value[i];
		delete []l_value;

		delete []sum_m;
		delete []start_m;

	}


	//	cout <<"vec_num"<<vec_num<<endl;
	//	cout <<"num"<<num<<endl;

}
void DDAblockFile(Vec3 viewvec, Vec3 abview,Block bl,int* _reso,int* _exist_in_main_count,unsigned int** _blockstate,bool ****needCountMap,bool **** existInMainMap,deque<Block>* blockQue)
{// abviewとviewvecは対の関係みたいになってる。
	int DDAb[3];
	if(bl.level == NUMLEVEL-1)
	{
		DDAb[0] = INIBLX;//2
		DDAb[1] = INIBLY;//2
		DDAb[2] = INIBLZ;//2
	}
	else
	{
		DDAb[0] = 2;
		DDAb[1] = 2;
		DDAb[2] = 2;
	}

	int deltal;
	int deltam;                  
	int deltas;                  
	int num;
	int llength;
	int mlength;
	int slength;
	//abview.print("abview");//abviewの成分は0か1
	if(abview.x >= abview.y && abview.y >= abview.z)
	{
		deltam = (int)(DDAb[2]*abview.y/abview.z);
		deltas =  DDAb[2];
		deltal = (int)(DDAb[2]*abview.x/abview.z);
		llength = DDAb[0];
		mlength = DDAb[1];
		slength = DDAb[2];
		num = 0;
	}
	else if(abview.x >= abview.z && abview.z >= abview.y)
	{

		deltam = (int)(DDAb[1]*abview.z/abview.y);
		deltas = DDAb[1];
		deltal = (int)(DDAb[1]*abview.x/abview.y);
		llength = DDAb[0];
		mlength = DDAb[2];
		slength = DDAb[1];
		num = 1;

	}
	else if(abview.y >= abview.z && abview.z >= abview.x)
	{


		deltam = (int)(DDAb[0]*abview.z/abview.x);
		deltas = DDAb[0];
		deltal = (int)(DDAb[0]*abview.y/abview.x);
		llength = DDAb[1];
		mlength = DDAb[2];
		slength = DDAb[0];
		num = 2;
	}
	else if(abview.y >= abview.x && abview.x >= abview.z)
	{

		deltam = (int)(DDAb[2]*abview.x/abview.z);
		deltas = DDAb[2];
		deltal = (int)(DDAb[2]*abview.y/abview.z);
		llength = DDAb[1];
		mlength = DDAb[0];
		slength = DDAb[2];
		num = 3;
	}
	else if(abview.z >= abview.x && abview.x >= abview.y)
	{

		deltam = (int)(DDAb[1]*abview.x/abview.y);
		deltas = DDAb[1];
		deltal = (int)(DDAb[1]*abview.z/abview.y);
		llength = DDAb[2];
		mlength = DDAb[0];
		slength = DDAb[1];
		num = 4;
	}
	else
	{

		deltam = (int)(DDAb[0]*abview.y/abview.x);
		deltas = DDAb[0];
		deltal = (int)(DDAb[0]*abview.z/abview.x);
		llength = DDAb[2];
		mlength = DDAb[1];
		slength = DDAb[0];
		num =5;
	}




	int a;
	int* x;
	int* y;
	int* z;
	int i,j;
	int count = 0;
	double temp_sum= 0.0;

	//QueryPerformanceCounter(&temp1);
	//					QueryPerformanceCounter(&temp2);
	//			QueryPerformanceFrequency(&freq);					
	//			temp_sum += (double)(temp2.QuadPart-temp1.QuadPart)/(double)freq.QuadPart;
	//		count++;


	int sg[3];
	int vol[3];
	int vec_num;
	//viewvec.print("viewvec");//viewvecの成分は、0か1か-1 8通りの組み合わせ。
	if(viewvec.x >= 0.0 && viewvec.y >= 0.0 && viewvec.z >= 0.0)
	{
		sg[0]=1;sg[1]=1;sg[2]=1;
		vol[0]=0;vol[1]=0;vol[2]=0;
		vec_num = 0;
	}
	else if(viewvec.x < 0.0 && viewvec.y < 0.0 && viewvec.z < 0.0)
	{
		sg[0]=-1;sg[1]=-1;sg[2]=-1;
		vol[0]=DDAb[0]-1;vol[1]=DDAb[1]-1;vol[2]=DDAb[2]-1;
		vec_num = 1;
	}
	else if(viewvec.x >= 0.0 && viewvec.y >= 0.0 && viewvec.z < 0.0)
	{
		sg[0]=1;sg[1]=1;sg[2]=-1;
		vol[0]=0;vol[1]=0;vol[2]=DDAb[2]-1;
		vec_num = 2;
	}
	else if(viewvec.x < 0.0 && viewvec.y < 0.0 && viewvec.z >= 0.0)
	{
		sg[0]=-1;sg[1]=-1;sg[2]=1;
		vol[0]=DDAb[0]-1;vol[1]=DDAb[1]-1;vol[2]=0;
		vec_num = 3;
	}
	else if(viewvec.x < 0.0 && viewvec.y >= 0.0 && viewvec.z < 0.0)
	{
		sg[0]=-1;sg[1]=1;sg[2]=-1;
		vol[0]=DDAb[0]-1;vol[1]=0;vol[2]=DDAb[2]-1;
		vec_num = 4;
	}
	else if(viewvec.x >= 0.0 && viewvec.y < 0.0 && viewvec.z >= 0.0)
	{
		sg[0]=1;sg[1]=-1;sg[2]=1;
		vol[0]=0;vol[1]=DDAb[1]-1;vol[2]=0;
		vec_num = 5;
	}
	else if(viewvec.x < 0.0 && viewvec.y >= 0.0 && viewvec.z >= 0.0)
	{
		sg[0]=-1;sg[1]=1;sg[2]=1;
		vol[0]=DDAb[0]-1;vol[1]=0;vol[2]=0;
		vec_num = 6;
	}
	else
	{
		sg[0]=1;sg[1]=-1;sg[2]=-1;
		vol[0]=0;vol[1]=DDAb[1]-1;vol[2]=DDAb[2]-1;
		vec_num = 7;
	}



	if(deltam*llength < deltal && deltam <= deltas*llength && deltas <= deltam )
	{


		int end_s = 0;
		int sum = -deltam;
		int *m_value;
		m_value = new int[slength];
		//初期化
		for(i=0;i<slength;i++){
			m_value[i] = 0;
		}
		int d = deltas;
		int start_s = 0;



		switch(num) 
		{
		case 0:
			x = &j;y = &a;z = &i;
			break;
		case 1:
			x = &j;y = &i;z = &a;
			break;
		case 2:
			x = &i;y = &j;z = &a;
			break;
		case 3:
			x = &a;y = &j;z = &i;
			break;
		case 4:
			x = &a;y = &i;z = &j;
			break;
		default:
			x = &i;y = &a;z = &j;
			break;
		}

//最も離れているボクセル面をPに入れる
		do{
			for(i = start_s ;i<=end_s;i++){//P内の全各ボクセル面index
				int &vl = m_value[i];
				a = vl;
				for(j = 0;j < llength ; j++)//s[index]内の全各ボクセル列
				{
					Block block(vol[0]+sg[0]**x+bl.x,vol[1]+sg[1]**y+bl.y,vol[2]+sg[2]**z+bl.z,bl.level,bl.bnumx,bl.bnumy,bl.bnumz);
					//testRenderFrustum(block,normalreso,backupreso,existblcount,blockQue);
					testLoadFrustum(block,_reso,_exist_in_main_count,_blockstate,needCountMap,existInMainMap,blockQue);
					

					//視点から最も離れている未描画ボクセル１つを描画する。
				}
				vl++;
				if(vl == mlength)
					start_s++;
			}	
			sum += d;//△y/△z
			if(sum >0 && end_s<slength-1){//Pの先頭ボクセル面内の全ボクセルが描画済み。
				end_s++;
				sum -= deltam;//先頭ボクセル面をPから外す
			}
		}while(start_s <= slength-1);	//Pが空ではない。

		delete []m_value;



	}
	else
	{   


		int ms = deltam*deltas;//△y/△z
		int ls = deltal*deltas;
		int lm = deltal*deltam;


		int end_s = 0;
		int sum_s = -lm;//
		int start_s = 0;

		int *end_m;
		end_m = new int[slength];	
		int** l_value;
		l_value = new int*[slength];


		int* sum_m;
		sum_m = new int[slength];
		int* start_m;
		start_m = new int[slength];




		for(i = 0;i<slength;i++)
		{
			start_m[i] = 0;
			sum_m[i] = -ls;
			end_m[i] = 0;

			l_value[i] = new int [mlength];

			for(j = 0;j < mlength;j++)
				l_value[i][j] = 0;	
		}


		switch(num) 
		{
		case 0:
			x = &a;y = &j;z = &i;
			break;
		case 1:
			x = &a;y = &i;z = &j;
			break;
		case 2:
			x = &i;y = &a;z = &j;
			break;
		case 3:
			x = &j;y = &a;z = &i;
			break;
		case 4:
			x = &j;y = &i;z = &a;
			break;
		default:
			x = &i;y = &j;z = &a;
			break;
		}
//最も離れているボクセル面をPに入れる
		do
		{
			for(i = start_s;i<= end_s;i++)//P内の全各ボクセル面index
			{   
				for(j = start_m[i];j <= end_m[i]; j++)//s[index]内の全各ボクセル列
				{	
					int &vl = l_value[i][j];
					a = vl;
					Block block(vol[0]+sg[0]**x+bl.x,vol[1]+sg[1]**y+bl.y,vol[2]+sg[2]**z+bl.z,bl.level,bl.bnumx,bl.bnumy,bl.bnumz);
					//testRenderFrustum(block,normalreso,backupreso,existblcount,blockQue);
					testLoadFrustum(block, _reso,_exist_in_main_count,_blockstate,needCountMap, existInMainMap,blockQue);
				//もっとも遠いボクセルを一つ描画する。
					vl++;
				}
				sum_m[i] += ms;//ds[index]+=△y/△z
				if(sum_m[i] >0 && end_m[i]<mlength-1){//ds[index]>=1.0
					end_m[i]++;//視点から最も離れている未描画のボクセル列をS[index]の最後に加える。
					sum_m[i] -= ls;//ds[index]-=1.0;
				}
				if(l_value[i][start_m[i]] == llength )//S[index]の先頭ボクセル列内の全ボクセルが描画済み。
					start_m[i]++;//先頭ボクセル列をS[index]から外す。
			}	
			sum_s += ms;//dp+=△x/△z
			if(sum_s >0 && end_s<slength-1){
				end_s++;
				sum_s -= lm;
				sum_m[end_s] = sum_s;
			}
			if(start_m[start_s] >  mlength-1)//Pの先頭ボクセル面内の全ボクセルが描画済み。
				start_s++;//先頭ボクセル面をPから外す
		}while(start_s <= slength - 1);	//Pが空ではない。

		delete []end_m;
		for(i=0;i < slength;i++)
			delete []l_value[i];
		delete []l_value;

		delete []sum_m;
		delete []start_m;

	}


	//	cout <<"vec_num"<<vec_num<<endl;
	//	cout <<"num"<<num<<endl;

}
void renderBitmapString(float x, float y, void *font,char *string)//画面に文字を出力
{

	char *c;
	// set position to start drawing fonts
	glRasterPos2f(x, y);
	// loop all the characters in the string
	for (c=string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}
void setOrthographicProjection() {

	// switch to projection mode
	glMatrixMode(GL_PROJECTION);
	// save previous matrix which contains the 
	//settings for the perspective projection
	glPushMatrix();
	// reset matrix
	glLoadIdentity();
	// set a 2D orthographic projection
	gluOrtho2D(0, winwidth, 0, winheight);
	// invert the y axis, down is positive
	glScalef(1, -1, 1);
	// mover the origin from the bottom left corner
	// to the upper left corner
	glTranslatef(0, -(GLfloat)winheight, 0);
	glMatrixMode(GL_MODELVIEW);
}
void resetPerspectiveProjection() {
	// set the current matrix to GL_PROJECTION
	glMatrixMode(GL_PROJECTION);
	// restore previous settings
	glPopMatrix();
	// get back to GL_MODELVIEW matrix
	glMatrixMode(GL_MODELVIEW);
}
void renderInfo()
{
	if(infoflag)
	{

		setOrthographicProjection();
		glPushMatrix();
		glLoadIdentity();


		glColor4f(0.0,0.0,0.0,0.5);
		glBegin(GL_QUADS);  
		glVertex2f(0.0,0.0);
		glVertex2f(320.0,0.0);
		glVertex2f(320.0,(GLfloat)winheight);
		glVertex2f(0.0,(GLfloat)winheight);
		glEnd();




		float averaycasttime = raycasttime/(float)rbn;
		blockrendertime.str("");
		blockrendertime<<"avarage render each block time [msec]:"<<averaycasttime;

		//cout<<"render time:"<<raycasttime<<endl;


		float aveocclutiontime = occlutiontime/(float)renderBlockNum;//←こっちが正しいと思う((float)normalnbn+obn);
		//オクルージョンクエリーにかかる時間　normalnbn=//描画に必要な現解像度ブロックの数
		oquerytime.str("");
		oquerytime<<"occlution query time [msec]:"<<aveocclutiontime;

		//cout<<"average occlution time:"<<occlutiontime<<endl;

		float avetexprocesstime = texprocesstime/(float)rbn;
		//CPUからGPUへのテクスチャ転送時間．
		tptime.str("");
		tptime<<"avarage texture process time [msec]:"<<avetexprocesstime;
		
		//cout<<"texture time:"<<texprocesstime<<endl;

		float avetexrequesttime = texrequesttime/(float)rbn;
		//loadMainToTexにかかる時間
		trtime.str("");
		trtime<<"avarage texture request time [msec]:"<<avetexrequesttime;

		//cout<<"texture request time:"<< texrequesttime<<endl;

		

		//
		// meter
		//
		float meterxorigin = 15.0;
		float meterlength = 285.0;
		float meteryorigin = 530.0;
		float meterwidth = 10.0;


		//比率計算
		float rcresio = raycasttime;
		float ocresio = occlutiontime;
		float tpresio = texprocesstime;
		float trresio = texrequesttime;
		

		float sumresio = rcresio + ocresio + tpresio + trresio;

		rcresio = rcresio/sumresio;
		ocresio = ocresio/sumresio;
		tpresio = tpresio/sumresio;
		trresio = trresio/sumresio;
		


		//raycasting time
		glColor(Color::MediumSeaGreen);
		float rcend = meterxorigin + rcresio*meterlength;
		glBegin(GL_QUADS);  
		glVertex2f(meterxorigin,meteryorigin);
		glVertex2f(rcend,meteryorigin);
		glVertex2f(rcend,meteryorigin + meterwidth);
		glVertex2f(meterxorigin,meteryorigin + meterwidth);
		glEnd();

		//occlution culling time
		glColor(Color::MidnightBlue);
		float ocorigin = rcend;
		float ocend = ocorigin + ocresio*meterlength;
		glBegin(GL_QUADS);  
		glVertex2f(ocorigin,meteryorigin);
		glVertex2f(ocend,meteryorigin);
		glVertex2f(ocend,meteryorigin + meterwidth);
		glVertex2f(ocorigin,meteryorigin + meterwidth);
		glEnd();


		//texture process time
		glColor(Color::Pink);
		float tporigin =  ocend;
		float tpend = tporigin + tpresio*meterlength;
		glBegin(GL_QUADS);  
		glVertex2f(tporigin,meteryorigin);
		glVertex2f(tpend,meteryorigin);
		glVertex2f(tpend,meteryorigin + meterwidth);
		glVertex2f(tporigin,meteryorigin + meterwidth);
		glEnd();

		//texture request time
		glColor(Color::Silver);
		float trorigin =  tpend;
		float trend = trorigin + trresio*meterlength;
		glBegin(GL_QUADS);  
		glVertex2f(trorigin,meteryorigin);
		glVertex2f(trend,meteryorigin);
		glVertex2f(trend,meteryorigin + meterwidth);
		glVertex2f(trorigin,meteryorigin + meterwidth);
		glEnd();



		glColor(Color::MediumPurple);
		renderBitmapString(15,15,font,(char*)floadtime.str().c_str()); 
		renderBitmapString(15,35,font,(char*)texloadtime.str().c_str()); 
		glColor(Color::MediumSpringGreen);
		renderBitmapString(15,55,font,(char*)blockrendertime.str().c_str()); 
		renderBitmapString(15,75,font,(char*)oquerytime.str().c_str());
		renderBitmapString(15,95,font,(char*)tptime.str().c_str());
		renderBitmapString(15,115,font,(char*)trtime.str().c_str());
		renderBitmapString(15,135,font,(char*)mrtime.str().c_str());
		glColor(Color::Plum);
		renderBitmapString(15,195,font,(char*)fps.str().c_str());
		glColor(Color::AliceBlue);
		renderBitmapString(15,215,font,(char*)normalnbnstr.str().c_str());
		renderBitmapString(15,235,font,(char*)normalebnstr.str().c_str());
		renderBitmapString(15,255,font,(char*)backnbnstr.str().c_str());
		renderBitmapString(15,275,font,(char*)backebnstr.str().c_str());
		renderBitmapString(15,295,font,(char*)rendernbnstr.str().c_str());
		renderBitmapString(15,315,font,(char*)renderebnstr.str().c_str());
		renderBitmapString(15,335,font,(char*)normalresostr.str().c_str());
		renderBitmapString(15,355,font,(char*)backresostr.str().c_str());
		renderBitmapString(15,375,font,(char*)renderresostr.str().c_str());
		
		glColor(Color::MediumSeaGreen);
		renderBitmapString(15,395,font,(char*)occlutionnum.str().c_str());
		renderBitmapString(15,415,font,(char*)renderblocknum.str().c_str());
		renderBitmapString(15,435,font,(char*)loadtexblocknum.str().c_str());
		renderBitmapString(15,455,font,(char*)maxloadtexblocknum.str().c_str());
		renderBitmapString(15,475,font,(char*)maxloadmainblocknum.str().c_str());
		renderBitmapString(15,495,font,(char*)filethreadtime.str().c_str());
		glColor(Color::PeachPuff);
		renderBitmapString(15,515,font,(char*)widthstring.str().c_str());
		renderBitmapString(15,535,font,(char*)heightstring.str().c_str());
		glPopMatrix();
		resetPerspectiveProjection();


		////メーター初期化
		//raycasttime = 0.0;
		//occlutiontime  = 0.0;
		//texprocesstime = 0.0;
		//texrequesttime = 0.0;
		//memrequesttime = 0.0;
	}

}
void setFrontSlice()
{
	Vec3 view_z(0.0,0.0,1.0);
	view_z = inv_modelView.vec_maltiply3(&view_z);


	//スライスの頂点決定
	int frontIdx;
	float vecView[3] = {(float)view_z.x,(float)view_z.y,(float)view_z.z};
	if(vecView[0] >= 0 && vecView[1] >= 0 && vecView[2] >= 0)
		frontIdx = 0;
	else if(vecView[0] >= 0 && vecView[1] >= 0 && vecView[2] < 0)
		frontIdx = 1;
	else if(vecView[0] >= 0 && vecView[1] < 0 && vecView[2] >= 0)
		frontIdx = 2;
	else if(vecView[0] < 0 && vecView[1] >= 0 && vecView[2] >= 0)
		frontIdx = 3;
	else if(vecView[0] < 0 && vecView[1] >= 0 && vecView[2] < 0)
		frontIdx = 4;
	else if(vecView[0] >= 0 && vecView[1] < 0 && vecView[2] < 0)
		frontIdx = 5;
	else if(vecView[0] < 0 && vecView[1] < 0 && vecView[2] >= 0)
		frontIdx = 6;
	else 
		frontIdx = 7;
	Cg.SetParameter(Cg.frontIdxParam,(float)frontIdx);
	Cg.SetParameter(Cg.vecViewParam ,vecView );
}
void renderSubWindow(){
	if(subwindowflag)
	{
		unsigned int** rb;//blockstate
		FrustumG fs;
		if(frustumstate == FrustumG::normal)//この視錐台はノーマル
		{
			rb = Block::nRequestBlock;
			fs = frustum;
		}
		else if(frustumstate == FrustumG::back)//この視錐台はバックアップ用
		{
			rb = Block::bRequestBlock;
			fs = frustum;
		}
		else//この視錐台はレンダリング用
		{	
			rb = Block::rRequestBlock;//ピンク色
			fs = renderfrustum;
		}
		if(showGridFlag){
		//ブロックグリッド線描画(メイン用)
		for(int i=0; i< INIBLX;i++)
		{
			for(int j=0;j < INIBLY;j++)
			{
				for(int k=0;k< INIBLZ;k++){//rb=blockstate
					Block::renderSubblock(rb,Block(i,j,k,NUMLEVEL-1,INIBLX,INIBLY,INIBLZ),0.15);
				}
			}
		}
		}

		if(winwidth >= winheight)
			glViewport(winwidth - winheight/2,0, winheight/2, winheight/2);
		else
			glViewport(winwidth - winwidth/2,0, winwidth/2, winwidth/2);



		//サブ画面背景作成
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glTranslatef(0.0,0.0,-7.0);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(-1.0,1.0,-1.0,1.0,0.25,20.0);

		glColor4f(0.0,0.0,0.0,0.5);
		glBegin(GL_QUADS);  
		glVertex2f(-1.0,-1.0);
		glVertex2f(1.0,-1.0);
		glVertex2f(1.0,1.0);
		glVertex2f(-1.0,1.0);
		glEnd();

		//サブ画面描画ブロック表示
		glLoadIdentity();
		glOrtho(-eyeDis*1.5,eyeDis*1.5,-eyeDis*1.5,eyeDis*1.5,0.25,20.0);


		glMatrixMode(GL_MODELVIEW);
		glRotatef(45.0f,1.0f,-1.0f,-0.29f);

		//ブロック描画
		for(int i=0; i< INIBLX;i++)
		{
			for(int j=0;j < INIBLY;j++)
			{
				for(int k=0;k< INIBLZ;k++)//rb=blockstate
					Block::renderSubblock(rb,Block(i,j,k,NUMLEVEL-1,INIBLX,INIBLY,INIBLZ),0.15);
			}
		}

		//画角描画
		glColor4f(RED[0],RED[1],RED[2],0.25);
		fs.drawLines();//視錐台の線を赤いので描画。
		glColor4f(0.42,0.84,0.14,0.25);
		frustum.drawLines();//	ファイル用視錐台を緑で描画
		glColor4f(1.0,1.0,1.0,1.0);
		

		double mM[16];
		mM[0]=inv_modelView.getXX();mM[4]=inv_modelView.getXY();mM[8]=inv_modelView.getXZ() ;mM[12]=inv_modelView.getXW();
		mM[1]=inv_modelView.getYX();mM[5]=inv_modelView.getYY();mM[9]=inv_modelView.getYZ() ;mM[13]=inv_modelView.getYW();
		mM[2]=inv_modelView.getZX();mM[6]=inv_modelView.getZY();mM[10]=inv_modelView.getZZ();mM[14]=inv_modelView.getZW();
		mM[3]=inv_modelView.getWX();mM[7]=inv_modelView.getWY();mM[11]=inv_modelView.getWZ();mM[15]=inv_modelView.getWW();

		glMultMatrixd(mM);


		//視点描画
		glRotatef(180.0,0.0,1.0,0.0);
		glTranslatef(0.0,0.0,-0.5);
		glColor3f(0.0,1.0,0.0);
		glutSolidCone(0.05,0.5,5,5);


		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();


		glViewport(0, 0, winwidth, winheight);



	}

}
void testRenderFrustum(Block block,int* Preso,int* Sreso,int* NeedInTex_ExistInMainBlcount,deque<Block>* blockQue)//DDAブロックメソッドの中で使われる
{//Preso=normalreso,Sreso=backupreso
	//blockにデータが入っているか判定
	int id = file.getIndexblock(block);
	block.setBlockState(Block::notexist,Block::rRequestBlock);
	if(id != 0)//そのブロックにちょっとでもデータが入ってれば
	{
		//もしビューボリュームにblockが入っていれば
		Vec3 corner;
		corner.x = (2.0f*block.x-(float)block.bnumx)*Block::brX*Block::iniX/(float)block.bnumx;
		corner.y = (2.0f*block.y-(float)block.bnumy)*Block::brY*Block::iniY/(float)block.bnumy;
		corner.z = (2.0f*block.z-(float)block.bnumz)*Block::brZ*Block::iniZ/(float)block.bnumz;


		AABox abox(corner,2.0f*Block::brX*Block::iniX/block.bnumx,2.0f*Block::brY*Block::iniY/block.bnumy,2.0f*Block::brZ*Block::iniZ/block.bnumz);
		if(renderfrustum.boxInFrustum(abox) != FrustumG::OUTSIDE)//視錐台の中に入ってたら
		{
			if(block.resoChange(modelMatrix,projMatrix,winwidth,winheight) &&blockQue->size()<=MAXTEXNUM)//1ボクセルの大きさが、1ピクセルよりも大きいかどうかチェック。大きかったらtrue
			{
				ProcessNextResoRender(block,Preso,Sreso,NeedInTex_ExistInMainBlcount,blockQue);//実際に1段階高解像度のノードを探索
			}
			else{  //1ボクセルの大きさが、1ピクセルよりも小さい場合　or すべてが最適解像度ブロックになったら
				//何段階か上	
				Block Pblock = block.getMultiLowblock(*Preso);
				if(!render_normal_blocks[Pblock.level][Pblock.x][Pblock.y][Pblock.z])
				{	
					blockQue->push_back(Pblock);//Pblockと同じ値の要素を両端キューの末尾に追加する。
					render_normal_blocks[Pblock.level][Pblock.x][Pblock.y][Pblock.z] = true;
					render_normal_num++;//miffy added
				}

				
				if(!file.texBlockExist(Pblock))//テクスチャメモリになく
				{
					if(file.blockExist(Pblock))//メインメモリにあるブロックを探す
					{
						if(!r_existInMainMemNotInTexMem[Pblock.level][Pblock.x][Pblock.y][Pblock.z])
						{
							*NeedInTex_ExistInMainBlcount += 1;
							r_existInMainMemNotInTexMem[Pblock.level][Pblock.x][Pblock.y][Pblock.z] = true;
						}
					}
					else if(*Preso != *Sreso)//テクスチャメモリにもメインメモリにもないブロックは低解像度にする
					{
						Block Sblock = block.getMultiLowblock(*Sreso);
						if(file.blockExist(Sblock) && !file.texBlockExist(Sblock))//低解像度にしたけど、テクスチャメモリになくてメインメモリにあるか
						{
							if(!r_existInMainMemNotInTexMem[Sblock.level][Sblock.x][Sblock.y][Sblock.z])
							{
								*NeedInTex_ExistInMainBlcount += 1;
								r_existInMainMemNotInTexMem[Sblock.level][Sblock.x][Sblock.y][Sblock.z] = true;
							}
						}else if(file.blockExist(Sblock)){Sblock.renderBlockLines(1.0,0.0,1.0,1.0);//ピンク（バックアップ解像度がメインメモリにすらないピンチなブロック）
						pinchBlockCount++;
						//if(Sblock.level==4){printf("level4が消えた！！\n");}
						//Sblock.printBlockInfo("Sメインメモリにすらないピンチブロック(バックアップ)S",0);
						/*Sblock.printBlockInfo("SメインメモリにすらないピンチブロックS",0);*/
						}
					}
				}
			}//1ボクセルが1ピクセルよりも大きいかどうか判定
		}//視錐台の中に入ってるかどうか判定	
		else
		{//クリップアウト 
			block.setBlockState(Block::frameout,Block::rRequestBlock);
		}
	}//そのブロックにちょっとでもデータが入ってるかどうか
}
//読込み視錐台用 1フレーム先回りして読み込み要求発行。
void testLoadFrustum(Block block,int* octreeReso,int* existblcount,unsigned int** renderblock,bool **** needCountflag,bool **** existCountflag, deque<Block>* blockQue)
{
	
	int id = file.getIndexblock(block);//idは0か1の値 データが存在してたら１なんだと思うが。
	block.setBlockState(Block::notexist,renderblock);//配列renderblockにブロックステートを記録
	if(id != 0)//blockにデータが入っているか判定
	{
		//もしビューボリュームにblockが入っていれば
		Vec3 corner;//cornerの成分は0か-1だ。
		corner.x = (2.0f*block.x-block.bnumx)*Block::brX*Block::iniX/block.bnumx;
		corner.y = (2.0f*block.y-block.bnumy)*Block::brY*Block::iniY/block.bnumy;
		corner.z = (2.0f*block.z-block.bnumz)*Block::brZ*Block::iniZ/block.bnumz;
//corner.print("corner");
		AABox abox(corner,2.0f*Block::brX*Block::iniX/block.bnumx,2.0f*Block::brY*Block::iniY/block.bnumy,2.0f*Block::brZ*Block::iniZ/block.bnumz);//ブロックの情報をAxis Aligned Boxで表現している。AABoxのx,y,zはブロックの辺の長さ。
		if(frustum.boxInFrustum(abox) != FrustumG::OUTSIDE)//このブロックが視錐台の内側だったら。
		{
			if(block.resoChange(modelMatrix,projMatrix,winwidth,winheight) && blockQue->size()<=MAXTEXNUM)//ボクセルのスクリーン上での大きさが1ピクセルの大きさよりも大きい場合にのに、実際に一段階高解像度のノードを探索する。
			{//
				ProcessNextResoLoad(block,octreeReso,existblcount,renderblock,needCountflag,existCountflag,blockQue);
			}//↑ここでちょっと値を変えてまたtestLoadFrustumが呼び出される。blockstateは、nextresoに設定される。実際に1段階高解像度のノードの探索。
			else
			{//最適解像度ブロックになったら
				//何段階か上	
				
				Block mlowblock = block.getMultiLowblock(*octreeReso);//現解像度ブロックに対して1段階低解像度なブロックを予めバックアップとして主メモリに読み込んでおく。描画に必要な現解像度ブロックが主メモリに読み込まれていない場合には、このブロックを代わりに用いることで、結果が欠けてしまう事態を防ぐ。
				//octreeResoは詳細度レベルを表す。関数の引数の時点でoctreeResoはポインタだからintの値に直すために*をつけてる。
				//mlowblockはバックアップブロックということになる？
				if(!needCountflag[mlowblock.level][mlowblock.x][mlowblock.y][mlowblock.z])
				{
					blockQue->push_back(mlowblock);//mlowblockと同じ値の要素を両端キューの末尾に追加する。
					needCountflag[mlowblock.level][mlowblock.x][mlowblock.y][mlowblock.z] = true;
				}

				if(file.blockExist(mlowblock))//main memory exist or not
				{
					Block::blockState bstate = block.getLowState(mlowblock);//blockとmlowblockのレベルの差によって決まる
					block.setBlockState(bstate,renderblock);
					
					if(!existCountflag[mlowblock.level][mlowblock.x][mlowblock.y][mlowblock.z])
					{
						*existblcount += 1;
						existCountflag[mlowblock.level][mlowblock.x][mlowblock.y][mlowblock.z] = true;
					}
				}
				else
					block.setBlockState(Block::waitblock,renderblock);
			}
		}//視錐台の外側だったら。
		else{
			block.setBlockState(Block::frameout,renderblock);}
	}
}
void requestNormalFrustum(Vec3 viewvec,Vec3 abview,Block block)//file threadで使う
{	nowprocessingflag=true;
	normalreso = -1;//LowBlockを作る回数に関わる変数。
	int counter=0;
	do//ここが重要！！　nRequestQueue.size() >MAXTEXNUMになったらやめる。
	{
		normalreso++;
		normalebn = 0;//exist blocks of normal reso
		nRequestQueue.clear();//両端キューからすべての要素を削除する。
		initCountflag(normalreso,n_needInMain,n_existInMain);
//level4の計算してるっぽい。初期状態を作ってる。
		
		DDAblockFile(viewvec,abview,block,&normalreso,&normalebn,Block::nRequestBlock,n_needInMain,n_existInMain,&nRequestQueue);
		//testLoadFrustum(block,&normalreso,&normalebn,Block::nRequestBlock,n_needInMain,n_existInMain,&nRequestQueue);
		
		if(counter==0){idealBlockNum=nRequestQueue.size();}
		counter++;
		if(nRequestQueue.size()>MAXTEXNUM){//多すぎたらテクスチャ圧縮処理
			printf("miffyの圧縮処理発動\n");
			int difference=nRequestQueue.size()-MAXTEXNUM;
			int sum=0;
			while(sum>=difference){
			Block bl=nRequestQueue.back();
			sum+=file.getTexCompressInfo(bl);
			nRequestQueue.pop_back();//仲間も外さなきゃいかん
			nRequestQueue.push_back(bl.returnParent());
			}
		}
	}while(normalreso < NUMLEVEL-1 && nRequestQueue.size() >MAXTEXNUM);
	normalnbn = nRequestQueue.size();//描画に必要な現解像度ブロックの数
	normalnbnstr.str("");
	normalnbnstr<<"need blocks of normal reso:(int this scene)"<<normalnbn<<"/"<<MAXBNUM<<"(MAXBNUM)";//描画に必要な現解像度ブロックの数
	normalebnstr.str("");
	normalebnstr<<"exist in Main blocks of normal reso:"<<normalebn<<"/"<<MAXBNUM<<"(MAXBNUM)";

	normalresostr.str("");
	normalresostr<<"normal reso level:"<<normalreso;

	while(!nRequestQueue.empty())//呼び出し先の両端キューが空の場合はtrueを返し、そうでない場合はfalseを返す。
	{/*(Block)nRequestQueue.front()がまだreqqueに入れられてないなら空いてる場所orreqqueが満杯なら優先度の一番低いところに入れる。(Block)nRequestQueue.front()がもうreqque、またはメインメモリに転送済みにあるなら優先度を更新する*/
		file.mainBlockRequest(modelMatrix,projMatrix,(Block)nRequestQueue.front(),frustum,FrustumG::normal);
		nRequestQueue.pop_front();//両端キューの最初の要素を削除する。
	}

	//論文のp35 (file.getThreadTime()=ファイルスレッドのwhile1ループにかかる時間
	BACKUPTHRE = (int)((file.getThreadTime()/file.getMainLoadTime())*HDMAINALPHA);
	//BACKUPTHRE = (int)((totalrender.totalrendertime/file.getMainLoadTime())*HDMAINALPHA);
	//論文通りだとこっちが正しいと思うが。。。
	//BACKUPTHRE=1フレームあたりに主メモリに読み込むことが出来るブロックの最大数
	if(BACKUPTHRE == 0)
		BACKUPTHRE = 1;
	/*バックアップブロックの解像度調整*/
	backupreso = normalreso;//論文p34
	/*unsigned int*/ BBLoadMargin = MAXBNUM - normalnbn;//normalnbn=描画に必要な現解像度ブロックの数
	 if(normalnbn>=MAXBNUM){BBLoadMargin=0;}
	//BBLoadMargin=バックアップブロックを読み込んでもよい数
	
	do
	{
		backupreso++;//0-4の値をとるみたい
		backebn = 0;
		bRequestQueue.clear();//両端キューからすべての要素を削除する。
		initCountflag(backupreso,b_needInMain,b_existInMain);
		DDAblockFile(viewvec,abview,block,&backupreso,&backebn,Block::bRequestBlock,b_needInMain,b_existInMain,&bRequestQueue);
			
		
	}while(backupreso < NUMLEVEL-1 && (bRequestQueue.size() > BBLoadMargin || (bRequestQueue.size()-backebn) > BACKUPTHRE));
	
	//論文35ページあたり                               式４．４                  　式4.6（主メモリに読み込まなくてはいけないバックアップブロックの数＜1フレーム内にloadFile()してもよいブロックの個数）
	//なぜ　BACKUPTHRE　を小さくすると、「解像度変更後のバックアップブロックの読み込み量を抑えることが出来る」のか？
	// printf("backupreso[%d],bRequestQue.size(%d),backebn[%d],BBLoadMArgin[%d],BACKUPTHRE[%d]\n",backupreso,bRequestQueue.size(),backebn,BBLoadMargin,BACKUPTHRE);

	backnbnstr.str("");
	backnbnstr<<"need in Main blocks of backup reso:"<<bRequestQueue.size()<<"/"<<MAXBNUM;
	backnbn = bRequestQueue.size();
	
	backebnstr.str("");
	backebnstr<<"exist in Main blocks of backup reso:"<<backebn<<"/"<<MAXBNUM;

	backresostr.str("");
	backresostr<<"backup reso level:"<<backupreso;

	maxloadmainblocknum.str("");
	maxloadmainblocknum<<"max number of main load blocks:per 1frame[BACKUTHRE]"<<BACKUPTHRE;//filethreadのwhile1ループあたりに読み込めるブロック数の最大数

	filethreadtime.str("");
	filethreadtime<<"file thead time:(1 while)"<<file.getThreadTime();
	
	while(!bRequestQueue.empty())
	{/*bRequestQueue.front()がまだreqqueに入れられてないなら空いてる場所orreqqueが満杯なら優先度の一番低いところに入れる。bRequestQueue.front()がもうreqque、またはメインメモリに転送済みにあるなら優先度を更新する*/
		file.mainBlockRequest(modelMatrix,projMatrix,(Block)bRequestQueue.front(),frustum,FrustumG::back);
		bRequestQueue.pop_front();
	}
	
}
void renderFrustum(Vec3 viewvec, Vec3 abview,Block block)//これがなくなると、ボリュームとブロックの線がなくなる。
{	
	bltime_perframe=0;
	bltime_sum=0;
	blcount=0;
	file.countTexLRU();//1フレーム終わるごとに優先度スコアを５ビット下げる

	renderblockcount = 0;//レンダリングしてるブロックの数をカウント初期化
	loadtexblockcount = 0;

	pinchBlockCount=0;
	
	static int count = 0;
	static float temp_num2 = 0.0;
	static float temp_occnum = 0.0;
	static float temp_rennum = 0.0;
	static int ltncount = 0;
	bool mflag = false;



	if(infoflag)
	{	
		//時間測定用．ないほうが速いかも！！
		//glFinish();
		//QueryPerformanceCounter(&temp1);
		//QueryPerformanceFrequency(&freq);
		mflag = true;
	}


	//最大テクスチャロード数計算
	float hopeframeLimitTime = 1000.0f/TEXLOADPARAMETER;
	if(totalrender.flag&& file.getTexLoadflag())
		MAXTEXLOAD = (int)((hopeframeLimitTime - totalrender.totalrendertime)/file.getTexLoadTime()) + ltn;//ltn=loadtexblockcount(1フレーム内でテクスチャロードしたブロックの数)
	else
		MAXTEXLOAD = MINTEXLOAD;//MINTEXLOAD=1 １回のフレームでテクスチャロードできる最少保障数
	
	if(MAXTEXLOAD < MINTEXLOAD)//MAXTEXLOADが１より少なかったら、
		MAXTEXLOAD = MINTEXLOAD;//MAXTEXLOADは１にする

	occblockcount = 0;
	maxRenderNum=(int)(hopeframeLimitTime/(raycasttime/(float)rbn+occlutiontime/(float)(normalnbn+obn)));
	//1フレーム内にレンダリングしてもよいブロック数
	
	//通常解像度octree追跡
	int renderNormalreso = normalreso;
	int renderBackreso = backupreso;
	int normalNeedInTex_ExistInMainBn = 0;
	rnRequestQueue.clear();
	render_normal_num=0;
	initCountflag(renderNormalreso,render_normal_blocks,r_existInMainMemNotInTexMem);//テクスチャメモリになくてメインメモリにあるブロック表がすべてfalseに初期化される
	DDAblock(viewvec,abview,block,&renderNormalreso,&renderBackreso,&normalNeedInTex_ExistInMainBn,&rnRequestQueue);
	//printf("必要な現解像度ブロックの数=%d,これからロード[%d],MAXTEXLOAD[%d]\n",render_normal_num,normalNeedInTex_ExistInMainBn,MAXTEXLOAD);

	////ロードするブロック数が多かったらバックアップ解像度octree追跡
	if(normalNeedInTex_ExistInMainBn > MAXTEXLOAD || rnRequestQueue.size()>maxRenderNum)
	{
		backupOrNot=true;
		int backupNeedInTex_ExistInMainBn = 0;
		rbRequestQueue.clear();
		initCountflag(renderBackreso,render_normal_blocks,r_existInMainMemNotInTexMem);
		DDAblock(viewvec,abview,block,&renderBackreso,&renderBackreso,&backupNeedInTex_ExistInMainBn,&rbRequestQueue);
		renderBlockNum=rbRequestQueue.size();
		if(backupNeedInTex_ExistInMainBn>=MAXTEXNUM){printf("変に上書きされる危険性あり\n");}
		//printf("バックアップlevel[%d][%d],これからロード[%d]\n",renderBackreso,rbRequestQueue.size(),backupNeedInTex_ExistInMainBn);
		rendernbnstr.str("");
		rendernbnstr<<"need blocks for whole render reso:[backup zone]"<<rbRequestQueue.size()<<"/"<<MAXTEXNUM<<"[MAXTEXNUM]";
		renderebnstr.str("");
		renderebnstr<<"exist in Main not in TexMem blocks of render reso:[backup zone]"<<backupNeedInTex_ExistInMainBn<<"/"<<MAXTEXNUM<<"[MAXTEXNUM]";
		renderresostr.str("");
		renderresostr<<"render reso level:"<<renderBackreso<<"(backupZone)";

		//ブロック描画
		int normalloadmargin = MAXTEXLOAD- backupNeedInTex_ExistInMainBn;
		int j=0;
		//printf("MAXTEXLOAD=%d,キューサイズ=%dピンチブロック=%d\n",MAXTEXLOAD,rbRequestQueue.size(),pinchBlockCount);
	//	if(rbRequestQueue.empty()){printf("backupzone呼び出す前から空だ\n");}
		while(j < MAXTEXLOAD && !rbRequestQueue.empty())//呼び出し先の両端キューが空の場合はtrueを返し、そうでない場合はfalseを返す
		{
			LARGE_INTEGER blbe,blaf,blfr;
			QueryPerformanceCounter(&blbe);
			QueryPerformanceFrequency(&blfr);
			Block bl = rbRequestQueue.front();//両端キューの先頭への参照を返す。
			if(!file.texBlockExist(bl) && file.blockExist(bl))
			{
				file.loadMainToTex(bl,&Cg,Cg.vdecalParam);//実際vdecalParamはつかってない。
				loadtexblockcount += 1;
				j++;
			}
			renderBlock(bl);//ここを消してもあまり結果が変わらない。
			rbRequestQueue.pop_front();//両端キューの最初の要素を削除する。
			QueryPerformanceCounter(&blaf);
			bltime_sum+=(double)(blaf.QuadPart-blbe.QuadPart)*1000/(double)blfr.QuadPart;
			blcount++;
		}
		//ブロックロード
		int i = 0;
		while(i < normalloadmargin && !rnRequestQueue.empty())//呼び出し先の両端キューが空の場合はtrueを返し、そうでない場合はfalseを返す
		{
			LARGE_INTEGER blbe,blaf,blfr;
			QueryPerformanceCounter(&blbe);
			QueryPerformanceFrequency(&blfr);

			Block bl = rnRequestQueue.front();//両端キューの先頭への参照を返す。
			if(!file.texBlockExist(bl) && file.blockExist(bl))
			{
				file.loadMainToTex(bl,&Cg,Cg.vdecalParam);
				loadtexblockcount += 1;
				i++;
			}
			rnRequestQueue.pop_front();//両端キューの最初の要素を削除する。
			QueryPerformanceCounter(&blaf);
			bltime_sum+=(double)(blaf.QuadPart-blbe.QuadPart)*1000/(double)blfr.QuadPart;
			blcount++;

		}

	}
	else
	{//多すぎなかった場合
		
		backupOrNot=false;
		renderBlockNum=rnRequestQueue.size();
		rendernbnstr.str("");
		rendernbnstr<<"need blocks of render reso:[normal zone]"<<rnRequestQueue.size()<<"/"<<MAXTEXNUM<<"[MAXTEXNUM]";
		renderebnstr.str("");
		renderebnstr<<"exist in Main not in TexMem blocks of render reso:[normal zone]"<<normalNeedInTex_ExistInMainBn<<"/"<<MAXTEXNUM<<"[MAXTEXNUM]";
		renderresostr.str("");
		renderresostr<<"render reso level:"<<renderNormalreso<<"(normalzone)";
		//ブロック描画
	//	printf("MAXTEXLOAD=%d,キューサイズ=%d,ピンチブロック=%d\n",MAXTEXLOAD,rnRequestQueue.size(),pinchBlockCount);
		if(rnRequestQueue.empty()){printf("現解像度zone呼び出す前から空だ\n");}
		while(!rnRequestQueue.empty())//呼び出し先の両端キューが空の場合はtrueを返し、そうでない場合はfalseを返す
		{
			LARGE_INTEGER blbe,blaf,blfr;
			QueryPerformanceCounter(&blbe);
			QueryPerformanceFrequency(&blfr);
			Block bl = rnRequestQueue.front();//両端キューの先頭への参照を返す。
			if(!file.texBlockExist(bl))
			{
				Block backup = bl.getMultiLowblock(backupreso-normalreso);
				if(file.blockExist(bl))
				{
					file.loadMainToTex(bl,&Cg,Cg.vdecalParam);
					loadtexblockcount += 1;
				}
				else if(file.blockExist(backup) && !file.texBlockExist(backup))
				{
					file.loadMainToTex(backup,&Cg,Cg.vdecalParam);
					loadtexblockcount += 1;
				}  
			}
			//float dis=bl.calcDistanceFromcamera(modelMatrix);
			//bl.printBlockInfo("distance",dis);
			renderBlock(bl);//ここを消したら一瞬見えてまた消える。
			rnRequestQueue.pop_front();//両端キューの最初の要素を削除する。
			QueryPerformanceCounter(&blaf);
			bltime_sum+=(double)(blaf.QuadPart-blbe.QuadPart)*1000/(double)blfr.QuadPart;
			blcount++;
		}
	}//backup or normal
	

	ltncount++;
	if(ltncount>SAMPLEINTERVAL)//SAMPLEINTERVAL=1
	{
			loadtexblocknum.str("");
			ltn = loadtexblockcount;
			loadtexblocknum<<"average number of texture load blocks:"<<ltn;
			ltncount = 0;
	}


	if(infoflag)
	{
		if(mflag)
		{

			temp_occnum += occblockcount;//カリングされたブロックの数
			temp_rennum += renderblockcount;//レンダリングしてるブロックの数

			count++;
			if(count>0)
			{

				occlutionnum.str("");
				obn = temp_occnum/count;
				occlutionnum<<"number of culling blocks at last frame:"<<obn;
				temp_occnum = 0.0;

				renderblocknum.str("");
				rbn = temp_rennum/count;
				renderblocknum<<" number of rendering blocks  at last frame:"<<rbn;
				temp_rennum = 0.0;

				
				maxloadtexblocknum.str("");
				maxloadtexblocknum<<"max number of texture load blocks:[MAXTEXLOAD]"<<MAXTEXLOAD;


				count = 0;
			}
		}
	} 
	bltime_perframe=bltime_sum/blcount;
	double loadpercent=(double)(loadtexblockcount)/blcount;
	timelog<<bltime_perframe<<","<<loadpercent<<","<<blcount<<"\n";

}
void initCountflag(int diff,bool **** needCountflag,bool **** existCountflag)
{
	for(int a= diff; a < NUMLEVEL;a ++)
	{
		int num = (int)pow(2.0,(NUMLEVEL-1.0)-a);
		int numx = INIBLX*num;
		int numy = INIBLY*num;
		int numz = INIBLZ*num;

		for(int i=0 ;i < numx;i++)
		{
			for(int j=0;j < numy;j++)
			{
				for(int k=0;k <numz;k++)
				{
					needCountflag[a][i][j][k] = false;
					existCountflag[a][i][j][k] = false;
				}
			}
		}
	}
}
/* 立方根を求める */
double GetCbRoot(double x)
{
    double stv;
	 if (x >= 0)        /* x が正なら平方根を返す */
        stv = (sqrt(x));
    if (x < 0)      /* x が負なら絶対値の平方根 */
        stv = (-sqrt(-x));    /* に-を付けて返す */


	double dx = stv / 10;
	
    if (x == 0.0)        /* x が０なら平方根も０ */
        return (0.0);

    if (x < stv * stv * stv)
        dx *= -1.0;

    while (1) {    /* ２重の無限ループ */
        while (1) {
            if (( dx > 0 && (x < stv * stv * stv)) || 
                (dx < 0 && (x > stv * stv * stv))) 
                              /* 立方根と近似値の大小関係が変化したら */
                break;        /* 内側の無限ループから抜ける */
            else
                stv += dx;
        }
        if (fabs(dx) < 0.00000001)
                      /* 小数点以下８桁まで精度が出たら */
            break;    /* 外側の無限ループから抜ける */
        else          /* まだならここ */
            dx *= -0.1;
    }
    return (stv);
}
void display(void)
{	//printf("描画開始,");
	frameTime=0;
 order=0;
 LARGE_INTEGER fraf,frbe,frfr;
	QueryPerformanceCounter(&frbe);
	QueryPerformanceFrequency(&frfr);
	/* ライティング設定 */
	lighting();                                        

	//読み取りよう視錐台のモデルビュー行列作成
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(Next_translate.x,Next_translate.y, Next_translate.z);     /* 平行移動(奥行き方向) 　　*/
	glMultMatrixd(modelMatrix);	
	glRotated(angleDelta, axisX, axisY, axisZ);  
	glGetDoublev(GL_MODELVIEW_MATRIX,  Next_modelMatrix);
	glPopMatrix();
	//描画

	/*二つのFBOを初期化*/
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);//ここから、テクスチャRECTANGLEとして扱いたい描画内容のフレームバッファの内容を描く。
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
	modelViewMatrix.set((float)modelMatrix[0], (float)modelMatrix[4], (float)modelMatrix[8], (float)modelMatrix[12],
						(float)modelMatrix[1], (float)modelMatrix[5], (float)modelMatrix[9], (float)modelMatrix[13],
						(float)modelMatrix[2], (float)modelMatrix[6], (float)modelMatrix[10],(float)modelMatrix[14],
						(float)modelMatrix[3], (float)modelMatrix[7], (float)modelMatrix[11], (float)modelMatrix[15]);
	inv_modelView = App::inverseMatrix4X4(&modelViewMatrix);
	Vec4 cam = inv_modelView.vec_maltiply(&ini_cam);//ini_camは(0.0,0.0,0.0,1.0)
	float camera[3] = {cam.x,cam.y,cam.z};
	Cg.SetParameter(Cg.cameraParam,camera);
	Vec3 l = inv_modelView.vec_maltiply3(&ini_l);//注視点
	Vec3 u = inv_modelView.vec_maltiply3(&ini_u);//視界の上方向を決めるupper vector
	renderfrustum.setCamDef(cam.getXYZ(),l,u);//gluLookAtと同じ（カメラ、注視点、アッパーベクタ）　カメラパラメータから視錐台を構成する。　renderfrustumはFrustumGクラス
	
	Block block = Block::getInitblock();  
	Vec3 vz = block.getBlockVec(modelViewMatrix);
	setFrontSlice();//消えても問題ない。
	Vec3 abview = getViewvector(&vz);
	
	//abview.print("abview");
	renderFrustum(vz,abview,block);//これを消したらボリュームとブロックの線が見えなくなる。ここに一番大事なレンダリング情報が入ってる。
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	cgGLEnableTextureParameter(Cg.transfer_functionParam);	Cg.CheckCgError();
	


	// 'unbind' the FBO. things will now be drawn to screen as usual
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);//ここから、↑で描いた内容をウィンドウのどこに描くのかが決められる。

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_RECTANGLE, fbTex);//これを消すと画面が白くなる。フレームバッファのカラーコンポネント
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0,1.0,-1.0,1.0,1.0,20.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslated(0.0,0.0,-5);
	glEnable(GL_TEXTURE_RECTANGLE);
	glColor4f(1.0,1.0,1.0,1.0);//オブジェクト全体の色　というか、画面全体を覆うテクスチャの色かも。
	glBegin(GL_QUADS); //これは、画面全体を覆うテクスチャ。
	glTexCoord2i(0, 0);
	glVertex2f(-1.0, -1.0);

	glTexCoord2i(0,winheight);glVertex2f(-1.0, 1.0);

	glTexCoord2i(winwidth,winheight);glVertex2f(1.0,1.0);

	glTexCoord2i(winwidth,0);glVertex2f(1.0, -1.0);
	glEnd();
	glDisable(GL_TEXTURE_RECTANGLE);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, 0);//これを消してもレンダリング結果は変わらない。

	renderSubWindow();

	/* 画面文字表示*/
	renderInfo();

	glDisable(GL_BLEND);
	frame++;
	//time=glutGet(GLUT_ELAPSED_TIME);
	//if (time - timebase > 100) {//本当は1000だけど、100にしてあるのは深い意味があるかもしれない
	//if ( frame >= SAMPLEINTERVAL)
	//{
		//time=glutGet(GLUT_ELAPSED_TIME);
		fps.str("");
		//fpsresult = frame*1000.0f/(time-timebase);//miffy ver
		//float fpsresult = frame*1000.0f/(time-timebase);
		/*totalrender.totalrendertime = 1000.0f/fpsresult;
		fps<<"FPS:"<<fpsresult<<"  (rendering time per frame [msec]:"<<totalrender.totalrendertime<<")";
		timebase = time;		
		frame = 0;*/
		//totalrender.flag = true;//motomotokokoni atta
		stringstream ssmessage;
		QueryPerformanceCounter(&fraf);
totalrender.totalrendertime=(double)(fraf.QuadPart-frbe.QuadPart)*1000/(double)frfr.QuadPart;
totalrendertime_sum+=(double)(fraf.QuadPart-frbe.QuadPart)*1000/(double)frfr.QuadPart;
fpsresult=1000/totalrender.totalrendertime;
totalrender.flag = true;
maintotextime=file.getMainToTexTime();
if(infoflag)
	{
		//if(mflag)
		//{
			frame_special++;
		//}
}
ssmessage<<"最適解像度"<<idealBlockNum<<" backup?"<<backupOrNot<<" レンダリングブロック数"<<renderBlockNum;
//ssmessage<<"totalrendertime:"<<totalrender.totalrendertime<<" maxtexnum"<<MAXTEXLOAD<<" MAXBNUM="<<MAXBNUM<<" FPS:"<<fpsresult;
	
	//	cout<<"totalrendertime:"<<totalrender.totalrendertime<<endl;//miffyがコメントアウトした。
	glutSetWindowTitle((char*)ssmessage.str().c_str());//ウィンドウタイトルバーに文字を表示
	//}}

	//cout<<"block number resio:"<<needblcount*100.0/MAXBNUM<<"%"<<endl;
//for(int i=0;i<MAXTEXNUM;i++){mainmemlog<<file.returnTexMemInfo(i)<<",";}
//		mainmemlog<<"\n";
//		mainmemParameterLog<<BBLoadMargin<<","<<backupreso<<","<<backnbn<<","<<backebn<<","<<normalreso<<","<<normalnbn<<","<<normalebn<<"\n";
//
	/* ダブルバッファリング */
	glutSwapBuffers();

}
void SetFrustum()
{
	SetRenderFrustum();
	SetLoadFrustum();
}
void printMatrix(double matrix[16],const char* message){
	printf("%s\n",message);
	printf("[%.3f][%.3f][%.3f][%.3f]\n",matrix[0],matrix[1],matrix[2],matrix[3]);
	printf("[%.3f][%.3f][%.3f][%.3f]\n",matrix[4],matrix[5],matrix[6],matrix[7]);
	printf("[%.3f][%.3f][%.3f][%.3f]\n",matrix[8],matrix[9],matrix[10],matrix[11]);
	printf("[%.3f][%.3f][%.3f][%.3f]\n",matrix[12],matrix[13],matrix[14],matrix[15]);
	puts("");
}
void SetLoadFrustum()
{
	float Next_eyeDis = eyeDis;
	Next_eyeDis -= Next_translate.z;//printf("Next_eyeDis=%.4f[%.4f]\n,",Next_eyeDis,eyeDis);
	float Next_nearSlice = Next_eyeDis / ( 1.0f + Bclipeyeresio*( Bclipresio -1.0f));
	float Next_farSlice = Next_nearSlice * Bclipresio;
	double Next_projMatrix[16];

	glMatrixMode(GL_PROJECTION);               /* 投影変換の設定            */
	glPushMatrix();
	glLoadIdentity(); 
	gluPerspective(thetah, aspect, Next_nearSlice, Next_farSlice);
	glGetDoublev(GL_PROJECTION_MATRIX, Next_projMatrix);
	
	
	float N_a = GetCbRoot(NFrustumSize);//立方根を求める
	float N_m = Next_nearSlice*(1.0f + Bclipresio)/2.0f;
	float N_l = Next_nearSlice*(Bclipresio - 1.0f)*N_a/2.0f;
	float N_thetah = atan(Next_nearSlice*N_a*tan(thetah*M_PI/180.0f)/(N_m - N_l))*180.0f/M_PI;//画角を導き出した
	//printMatrix(Next_projMatrix,"nextproj");
	frustum.setCamInternals(Next_projMatrix,N_thetah,aspect,N_m - N_l,N_m + N_l);	
	//frustum.print("load");
	
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}
void SetRenderFrustum()
{
	glMatrixMode(GL_PROJECTION);               /* 投影変換の設定            */
	glLoadIdentity(); 
	dPlaneStart = eyeDis - nearSlice;     //スライス開始位置
	Cg.SetParameter(Cg.dPlaneStartParam , dPlaneStart);
	Cg.SetParameter(Cg.farSliceParam,farSlice); 
	gluPerspective(thetah, aspect, nearSlice, farSlice);
	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
	
	renderfrustum.setCamInternals(projMatrix,thetah,aspect,nearSlice,farSlice);
	
	
	glMatrixMode(GL_MODELVIEW);

}
void myReshape(int w, int h)
{
	angleDelta = 0.0;   
	Next_translate.x = 0.0;
	Next_translate.y = 0.0;
	Next_translate.z = 0.0;  


	winwidth = w;
	winheight = h;
	widthstring.str("");
	widthstring<<"window width:"<<winwidth;
	heightstring.str("");
	heightstring<<"window height:"<<winheight;


	glMatrixMode(GL_MODELVIEW);

	//FBO作り直し
	createFBO();


	glLoadIdentity();
	gluLookAt(eyeInitPosition[0], eyeInitPosition[1], eyeInitPosition[2],0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	/* 視点と注視点の指定 */



	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
	glGetIntegerv(GL_VIEWPORT, viewport);
	gluProject(0.0, 0.0, 0.0, modelMatrix, projMatrix, viewport,&originX, &originY, &originZ);

	multMatrix(modelMatrix, eyeInitPosition, eyePosition);
	eyeDis = eyeInitPosition[2];
	multMatrix(modelMatrix, lightAPosition, lightPosition);



	
	                         
	glViewport(0, 0, w, h);                    /* Window上での描画領域設定  */
	aspect = (float)w/(float)h;


	if( w < h ){
		thetah = atan(tan(M_PI/12.0f)*h/w)*360.0f/M_PI;

	}
	else {
		thetah = 30.0;
	}
	thetaw = thetah*aspect;
	

	
	
	//ブロック前後クリッピング初期化
	nearSlice = INITNEAR;
	farSlice  = INITFAR;
	Bclipeyeresio = (eyeDis-nearSlice)/(farSlice - nearSlice);
	SetFrustum();

}
void multMatrix(double matrix[], float org[], float dst[]) {
	float tmp[4];
	tmp[0] = org[0]; tmp[1] = org[1]; tmp[2] = org[2]; tmp[3] = org[3];
	for (int i = 0; i < 4; i++) {
		dst[i] = 0.0;
		for (int j = 0; j < 4; j++) {
			dst[i] += (float)matrix[j*4+i] * tmp[j];
		}
	}
}
void idle(void){                               /* Idleイベント関数    */
	if(zoomflag)
	{
		if(abs(Next_translate.z) > TranslateEpsilon)
			Next_translate.z *= IdleTranslateRatio;
		else
			Next_translate.z = 0.0;
	
		zoomflag = false;
	}
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(Next_translate.x,Next_translate.y,Next_translate.z);     /* 平行移動(奥行き方向) 　　*/
	glMultMatrixd(modelMatrix);
	glRotated(angleDelta, axisX, axisY, axisZ);   /* 回転　　　　　　*/
	
	eyeDis -= Next_translate.z;
	nearSlice = eyeDis / ( 1.0f + Bclipeyeresio*( Bclipresio -1.0f));
	farSlice = nearSlice * Bclipresio;
	SetFrustum();
	
	glutPostRedisplay();                       //Displayイベント発生 
}
void mouse(int button ,int state, int x, int y){  /* Mouseイベント関数 */
	if (state == GLUT_DOWN){                   /* ボタンプレスの場合  */
		buttondown = button;                   /* ボタン状態の変更　　*/
		beginX = x;                            /* カーソル位置の記録(更新) */
		beginY = y;
		if (button == GLUT_LEFT_BUTTON) {      /* 左ボタンのイベントの場合 */
			angleDelta = 0.0;   
			Next_translate.x = 0.0;
			Next_translate.y = 0.0;
			Next_translate.z = 0.0;                  /* 回転角のクリア(無回転)   */
			glutIdleFunc(NULL);                /* Idleイベント関数のクリア */
		}

		if(button == GLUT_MIDDLE_BUTTON){
			angleDelta = 0.0;   
			Next_translate.x = 0.0;
			Next_translate.y = 0.0;
			Next_translate.z = 0.0;
			glutIdleFunc(NULL);
		}
	}
	else if (state == GLUT_UP){                /* ボタンリリースの場合　*/
		buttondown = -1;                       /* ボタン状態の変更 up(-1)  */
		if (button == GLUT_LEFT_BUTTON) {      /* 左ボタンのイベントの場合 */
			if ( angleDelta > AngleEpsilon ){  /* 回転角がゼロでない場合　 */
				angleDelta *= IdleAngleRatio;
				
			}
			else { 
				angleDelta = 0.0;
			}		
		}
		
		if(button == GLUT_MIDDLE_BUTTON){
			if(abs(Next_translate.x) > TranslateEpsilon)
				Next_translate.x *= IdleTranslateRatio;
			else
				Next_translate.x = 0.0;

			if(abs(Next_translate.y) > TranslateEpsilon)
				Next_translate.y *= IdleTranslateRatio;
			else
				Next_translate.y = 0.0;

		}

		glutIdleFunc(idle);            /* Idleイベント関数の設定   */
	}
}
void motion(int x, int y){                     /* Motionイベント関数      */
	double deltaX, deltaY,deltaZ;                     /* マウスカーソルの移動量　*/
	double X, Y, Z;
	double identMatrix[16] =              /* 単位モデル変換行列 　　　*/
	{1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 
	0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0}; 



	if (buttondown >= 0) {                /* ボタン状態down(非負)の場合 */

		glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
		/* モデル変換行列の取得    */

		switch(buttondown){
case GLUT_LEFT_BUTTON:
	deltaX = x - beginX;                   /* カーソル移動量(x方向)   */
	deltaY = y - beginY;                   /* カーソル移動量(y方向)   */
	angleDelta = sqrt(deltaX*deltaX + deltaY*deltaY) * AngleRatio;
	/* 回転角の計算 = カーソル移動距離*AngleRatio */
	if (angleDelta > AngleEpsilon) {       /* 回転角がゼロでない場合 */
		gluUnProject(originX+deltaY, originY+deltaX, originZ,modelMatrix, projMatrix, viewport,&axisX, &axisY, &axisZ);       /* 回転軸の計算（点)       */
		gluUnProject(originX, originY, originZ, modelMatrix, projMatrix,viewport, &X, &Y, &Z);         /* 回転軸の計算 (原点)     */
		axisX -= X;  axisY -= Y;  axisZ -= Z; /* 回転軸の計算(ベクトル) */
		glRotated(angleDelta, axisX, axisY, axisZ); /* 回転           */
	}
	else { 
		angleDelta = 0.0;
	}
	break;
case GLUT_MIDDLE_BUTTON:
case GLUT_RIGHT_BUTTON:
	glMatrixMode(GL_PROJECTION);      /* 投影変換の設定 　　　　　*/
	gluUnProject((double)x, (double)y, originZ,identMatrix, projMatrix, viewport,&X, &Y, &Z);         /* 平行移動の後 　　　　　　*/
	gluUnProject((double)beginX, (double)beginY, originZ,identMatrix, projMatrix, viewport,&deltaX, &deltaY, &deltaZ); /* 平行移動の後　　　*/
	X -= deltaX;  Y -= deltaY;        /* 平行移動量の計算 　　　　*/
	if(buttondown == GLUT_RIGHT_BUTTON)
	{
		
		nearSlice -= (float)Y;
		farSlice = Bclipresio * nearSlice;
		Bclipeyeresio = (eyeDis-nearSlice)/(farSlice-nearSlice);
		SetFrustum();
	}
	else
	{
		Next_translate.x = X;
		Next_translate.y = -Y;
		glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslated(Next_translate.x,Next_translate.y, 0.0);     /* 平行移動(奥行き方向) 　　*/
		glMultMatrixd(modelMatrix);
	}
	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
	/* 射影変換行列の取得 　　　*/
	gluProject(0.0, 0.0, 0.0, modelMatrix, projMatrix, viewport,&originX, &originY, &originZ); /* モデル座標原点の変換　*/
	/* 原点がスクリーン座標に写された位置(回転軸の計算に利用)　*/
	glMatrixMode(GL_MODELVIEW);       /* モデル変換の設定 　　　　*/
	break;
		}

		glutPostRedisplay();                   /* Displayイベント発生     */
		beginX = x;                            /* カーソル位置の記録(x方向) */
		beginY = y;                            /* カーソル位置の記録(y方向  */
	}
}
void wheel(int wheel_number,int direction,int x,int y)
{
	Next_translate.z -= direction*0.01*eyeDis;
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(0.0,0.0,Next_translate.z);
	glMultMatrixd(modelMatrix);
	
	eyeDis -= Next_translate.z;
	nearSlice = eyeDis / ( 1.0f + Bclipeyeresio*( Bclipresio -1.0f));
	farSlice = nearSlice * Bclipresio;
	SetFrustum();	


	zoomflag = true;
	glutPostRedisplay();                   // Displayイベント発生     

}
static void keyboard(unsigned char c, int x, int y)
{
	if(c=='r'){
		//if(!nowprocessingflag){}

	//	mainmemlog<<"バックアップ領域"<<BBLoadMargin
		}
	if(c==27){
		exit(0);
	}
	if(c == 'f')
	{
		if(subwindowflag == false)
			subwindowflag = true;
		else 
			subwindowflag = false;
	}

	if(c=='g'){
		showGridFlag=!showGridFlag;
	}
	if(c == 's')
	{
		if(frustumstate == FrustumG::render)
			frustumstate = FrustumG::normal;
		else if(frustumstate == FrustumG::normal)
			frustumstate = FrustumG::back;
		else
			frustumstate = FrustumG::render;
	}

	if(c == 'd')
	{
		if(Block::resocolorflag == false)
			Block::resocolorflag = true;
		else 
			Block::resocolorflag = false;
	}

	if(c == 'a')
	{
		if(infoflag == false)
			infoflag = true;
		else 
			infoflag = false;
	}

	glutPostRedisplay();

}
void InitBflag()
{

	b_needInMain = new bool ***[NUMLEVEL];
	b_existInMain = new bool ***[NUMLEVEL];
	n_needInMain = new bool ***[NUMLEVEL];
	n_existInMain = new bool ***[NUMLEVEL];
	render_normal_blocks = new bool ***[NUMLEVEL];
	r_existInMainMemNotInTexMem = new bool ***[NUMLEVEL];
	//blockQueueMap=new unsigned int***[NUMLEVEL];
//	Block::occludedIndex=new bool ***[NUMLEVEL];

	for(int a= 0; a < NUMLEVEL;a ++)
	{
		int num = (int)pow(2.0,(NUMLEVEL-1.0)-a);
		int numx = INIBLX*num;
		int numy = INIBLY*num;
		int numz = INIBLZ*num;


		b_needInMain[a] = new bool **[numx];
		b_existInMain[a] = new bool **[numx];
		n_needInMain[a] = new bool **[numx];
		n_existInMain[a] = new bool **[numx];
		render_normal_blocks[a] = new bool **[numx];
		r_existInMainMemNotInTexMem[a] = new bool **[numx];
	//	Block::occludedIndex[a]=new bool **[numx];

		for(int i=0 ;i < numx;i++)
		{
			b_needInMain[a][i] = new bool *[numy];
			b_existInMain[a][i] = new bool *[numy];
			n_needInMain[a][i] = new bool *[numy];
			n_existInMain[a][i] = new bool *[numy];
			render_normal_blocks[a][i] = new bool *[numy];
			r_existInMainMemNotInTexMem[a][i] = new bool *[numy];
	//		Block::occludedIndex[a][i]= new bool *[numy];

			for(int j=0;j < numy;j++)
			{
				b_needInMain[a][i][j] = new bool [numz];
				b_existInMain[a][i][j] = new bool [numz];
				n_needInMain[a][i][j] = new bool [numz];
				n_existInMain[a][i][j] = new bool [numz];
				render_normal_blocks[a][i][j] = new bool [numz];
				r_existInMainMemNotInTexMem[a][i][j] = new bool [numz];
			//	Block::occludedIndex[a][i][j]=new bool [numz];

				for(int k=0;k <numz;k++)
				{
					b_needInMain[a][i][j][k] = false;
					b_existInMain[a][i][j][k] = false;
					n_needInMain[a][i][j][k] = false;
					n_existInMain[a][i][j][k] = false;
					render_normal_blocks[a][i][j][k] = false;
					r_existInMainMemNotInTexMem[a][i][j][k] = false;
				//	Block::occludedIndex[a][i][j][k]=false;
				}
			}
		}
	}
}
/* スレッド関数*/
void fileget_func(void *arg){	
//printf("セマフォ待ち\n");
	WaitForSingleObject(sem,INFINITE);//セマフォ開始（セマフォカウンタを１減らす）ここからReleaseSemaphore(sem)までが排他制御した処理
	//もしセマフォカウンタが０だったら、このスレッドはこのカウンタが他のスレッドによって１以上になるのを待って処理がブロックされる。
	//double sum=0;double	counter=0; double average;
	while(1)
	{//結局1ループに1個しかロードしてない]
		/*printf("\n");
		LARGE_INTEGER before,after,freq;
		QueryPerformanceCounter(&before);
		QueryPerformanceFrequency(&freq);*/
		file.countMainLRU();
		file.countThreadTime();/*ハードディスク読込み時間測定用*/
		Next_modelViewMatrix.set((float) Next_modelMatrix[0], (float) Next_modelMatrix[4], (float) Next_modelMatrix[8], (float) Next_modelMatrix[12],
			(float) Next_modelMatrix[1], (float) Next_modelMatrix[5], (float) Next_modelMatrix[9], (float) Next_modelMatrix[13],
			(float) Next_modelMatrix[2], (float) Next_modelMatrix[6], (float) Next_modelMatrix[10],(float) Next_modelMatrix[14],
			(float) Next_modelMatrix[3], (float) Next_modelMatrix[7], (float) Next_modelMatrix[11], (float) Next_modelMatrix[15]);
//Next_modelMatrix[]はdisplay関数でゲットしたもの。
		Next_inv_modelView = App::inverseMatrix4X4(&Next_modelViewMatrix);		
		Vec4 N_cam = Next_inv_modelView.vec_maltiply(&ini_cam);
		Vec3 N_l = Next_inv_modelView.vec_maltiply3(&ini_l);
		Vec3 N_u = Next_inv_modelView.vec_maltiply3(&ini_u);
		//printf(",set file,");
		frustum.setCamDef(N_cam.getXYZ(),N_l,N_u);
		//N_cam.print("file");
		Block block=Block::getInitblock();
		Vec3 vz=block.getBlockVec(Next_modelViewMatrix);
		Vec3 abview = getViewvector(&vz);
		requestNormalFrustum(vz,abview,block);//ここで、必要な現解像度ブロック、バックアップ解像度ブロックの数とかがわかる。
		file.loadHDToMain();
		file.CheckLoadComplete();
		nowprocessingflag=false;
		
		/*QueryPerformanceCounter(&after);
		double whiletime=(double)(after.QuadPart-before.QuadPart)*1000.0f/(double)freq.QuadPart;
		printf("%.3f[msec]\n",whiletime);*/
	}
}
void render_func(void *arg){	

	glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(winwidth,winheight);
	glutCreateWindow("OpenGL");
	glewInit();
	createTF();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutMotionFunc(motion);
	glutMouseFunc(mouse);
	glutReshapeFunc(myReshape);
	glutKeyboardFunc(keyboard);
	glutMouseWheelFunc(wheel);
	Cg.Init();
	cgGLSetTextureParameter(Cg.transfer_functionParam,preintName);Cg.CheckCgError();
	

	myInit();
	file.initTexLoad();//最初にテクスチャメモリにデータをいっぱい載せるか？
	//printf("もすぐセマフォ解放\n");
	//セマファ解放！！
	ReleaseSemaphore(sem,1,NULL);//セマフォ終了（排他処理終了）セマフォカウンタを１増やす
	//printf("セマフォ解放 sem+1\n");
	glutMainLoop();
	Cg.Term();
}
void createTF(){//table for 256*4

	FILE *tffile;
	tffile=fopen("64Bonsai3.tf","rb");
				if(tffile==NULL){
					
					tr2::sys::path mypath;
					cout<<"現在のディレクトリ"<<tr2::sys::current_path<tr2::sys::path>().string()<<endl;
					cout<<"ファイルが見つかりません";abort();
				}
				int tfsize=64;
				GLubyte *TransferFunction = new GLubyte[tfsize*4];
				size_t num = fread(TransferFunction,sizeof(GLubyte),tfsize*4,tffile);
				printf("data num=%d\n",num);
				glGenTextures(1,&preintName);
				glBindTexture(GL_TEXTURE_1D, preintName);
				glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,tfsize,0,GL_RGBA,GL_UNSIGNED_BYTE,TransferFunction);
	
	
}
void exit_func(void){
	
	///*if(frame_special==0 ){frame_special=1;}
	//if(frame_special_occ==0){frame_special_occ=1;}
	//if(tex_req_frame==0){tex_req_frame=1;}
	//if(frame_tex_pro==0){frame_tex_pro=1;}*/
	//stringstream filename;
	//filename<<"texmemlog_alpha"<<HDMAINALPHA*10<<"_tex"<<MAXTEXNUM<<"fopeFPS"<<TEXLOADPARAMETER<<"win"<<winwidth<<"height"<<winheight<<".txt";
	//ofstream outPutFile(filename.str().c_str());
	//outPutFile.setf(ios_base::fixed,ios_base::floatfield);
	//outPutFile.precision(5);
	///*float averaycasttime = raycasttime/(float)frame_special;
	//float aveocclutiontime = occlutiontime/((float)frame_special);
	//float avetexrequesttime = texrequesttime/(float)frame_special;
	//float averagetexprocesstime=texprocesstime/(float)frame_special;
	//float averagetotalrendertime=totalrendertime_sum/(float)frame;*/
	////float averagememrequesttime=memrequesttime/(float)frame;
	////outPutFile<<"raycasttime,occlutiontime,texprocesstime,texrequesttime,totalrendertime,FPS\n";
	////outPutFile<<averaycasttime<<","<<aveocclutiontime<<","<<averagetexprocesstime<<","<<avetexrequesttime<<","<<averagetotalrendertime<<","<<fpsresult<<"\n";
	//outPutFile<<mainmemlog.str().c_str();
	////outPutFile<<"FPS,"<<fpsresult<<",MAXBNUM,"<<MAXBNUM<<",MAXTEXNUM,"<<MAXTEXNUM<<",alpha,"<<HDMAINALPHA<<"\n";
	////outPutFile<<"notexist,renderpreblock,frameout,nextreso,waitblock,renderfirstlowblock,rendersecondlowblock,renderthirdlowblock,renderfourthlowblock,occlusionculling\n";
	////outPutFile<<blockStateCount[0]<<","<<blockStateCount[1]<<","<<blockStateCount[2]<<","<<blockStateCount[3]<<","<<blockStateCount[4]<<","<<blockStateCount[5]<<","<<blockStateCount[6]<<","<<blockStateCount[7]<<","<<blockStateCount[8]<<","<<blockStateCount[9]<<"\n";
	////printf("notexist,renderpreblock,frameout,nextreso,waitblock,renderfirstlowblock,rendersecondlowblock,renderthirdlowblock,renderfourthlowblock,occlusionculling\n");
	///*for(int i=0;i<10;i++){
	//	printf("%d,",blockStateCount[i]);
	//}*/
	stringstream filename2;
	filename2<<"blocktimelog_preTexLoad_alpha"<<HDMAINALPHA*10<<"_tex"<<MAXTEXNUM<<"fopeFPS"<<TEXLOADPARAMETER<<"win"<<winwidth<<"height"<<winheight<<".txt";
	ofstream outPutFile2(filename2.str().c_str());
	outPutFile2.setf(ios_base::fixed,ios_base::floatfield);
	outPutFile2.precision(5);
	outPutFile2<<timelog.str().c_str();
	outPutFile2.close();
	//outPutFile2<<mainmemParameterLog.str().c_str();
	//outPutFile<<raycasttime<<","<<occlutiontime<<","<<texprocesstime<<","<<texrequesttime<<","<<memrequesttime<<"\n";
	file.deleteMemory();
	printf("メモリ解放\n");
	printf("exit\n");
}
/* メインプログラム*/
int main(int argc, char *argv[])
{
	timelog<<"平均処理時間,テクスチャロードした割合,総ブロック数\n";
	//	mainmemParameterLog<<"BBLoadMargin,バックアップ解像度,必要なバックアップブロック,既に存在しているバックアップブロックの数,現解像度,必要な現解像度ブロック,既に存在している現解像度ブロック\n";
	atexit( exit_func );//プログラム終了時に呼び出される関数 escape押したときだけ
	//メインメモリ空き容量取得
	MEMORYSTATUS ms;
	GlobalMemoryStatus(&ms);
	DWORD dwPh = (ms.dwAvailPhys);
	//printf("dwPh=%i\n",dwPh);
	MAXBNUM = (int)(DATAPOOLALPHA*(float)(dwPh/((BLX+2)*(BLY+2)*(BLZ+2)*VOXELSIZE)));//1150,1173 実行するたびに変わる！！！
	if(MAXBNUM>DATAPOOLBORDER){MAXBNUM=1000;}//1000ならいけた。
	printf("MAXBNUM=%d\n",MAXBNUM);
	//MAXBNUM = 800;
	QSIZE = (int)(DATAPOOLALPHA*(float)(dwPh/((BLX+2)*(BLY+2)*(BLZ+2)*VOXELSIZE)));
	//printf("QSIZE=%i",QSIZE);//1219

	glutInit(&argc, argv);
	InitBflag();

	thread_arg_t ftarg, rtarg;//スレッド関数の引数（どっちもなしかな）

	sem = CreateSemaphore(NULL,0,1,NULL);//セマフォの初期値＝0,セマフォの最大数=1　ここでエラー
	
	//ファイルインスタント初期化
	nowprocessingflag=true;
	file.Init(path,dataname);
	file.initMainLoad();//最初にメインメモリにいっぱいデータを載せておくか？
	nowprocessingflag=false;
	/*filegetスレッドの作成*/
	/*スレッド関数の引数データの初期化*/
	ftarg.thread_no = 0;

	/*スレッドの生成*/
	HANDLE filegetHandle =CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)fileget_func,(void *)&ftarg,0,NULL);
	//fileget = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)fileget_func,(void *)&ftarg,0,NULL);
	SetThreadIdealProcessor(filegetHandle,0);
	//SetThreadIdealProcessor(fileget,0);


	/*renderスレッドの作成*/
	/*スレッド関数の引数データの初期化*/
	rtarg.thread_no = 0;

	/*スレッドの生成*/
	HANDLE renderHandle= CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)render_func,(void *)&rtarg,0,NULL);
	SetThreadIdealProcessor(renderHandle,1);//生成したスレッドrenderの優先プロセッサを１番に指定。

	//render = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)render_func,(void *)&rtarg,0,NULL);
	//SetThreadIdealProcessor(render,1);//生成したスレッドrenderの優先プロセッサを１番に指定。

	//WaitForMultipleObjects(2,handle,TRUE,INFINITE);//この辺で終わる。
	WaitForSingleObject(renderHandle,INFINITE);//セマフォ開始（セマフォカウンタを１減らす）
	return 0;
}




