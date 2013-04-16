#pragma once
#include <assert.h>
#include <queue>//<queue>ヘッダの内部で<deque>ヘッダを呼んでいる。dequeはdouble-ended-queue（両頭キュー）の略。
#ifndef  __GLEW_H__
#include <GL/glew.h>
#endif
#ifndef MIFFY_VEC2
#include <miffy/math/vec2.h>
#endif
#ifndef MIFFY_VEC3
#include <miffy/math/vec3.h>
#endif
#include <miffy/math/color.h>
#include <miffy/math/colorpalette.h>
#include "Cg.h"
#include "File.h"
#include "Block.h"
/*レンダリング時間計測構造体*/
typedef struct _TotalRendering {
  float totalrendertime;
	bool flag;
} TotalRendering;

/*!
LODといいつつも、とりあえずはビューフラスタムカリングも入れる。おおざっぱにね。
*/
class CLod
{
public:
	CLod(int _w,int _h);
	~CLod(void);
	void InitBflag();
	void DDAblock(vec3<float> viewvec, vec3<float> abview,Block bl,int* normalReso,int* backupReso,int* existblcount,deque<Block>* blockQue);
	void DDAblockFile(vec3<float> _viewvec, vec3<float> _abview,Block _bl,int* _reso,int* _exist_in_main_count,unsigned int** _blockstate,bool ****needCountMap,bool **** existInMainMap,deque<Block>* blockQue);
	void Exit();
	void FileInit(string _path, string _dataname);
	void FileTexInit();//最初にテクスチャメモリにデータをいっぱい載せるか？
	void FileRun();
	void testLoadFrustum(Block block,int* octreeReso,int* existblcount,unsigned int** renderblock,bool **** needCountflag,bool **** existCountflag, deque<Block>* blockQue);
	void initCountflag(int diff,bool **** needCountflag,bool **** existCountflag);
	void Init();
	void Idle();
	void createTF();
	void exit_func(void);
	vec3<float> getViewvector(vec3<float>* view_z);
	void ProcessNextResoLoad(Block bl,int* countA,int* countB,unsigned int** rblock,bool **** needCountflag,bool **** existCountflag,deque<Block>* blockQue);//NExt->次のより高解像度のやつって意味。
	void ProcessNextResoRender(Block bl,int* countA,int* countB,int* countC,deque<Block>* blockQue);//1段階高解像度のブロックのノードを探索
	void display();
	void setFrontSlice(Cg& _cg);
	void renderFrustum(vec3<float> _viewvec, vec3<float> _abview,Block _rootBlock);//これがなくなると、ボリュームとブロックの線がなくなる。
	void renderBlock(Block block);//Blockクラスにいるべきでは？
	void testRenderFrustum(Block block,int* Preso,int* Sreso,int* NeedInTex_ExistInMainBlcount,deque<Block>* blockQue);
	void renderDebugWindow();
	void requestNormalFrustum(vec3<float> viewvec,vec3<float> abview,Block block);
	void SetClippingPlane();
	void PrintInfoOnWindow();
	void Reshape(int _w,int _h);
	void Translate(float _shift);
	void destroyFBO();
	void createFBO();
	void SetLighting(void);
	void SetNextFrustum();
	void SetRenderFrustum();
	void PasteFBOTexture();
private:
	int s_normalreso;//現解像度
	int s_backupreso;//バックアップ用の解像度。現解像度よりも必ず１個粗い。
	/*!
	@name 先回り視錐台関連
	*/
	//@{
	frustum<float> m_localNextFrustum;/// 先回りしてファイルを読んでおく用の視錐台 予測は、現在の回転ベクトル、へいこういどう　ベクトルから予測する。しかもローカル座標での位置がほしい
	mat4<float> Next_modelViewMatrix;
	//@}
	mat4<float> m_InvModelView;//いろいろなとこで使う setFrontSlice getViewvector renderDebugWindow
	frustum<float> m_localRenderFrustum;/// 今レンダリング中の視錐台 
	
	float aspect;
	float m_FovY;
	float m_Bclipresio;
	float Bclipeyeresio;//ズーム値が変わるたびに代わるクリッピング面。
	
	
	/*!
	@nameプロファイリング用
	*/
	//@{
	TotalRendering totalrender;
	double bltime_sum;//1フレームごとの足し算
	double blcount;//1フレームごと
	double bltime_perframe;//1フレームの、ブロックの各処理にかかった時間の平均
	double maintotextime;
	
	int frame_tex_pro;
	int tex_req_frame;
	float rbn;/// これはmaxRendernumにかかわるから大事だ。
	float obn;/// これはmaxRendernumにかかわるから大事だ。
	float ltn;/// これはMAXTEXLOADにかかわるから大事だ。loadtexblockcountと同義かもしれない。

	float totalrendertime_sum;
	
	int loadtexblockcount;/// MAXTEXLOADにかかわるから大事
	//@}
	int normalnbn;//描画に必要な現解像度ブロックの数
	int normalebn;//既に存在している現解像度ブロックの数
	int backnbn;
	int backebn;
	int idealBlockNum;/// 最適解像度ブロックの数
	bool backupOrNot;/// 今、レンダリングが間に合わなくてバックアップ解像度状態になっているのか、そうでないのか。あれ、私は部分的制御したはずだけど。
	int renderBlockNum;/// 実際にレンダリングしているブロックの数
	float nearSlice;
	float farSlice;
	File m_File;
	//パラ5
	float NFrustumSize; //normalfrustumのサイズはAPIビューイングフラスタムの何倍？(台形の体積で考えてます)
	bool**** n_needInMain;
	bool**** n_existInMain;
	bool**** b_needInMain;
	bool**** b_existInMain;
	deque<Block> bRequestQueue;/// requestNormalFrustumで使う
	deque<Block> nRequestQueue;/// requestNormalFrustumで使う
	deque<Block> rnRequestQueue;/// バックアップと現解像度の中から選ばれたやつ（だと思う）
	deque<Block> rbRequestQueue;/// バックアップと現解像度の中から選ばれたやつ（だと思う）MAXTEXLOADを超えたときはこっちを使う。（しかし、このマシンのスペックならこのループに入ることはまずないっぽい）
	bool**** render_normal_blocks;/// どのメモリにあるなし関係なく今描画に必要なブロックリスト
	int render_normal_num;
	bool**** r_existInMainMemNotInTexMem;///テクスチャメモリにはないけどメインメモリにはある
	unsigned int **** blockQueueMap;///キューの何番目に何のブロックが入ってるかマップ
	int pinchBlockCount;
	float dPlaneStart;     //スライス開始位置
	vec4<double> m_wBasicCamPos;///これは、ワールド座標系だよね？ glulookatに使う。

	const vec4<float> ini_cam;///0,0,0つまりただの原点。だけど何回も使う。
	const vec3<float> ini_l;//0.1,0
	const vec3<float> ini_u;//0.0,0
	GLuint pbo;//使ってない。
	GLuint fb;
	GLuint fbTex;
	GLuint fbDep;
	int MAXTEXLOAD;
	int maxRenderNum;/// 1度に描画してもよい数
	vec4<double> eyePosition;
	int BACKUPTHRE; /// ファイルのwhileループ1回あたりに読み込んでもよいブロックの最大数
	int BBLoadMargin;
	
public:
	mat4<double>  modelMatrix;//gluUnprojectがdoubleしか対応してないから仕方なく。
	mat4<double> projMatrix;
	int m_ViewPort[4];/// gluUnprojectで使う。Reshapeで更新する。
	mat4<float> modelViewMatrix;
	vec4<float> m_Translate;//ただの平行移動値っぽいんだが
	double axisX;
	double axisY;
	double axisZ;              /* idle時の回転角(方向ベクトル) */
	double angleDelta;                 /* idle時の回転角(角速度)   */
	float eyeDis;
	//詳細度制御に必要
	vec2<int> m_WinSize;
	/*FPS測定・表示*/
	
	int miffytime;
	int timebase;
	double frameTime;
	int order;//描かれた順
	bool nowprocessingflag;//結果をファイル出力するための文字列作成のため
	Cg cg;
	bool showGridFlag;
	/*! @name デバッグウィンドウ制御用の変数
    */
    //@{
	frustum<float>::FrustumName frustumstate;/// サブウィンドウで表示する視錐台を切り替えるためのもの
	//@}

};

