#include "Lod.h"
void* font=GLUT_BITMAP_HELVETICA_10;
/// @name ライティング関係
  ///@{
	float globalAmbient[3] = {0.5f, 0.5f, 0.5f};
	vec4<double> lightAPosition(0.0, 0.0, 20.0, 1.0);
	vec4<double>  lightPosition;
	float lightColor[3] = {1.0f, 1.0f, 1.0f};

	float Ke[3] = {0.0f, 0.0f, 0.0f};
	float Ka[3] = {0.2f, 0.2f, 0.2f};
	float Kd[3] = {0.7f, 0.7f, 0.7f};
	float Ks[3] = {0.9f, 0.9f, 0.9f};
	float shininess = 50.0f;
	///@}
	void resetPerspectiveProjection() {
	// set the current matrix to GL_PROJECTION
	glMatrixMode(GL_PROJECTION);
	// restore previous settings
	glPopMatrix();
	// get back to GL_MODELVIEW matrix
	glMatrixMode(GL_MODELVIEW);
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
static void check_framebuffer_status() 
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
/* 立方根を求める */
static double GetCbRoot(double x)
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
static void setOrthographicProjection(int _winwidth,int _winheght) {

	// switch to projection mode
	glMatrixMode(GL_PROJECTION);
	// save previous matrix which contains the 
	//settings for the perspective projection
	glPushMatrix();
	// reset matrix
	glLoadIdentity();
	// set a 2D orthographic projection
	gluOrtho2D(0, _winwidth, 0, _winheght);
	// invert the y axis, down is positive
	glScalef(1, -1, 1);
	// mover the origin from the bottom left corner
	// to the upper left corner
	glTranslatef(0, -(GLfloat)_winheght, 0);
	glMatrixMode(GL_MODELVIEW);
}
CLod::CLod(int _w,int _h)
	:m_Bclipresio(INITFAR / INITNEAR)
	, normalnbn(0)//描画に必要な現解像度ブロックの数
	, normalebn( 0)//既に存在している現解像度ブロックの数
	, backnbn(0)
	, backebn(0)
	, idealBlockNum(0)//最適解像度ブロックの数
	, backupOrNot(false)
	, renderBlockNum(0)//実際にレンダリングしているブロックの数
	,render_normal_num(0)
	,pinchBlockCount(0)//危ないブロック
	, bltime_sum(0)//1フレームごとの足し算
	, blcount(0)//1フレームごと
	,NFrustumSize(INITNFRUSTUMSIZE)
	,frame_special(0)
	,frame_special_occ(0)
	,frame_tex_pro(0)
	,tex_req_frame(0)
	,totalrendertime_sum(0)
	,renderblockcount(0)
	,loadtexblockcount(0)
	,angleDelta(0.0)
	,frame(0)
	,miffytime(0)
	,timebase(0)
	,order(0)
	,nowprocessingflag(false)
	,frustumstate( frustum<float>::render)
	,showGridFlag(false)
	,axisX(0.0)
	,axisY(0.0)
	,axisZ(0.0)
	
{
	m_WinSize.set(_w,_h);
	ini_cam.set(0.0f,0.0f,0.0f);
	ini_l.set(0.0f,0.0f,-1.0f);
	ini_u.set(0.0f,1.0f, 0.0f);
	m_wBasicCamPos.set(0.0,0.0,9.0);
}
void CLod::Idle(){
	
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix.m);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(m_Translate.x,m_Translate.y,m_Translate.z);     /* 平行移動(奥行き方向) 　　*/
	glMultMatrixd(modelMatrix.m);
	glRotated(angleDelta, axisX, axisY, axisZ);   /* 回転　　　　　　*/
	
	eyeDis -= m_Translate.z;
	SetClippingPlane();
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glutPostRedisplay();                       //Displayイベント発生 
}

CLod::~CLod(void)
{
}
vec3<float> CLod::getViewvector(vec3<float>* view_z){

	
	*view_z = inv_modelView*(*view_z);
	
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


	vec3<float> abview;

	abview.x = 1.0f/(abs(view_z->x)*SPX);
	abview.y = 1.0f/(abs(view_z->y)*SPY);
	abview.z = 1.0f/(abs(view_z->z)*SPZ);

	abview.normalize();
	return abview;
}
//読込み視錐台用 1フレーム先回りして読み込み要求発行。
void CLod::testLoadFrustum(Block block,int* octreeReso,int* existblcount,unsigned int** renderblock,bool **** needCountflag,bool **** existCountflag, deque<Block>* blockQue)
{
	
	int id = m_File.getIndexblock(block);//idは0か1の値 データが存在してたら１なんだと思うが。
	block.setBlockState(Block::notexist,renderblock);//配列renderblockにブロックステートを記録
	if(id != 0)//blockにデータが入っているか判定
	{
		//もしビューボリュームにblockが入っていれば
		vec3<float> corner;//cornerの成分は0か-1だ。
		corner.x = (2.0f*block.x-block.bnumx)*Block::brX*Block::iniX/block.bnumx;
		corner.y = (2.0f*block.y-block.bnumy)*Block::brY*Block::iniY/block.bnumy;
		corner.z = (2.0f*block.z-block.bnumz)*Block::brZ*Block::iniZ/block.bnumz;

		aabox<float> abox(corner,2.0f*Block::brX*Block::iniX/block.bnumx,2.0f*Block::brY*Block::iniY/block.bnumy,2.0f*Block::brZ*Block::iniZ/block.bnumz);//ブロックの情報をAxis Aligned Boxで表現している。AABoxのx,y,zはブロックの辺の長さ。
		if(m_NextFrustum.boxInFrustum(abox) != frustum<float>::OUTSIDE)//このブロックが視錐台の内側だったら。
		{
			if(block.resoChange(modelMatrix.m,projMatrix.m,m_WinSize.x,m_WinSize.y) && blockQue->size()<=g_MaxTexNum)//ボクセルのスクリーン上での大きさが1ピクセルの大きさよりも大きい場合にのに、実際に一段階高解像度のノードを探索する。
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

				if(m_File.blockExist(mlowblock))//main memory exist or not
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

void CLod::SetNextFrustum()
{
	float next_eyeDis = eyeDis;
	next_eyeDis -= m_Translate.z;
	float Next_nearSlice = next_eyeDis / ( 1.0f + Bclipeyeresio*( m_Bclipresio -1.0f));
	float Next_farSlice = Next_nearSlice * m_Bclipresio;
	

	glMatrixMode(GL_PROJECTION);               /* 投影変換の設定            */
	glPushMatrix();
	glLoadIdentity(); 
	gluPerspective(m_FovY, aspect, Next_nearSlice, Next_farSlice);
	
	float N_a = GetCbRoot(NFrustumSize);//立方根を求める
	float N_m = Next_nearSlice*(1.0f + m_Bclipresio)/2.0f;
	float N_l = Next_nearSlice*(m_Bclipresio - 1.0f)*N_a/2.0f;
	float next_fovy = atan(Next_nearSlice*N_a*tan(m_FovY*M_PI/180.0f)/(N_m - N_l))*180.0f/M_PI;//画角を導き出した
	double next_projMatrix[16];
	glGetDoublev(GL_PROJECTION_MATRIX, next_projMatrix);
	m_NextFrustum.setFromPerspective(next_projMatrix,next_fovy,aspect,N_m - N_l,N_m + N_l);	
	
}
void  CLod::SetRenderFrustum()
{
	glMatrixMode(GL_PROJECTION);               /* 投影変換の設定            */
	glLoadIdentity(); 
	dPlaneStart = eyeDis - nearSlice;     //スライス開始位置
	cg.SetParameter(cg.dPlaneStartParam , dPlaneStart);
	cg.SetParameter(cg.farSliceParam,farSlice); 
	gluPerspective(m_FovY, aspect, nearSlice, farSlice);
	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix.m);	
	m_RenderFrustum.setFromPerspective(projMatrix.m,m_FovY,aspect,nearSlice,farSlice);
	glMatrixMode(GL_MODELVIEW);

}
void CLod::ProcessNextResoLoad(Block bl,int* countA,int* countB,unsigned int** rblock,bool **** needCountflag,bool **** existCountflag,deque<Block>* blockQue)//NExt->次のより高解像度のやつって意味。
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
void CLod::DDAblock(vec3<float> viewvec, vec3<float> abview,Block bl,int* normalreso,int* backupreso,int* existblcount,deque<Block>* _blockQue)
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
					testRenderFrustum(block,normalreso,backupreso,existblcount,_blockQue);
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
					testRenderFrustum(block,normalreso,backupreso,existblcount,_blockQue);
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
void CLod::DDAblockFile(vec3<float> _viewvec, vec3<float> _abview,Block _bl,int* _reso,int* _exist_in_main_count,unsigned int** _blockstate,bool ****needCountMap,bool **** existInMainMap,deque<Block>* blockQue)
{// abviewとviewvecは対の関係みたいになってる。
	int DDAb[3];
	if(_bl.level == NUMLEVEL-1)
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
	if(_abview.x >= _abview.y && _abview.y >= _abview.z)
	{
		deltam = (int)(DDAb[2]*_abview.y/_abview.z);
		deltas =  DDAb[2];
		deltal = (int)(DDAb[2]*_abview.x/_abview.z);
		llength = DDAb[0];
		mlength = DDAb[1];
		slength = DDAb[2];
		num = 0;
	}
	else if(_abview.x >= _abview.z && _abview.z >= _abview.y)
	{

		deltam = (int)(DDAb[1]*_abview.z/_abview.y);
		deltas = DDAb[1];
		deltal = (int)(DDAb[1]*_abview.x/_abview.y);
		llength = DDAb[0];
		mlength = DDAb[2];
		slength = DDAb[1];
		num = 1;

	}
	else if(_abview.y >= _abview.z && _abview.z >= _abview.x)
	{


		deltam = (int)(DDAb[0]*_abview.z/_abview.x);
		deltas = DDAb[0];
		deltal = (int)(DDAb[0]*_abview.y/_abview.x);
		llength = DDAb[1];
		mlength = DDAb[2];
		slength = DDAb[0];
		num = 2;
	}
	else if(_abview.y >= _abview.x && _abview.x >= _abview.z)
	{

		deltam = (int)(DDAb[2]*_abview.x/_abview.z);
		deltas = DDAb[2];
		deltal = (int)(DDAb[2]*_abview.y/_abview.z);
		llength = DDAb[1];
		mlength = DDAb[0];
		slength = DDAb[2];
		num = 3;
	}
	else if(_abview.z >= _abview.x && _abview.x >= _abview.y)
	{

		deltam = (int)(DDAb[1]*_abview.x/_abview.y);
		deltas = DDAb[1];
		deltal = (int)(DDAb[1]*_abview.z/_abview.y);
		llength = DDAb[2];
		mlength = DDAb[0];
		slength = DDAb[1];
		num = 4;
	}
	else
	{

		deltam = (int)(DDAb[0]*_abview.y/_abview.x);
		deltas = DDAb[0];
		deltal = (int)(DDAb[0]*_abview.z/_abview.x);
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
	if(_viewvec.x >= 0.0 && _viewvec.y >= 0.0 && _viewvec.z >= 0.0)
	{
		sg[0]=1;sg[1]=1;sg[2]=1;
		vol[0]=0;vol[1]=0;vol[2]=0;
		vec_num = 0;
	}
	else if(_viewvec.x < 0.0 && _viewvec.y < 0.0 && _viewvec.z < 0.0)
	{
		sg[0]=-1;sg[1]=-1;sg[2]=-1;
		vol[0]=DDAb[0]-1;vol[1]=DDAb[1]-1;vol[2]=DDAb[2]-1;
		vec_num = 1;
	}
	else if(_viewvec.x >= 0.0 && _viewvec.y >= 0.0 && _viewvec.z < 0.0)
	{
		sg[0]=1;sg[1]=1;sg[2]=-1;
		vol[0]=0;vol[1]=0;vol[2]=DDAb[2]-1;
		vec_num = 2;
	}
	else if(_viewvec.x < 0.0 && _viewvec.y < 0.0 && _viewvec.z >= 0.0)
	{
		sg[0]=-1;sg[1]=-1;sg[2]=1;
		vol[0]=DDAb[0]-1;vol[1]=DDAb[1]-1;vol[2]=0;
		vec_num = 3;
	}
	else if(_viewvec.x < 0.0 && _viewvec.y >= 0.0 && _viewvec.z < 0.0)
	{
		sg[0]=-1;sg[1]=1;sg[2]=-1;
		vol[0]=DDAb[0]-1;vol[1]=0;vol[2]=DDAb[2]-1;
		vec_num = 4;
	}
	else if(_viewvec.x >= 0.0 && _viewvec.y < 0.0 && _viewvec.z >= 0.0)
	{
		sg[0]=1;sg[1]=-1;sg[2]=1;
		vol[0]=0;vol[1]=DDAb[1]-1;vol[2]=0;
		vec_num = 5;
	}
	else if(_viewvec.x < 0.0 && _viewvec.y >= 0.0 && _viewvec.z >= 0.0)
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
					Block block(vol[0]+sg[0]**x+_bl.x,vol[1]+sg[1]**y+_bl.y,vol[2]+sg[2]**z+_bl.z,_bl.level,_bl.bnumx,_bl.bnumy,_bl.bnumz);
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
					Block block(vol[0]+sg[0]**x+_bl.x,vol[1]+sg[1]**y+_bl.y,vol[2]+sg[2]**z+_bl.z,_bl.level,_bl.bnumx,_bl.bnumy,_bl.bnumz);
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

void CLod::ProcessNextResoRender(Block bl,int* countA,int* countB,int* countC,deque<Block>* blockQue)//1段階高解像度のブロックのノードを探索
{
	Block nextbl = bl.getHighblock();//仮に1段階高解像度のブロックのノードを探索
	vec3<float> vz = nextbl.getBlockVec(modelViewMatrix);
	vec3<float> ab = getViewvector(&vz);
	DDAblock(vz,ab,nextbl,countA,countB,countC,blockQue);
	bl.setBlockState(Block::nextreso,Block::rRequestBlock);
}
void CLod::initCountflag(int diff,bool **** needCountflag,bool **** existCountflag)
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
void  CLod::setFrontSlice(Cg& _cg)
{
	vec3<float> view_z(0.0,0.0,1.0);
	view_z = inv_modelView*view_z;


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
	_cg.SetParameter(_cg.frontIdxParam,(float)frontIdx);
	_cg.SetParameter(_cg.vecViewParam ,vecView );
}
void CLod::display(){
	//printf("描画開始,");
	frameTime=0;
	order=0;
	SetLighting();
	//読み取りよう視錐台のモデルビュー行列作成
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix.m);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(m_Translate.x,m_Translate.y, m_Translate.z);     /* 平行移動(奥行き方向) 　　*/
	glMultMatrixd(modelMatrix.m);	
	glRotated(angleDelta, axisX, axisY, axisZ);  
	glGetFloatv(GL_MODELVIEW_MATRIX,  Next_modelMatrix);//file threadで使う。
	glPopMatrix();
	//描画

	/*二つのFBOを初期化*/
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);//ここから、テクスチャRECTANGLEとして扱いたい描画内容のフレームバッファの内容を描く。
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMatrix.m);
	modelViewMatrix.inv(&inv_modelView);
	vec4<float> cam = inv_modelView*ini_cam;//ini_camは(0.0,0.0,0.0,1.0)
	float camera[3] = {cam.x,cam.y,cam.z};
	cg.SetParameter(cg.cameraParam,camera);
	vec3<float> lookat = inv_modelView*ini_l;//注視点
	vec3<float> upper = inv_modelView*ini_u;//視界の上方向を決めるupper vector
	
	m_RenderFrustum.setCamDef(cam.toVec3(),lookat,upper);//gluLookAtと同じ（カメラ、注視点、アッパーベクタ）　カメラパラメータから視錐台を構成する。　s_RenderFrustumはfrustum<float>クラス
	Block root_block = Block::GetRootBlock();  
	vec3<float> viewvec = root_block.getBlockVec(modelViewMatrix);
	setFrontSlice(cg);//消えても問題ない。
	vec3<float> abview = getViewvector(&viewvec);
	//これらが変である。
	
	//abview.print("abview");
	renderFrustum(viewvec,abview,root_block);//これを消したらボリュームとブロックの線が見えなくなる。ここに一番大事なレンダリング情報が入ってる。

}
void CLod::renderBlock(Block block)
{
	//block.printBlockInfo("ryacasting",0);
	//Occlusion Culling判定
	bool occludeState=block.testOcclusion(&cg);
	block.setOccludeState(!occludeState);//Block::occludedIndex[block.level][block.x][block.y][block.z]=!occludeState;
	if(occludeState)//もしアルファ値がthresholdよりも小さいフラグメントが存在するならtrue,存在しないならflaseが変える
	{
		Block lowResoblock = block;
		do{//do-whileループを外すとちらちら見えなくなるブロックが増える
			//初めにテクスチャメモリを探す
			if(m_File.texBlockExist(lowResoblock))
			{				

				m_File.setTexBlock(&cg,cg.vdecalParam,lowResoblock);//シェーダにテクスチャ名を渡したりする。setTextureParameterする。
				m_File.texBlockRequest(modelMatrix.m,projMatrix.m,lowResoblock,m_RenderFrustum,frustum<float>::render);
				int dif = lowResoblock.level - block.level;
			
				block.RayCast(&cg,dif);//ここでボリューム描画
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
void  CLod::Init(){
	/*//////////////////////////////////////////////////////////////
	テクスチャ1(高解像度ブロック×g_MaxTexNum）
	//////////////////////////////////////////////////////////////*/
	
	for(int i=0;i < g_MaxTexNum;i++)
	{
		glGenTextures(1, m_File.getTexaddress(i));
		glBindTexture(GL_TEXTURE_3D, m_File.getTexName(i));
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
		glTexImage3D(GL_TEXTURE_3D,0,GL_ALPHA,BLX+2,BLY+2,BLZ+2,0,GL_ALPHA, GL_FLOAT,NULL);//これがないとボリュームがなくなる。
		
	}//なぜ２のべき乗でなくて平気なのか？
	//glBindTexture(GL_TEXTURE_2D, 0);//これがなくても支障はない。
	//トータルレンダリング時間計測構造体初期化
	totalrender.totalrendertime = 0.0;
	totalrender.flag = false;
}
void CLod::renderFrustum(vec3<float> _viewvec, vec3<float> _abview,Block _rootBlock)//これがなくなると、ボリュームとブロックの線がなくなる。
{	
	
	bltime_perframe=0;
	bltime_sum=0;
	blcount=0;
	m_File.countTexLRU();//1フレーム終わるごとに優先度スコアを５ビット下げる

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
	if(totalrender.flag&& m_File.getTexLoadflag())
		MAXTEXLOAD = (int)((hopeframeLimitTime - totalrender.totalrendertime)/m_File.getTexLoadTime()) + ltn;//ltn=loadtexblockcount(1フレーム内でテクスチャロードしたブロックの数)
	else
		MAXTEXLOAD = MINTEXLOAD;//MINTEXLOAD=1 １回のフレームでテクスチャロードできる最少保障数
	
	if(MAXTEXLOAD < MINTEXLOAD)//MAXTEXLOADが１より少なかったら、
		MAXTEXLOAD = MINTEXLOAD;//MAXTEXLOADは１にする

	occblockcount = 0;
	maxRenderNum=(int)(hopeframeLimitTime/(g_RayCastTime/(float)rbn+g_OcculusionTime/(float)(normalnbn+obn)));
	//1フレーム内にレンダリングしてもよいブロック数
	
	//通常解像度octree追跡
	int renderNormalreso = s_normalreso;
	int renderBackreso = s_backupreso;
	int normalNeedInTex_ExistInMainBn = 0;//通常解像度で、メモリに既に四方こまれていて、まだテクスチャメモリに存在いない、これからロードする必要のあるもの
	rnRequestQueue.clear();
	render_normal_num=0;
	initCountflag(renderNormalreso,render_normal_blocks,r_existInMainMemNotInTexMem);//テクスチャメモリになくてメインメモリにあるブロック表がすべてfalseに初期化される
	//ここでrnRequestQueueにいろいろ詰め込まれる。
	DDAblock(_viewvec,_abview,_rootBlock,&renderNormalreso,&renderBackreso,&normalNeedInTex_ExistInMainBn,&rnRequestQueue);
	if(rnRequestQueue.empty()){assert(!"ouch!");}
	//printf("必要な現解像度ブロックの数=%d,これからロード[%d],MAXTEXLOAD[%d]\n",render_normal_num,normalNeedInTex_ExistInMainBn,MAXTEXLOAD);

	////ロードするブロック数が多かったらバックアップ解像度octree追跡
	if(normalNeedInTex_ExistInMainBn > MAXTEXLOAD || rnRequestQueue.size()>maxRenderNum)
	{
		backupOrNot=true;
		int backupNeedInTex_ExistInMainBn = 0;
		rbRequestQueue.clear();
		initCountflag(renderBackreso,render_normal_blocks,r_existInMainMemNotInTexMem);
		DDAblock(_viewvec,_abview,_rootBlock,&renderBackreso,&renderBackreso,&backupNeedInTex_ExistInMainBn,&rbRequestQueue);
		renderBlockNum=rbRequestQueue.size();
		if(backupNeedInTex_ExistInMainBn>=g_MaxTexNum){printf("変に上書きされる危険性あり\n");}
		//printf("バックアップlevel[%d][%d],これからロード[%d]\n",renderBackreso,rbRequestQueue.size(),backupNeedInTex_ExistInMainBn);
		rendernbnstr.str("");
		rendernbnstr<<"need blocks for whole render reso:[backup zone]"<<rbRequestQueue.size()<<"/"<<g_MaxTexNum<<"[g_MaxTexNum]";
		renderebnstr.str("");
		renderebnstr<<"exist in Main not in TexMem blocks of render reso:[backup zone]"<<backupNeedInTex_ExistInMainBn<<"/"<<g_MaxTexNum<<"[g_MaxTexNum]";
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
			if(!m_File.texBlockExist(bl) && m_File.blockExist(bl))
			{
				m_File.loadMainToTex(bl,&cg,cg.vdecalParam);//実際vdecalParamはつかってない。
				loadtexblockcount += 1;
				j++;
			}
			renderBlock(_rootBlock);//ここを消してもあまり結果が変わらない。
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
			if(!m_File.texBlockExist(bl) && m_File.blockExist(bl))
			{
				m_File.loadMainToTex(bl,&cg,cg.vdecalParam);
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
		rendernbnstr<<"need blocks of render reso:[normal zone]"<<rnRequestQueue.size()<<"/"<<g_MaxTexNum<<"[g_MaxTexNum]";
		renderebnstr.str("");
		renderebnstr<<"exist in Main not in TexMem blocks of render reso:[normal zone]"<<normalNeedInTex_ExistInMainBn<<"/"<<g_MaxTexNum<<"[g_MaxTexNum]";
		renderresostr.str("");
		renderresostr<<"render reso level:"<<renderNormalreso<<"(normalzone)";
		//ブロック描画
	//	printf("MAXTEXLOAD=%d,キューサイズ=%d,ピンチブロック=%d\n",MAXTEXLOAD,rnRequestQueue.size(),pinchBlockCount);
		if(rnRequestQueue.empty()){printf("現解像度zone呼び出す前から空だ\n");abort();}
		while(!rnRequestQueue.empty())//呼び出し先の両端キューが空の場合はtrueを返し、そうでない場合はfalseを返す
		{
			LARGE_INTEGER blbe,blaf,blfr;
			QueryPerformanceCounter(&blbe);
			QueryPerformanceFrequency(&blfr);
			Block bl = rnRequestQueue.front();//両端キューの先頭への参照を返す。
			if(!m_File.texBlockExist(bl))
			{
				Block backup = bl.getMultiLowblock(s_backupreso-s_normalreso);
				if(m_File.blockExist(bl))
				{
					m_File.loadMainToTex(bl,&cg,cg.vdecalParam);
					loadtexblockcount += 1;
				}
				else if(m_File.blockExist(backup) && !m_File.texBlockExist(backup))
				{
					m_File.loadMainToTex(backup,&cg,cg.vdecalParam);
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

				m_OcclutionNum.str("");
				obn = temp_occnum/count;
				m_OcclutionNum<<"number of culling blocks at last frame:"<<obn;
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

void CLod::testRenderFrustum(Block _block,int* Preso,int* Sreso,int* NeedInTex_ExistInMainBlcount,deque<Block>* _blockQue)//DDAブロックメソッドの中で使われる
{//Preso=normalreso,Sreso=backupreso
	//blockにデータが入っているか判定
	int id = m_File.getIndexblock(_block);
	_block.setBlockState(Block::notexist,Block::rRequestBlock);
	if(id != 0)//そのブロックにちょっとでもデータが入ってれば
	{
		//もしビューボリュームにblockが入っていれば
		vec3<float> corner;
		corner.x = (2.0f*_block.x-(float)_block.bnumx)*Block::brX*Block::iniX/(float)_block.bnumx;
		corner.y = (2.0f*_block.y-(float)_block.bnumy)*Block::brY*Block::iniY/(float)_block.bnumy;
		corner.z = (2.0f*_block.z-(float)_block.bnumz)*Block::brZ*Block::iniZ/(float)_block.bnumz;
		
		aabox<float> abox(corner,2.0f*Block::brX*Block::iniX/_block.bnumx,2.0f*Block::brY*Block::iniY/_block.bnumy,2.0f*Block::brZ*Block::iniZ/_block.bnumz);
		int ret=m_RenderFrustum.boxInFrustum(abox);
		if(ret != frustum<float>::OUTSIDE)//視錐台の中に入ってたら いつもOUTSIDEと判定される。
		{
			if(_block.resoChange(modelMatrix.m,projMatrix.m,m_WinSize.x,m_WinSize.y) &&_blockQue->size()<=g_MaxTexNum)//1ボクセルの大きさが、1ピクセルよりも大きいかどうかチェック。大きかったらtrue
			{
				ProcessNextResoRender(_block,Preso,Sreso,NeedInTex_ExistInMainBlcount,_blockQue);//実際に1段階高解像度のノードを探索
			}
			else{  //1ボクセルの大きさが、1ピクセルよりも小さい場合　or すべてが最適解像度ブロックになったら
				//何段階か上	
				Block Pblock = _block.getMultiLowblock(*Preso);
				if(!render_normal_blocks[Pblock.level][Pblock.x][Pblock.y][Pblock.z])
				{	
					_blockQue->push_back(Pblock);//Pblockと同じ値の要素を両端キューの末尾に追加する。
					render_normal_blocks[Pblock.level][Pblock.x][Pblock.y][Pblock.z] = true;
					render_normal_num++;//miffy added
				}

				
				if(!m_File.texBlockExist(Pblock))//テクスチャメモリになく
				{
					if(m_File.blockExist(Pblock))//メインメモリにあるブロックを探す
					{
						if(!r_existInMainMemNotInTexMem[Pblock.level][Pblock.x][Pblock.y][Pblock.z])
						{
							*NeedInTex_ExistInMainBlcount += 1;
							r_existInMainMemNotInTexMem[Pblock.level][Pblock.x][Pblock.y][Pblock.z] = true;
						}
					}
					else if(*Preso != *Sreso)//テクスチャメモリにもメインメモリにもないブロックは低解像度にする
					{
						Block Sblock = _block.getMultiLowblock(*Sreso);
						if(m_File.blockExist(Sblock) && !m_File.texBlockExist(Sblock))//低解像度にしたけど、テクスチャメモリになくてメインメモリにあるか
						{
							if(!r_existInMainMemNotInTexMem[Sblock.level][Sblock.x][Sblock.y][Sblock.z])
							{
								*NeedInTex_ExistInMainBlcount += 1;
								r_existInMainMemNotInTexMem[Sblock.level][Sblock.x][Sblock.y][Sblock.z] = true;
							}
						}else if(m_File.blockExist(Sblock)){Sblock.renderBlockLines(1.0,0.0,1.0,1.0);//ピンク（バックアップ解像度がメインメモリにすらないピンチなブロック）
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
			_block.setBlockState(Block::frameout,Block::rRequestBlock);
		}
	}//そのブロックにちょっとでもデータが入ってるかどうか
}
///どんな様子かを俯瞰図で見るためのもの
void CLod::renderDebugWindow(){
	
		unsigned int** rb;//blockstate
		frustum<float> fs;
		if(frustumstate == frustum<float>::normal)//この視錐台はノーマル
		{
			rb = Block::nRequestBlock;
			fs = m_NextFrustum;
		}
		else if(frustumstate == frustum<float>::back)//この視錐台はバックアップ用
		{
			rb = Block::bRequestBlock;
			fs = m_NextFrustum;
		}
		else//この視錐台はレンダリング用
		{	
			rb = Block::rRequestBlock;//ピンク色
			fs = m_RenderFrustum;
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

		if(m_WinSize.x >= m_WinSize.y)
			glViewport(m_WinSize.x - m_WinSize.y/2,0, m_WinSize.y/2, m_WinSize.y/2);
		else
			glViewport(m_WinSize.x - m_WinSize.x/2,0, m_WinSize.x/2, m_WinSize.x/2);



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
		m_NextFrustum.drawLines();//	ファイル用視錐台を緑で描画
		glColor4f(1.0,1.0,1.0,1.0);

		glMultMatrixf(inv_modelView.m);

		//視点描画
		glRotatef(180.0,0.0,1.0,0.0);
		glTranslatef(0.0,0.0,-0.5);
		glColor3f(0.0,1.0,0.0);
		glutSolidCone(0.05,0.5,5,5);


		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();


		glViewport(0, 0, m_WinSize.x, m_WinSize.y);



	

}
void CLod::requestNormalFrustum(vec3<float> viewvec,vec3<float> abview,Block block)//file threadで使う
{	nowprocessingflag=true;
	s_normalreso = -1;//LowBlockを作る回数に関わる変数。
	int counter=0;
	do//ここが重要！！　nRequestQueue.size() >g_MaxTexNumになったらやめる。
	{
		s_normalreso++;
		normalebn = 0;//exist blocks of normal reso
		nRequestQueue.clear();//両端キューからすべての要素を削除する。
		initCountflag(s_normalreso,n_needInMain,n_existInMain);
//level4の計算してるっぽい。初期状態を作ってる。
		
		DDAblockFile(viewvec,abview,block,&s_normalreso,&normalebn,Block::nRequestBlock,n_needInMain,n_existInMain,&nRequestQueue);
		//testLoadFrustum(block,&normalreso,&normalebn,Block::nRequestBlock,n_needInMain,n_existInMain,&nRequestQueue);
		
		if(counter==0){idealBlockNum=nRequestQueue.size();}
		counter++;
		if(nRequestQueue.size()>g_MaxTexNum){//多すぎたらテクスチャ圧縮処理
			printf("miffyの圧縮処理発動\n");
			int difference=nRequestQueue.size()-g_MaxTexNum;
			int sum=0;
			while(sum>=difference){
			Block bl=nRequestQueue.back();
			sum+=m_File.getTexCompressInfo(bl);
			nRequestQueue.pop_back();//仲間も外さなきゃいかん
			nRequestQueue.push_back(bl.returnParent());
			}
		}
	}while(s_normalreso < NUMLEVEL-1 && nRequestQueue.size() >g_MaxTexNum);
	normalnbn = nRequestQueue.size();//描画に必要な現解像度ブロックの数
	normalnbnstr.str("");
	normalnbnstr<<"need blocks of normal reso:(int this scene)"<<normalnbn<<"/"<<MAXBNUM<<"(MAXBNUM)";//描画に必要な現解像度ブロックの数
	normalebnstr.str("");
	normalebnstr<<"exist in Main blocks of normal reso:"<<normalebn<<"/"<<MAXBNUM<<"(MAXBNUM)";

	normalresostr.str("");
	normalresostr<<"normal reso level:"<<s_normalreso;

	while(!nRequestQueue.empty())//呼び出し先の両端キューが空の場合はtrueを返し、そうでない場合はfalseを返す。
	{/*(Block)nRequestQueue.front()がまだreqqueに入れられてないなら空いてる場所orreqqueが満杯なら優先度の一番低いところに入れる。(Block)nRequestQueue.front()がもうreqque、またはメインメモリに転送済みにあるなら優先度を更新する*/
		m_File.mainBlockRequest(modelMatrix.m,projMatrix.m,(Block)nRequestQueue.front(),m_NextFrustum,frustum<float>::normal);
		nRequestQueue.pop_front();//両端キューの最初の要素を削除する。
	}

	//論文のp35 (file.getThreadTime()=ファイルスレッドのwhile1ループにかかる時間
	BACKUPTHRE = (int)((m_File.getThreadTime()/m_File.getMainLoadTime())*HDMAINALPHA);
	//BACKUPTHRE = (int)((totalrender.totalrendertime/file.getMainLoadTime())*HDMAINALPHA);
	//論文通りだとこっちが正しいと思うが。。。
	//BACKUPTHRE=1フレームあたりに主メモリに読み込むことが出来るブロックの最大数
	if(BACKUPTHRE == 0)
		BACKUPTHRE = 1;
	/*バックアップブロックの解像度調整*/
	s_backupreso = s_normalreso;//論文p34
	/*unsigned int*/ BBLoadMargin = MAXBNUM - normalnbn;//normalnbn=描画に必要な現解像度ブロックの数
	 if(normalnbn>=MAXBNUM){BBLoadMargin=0;}
	//BBLoadMargin=バックアップブロックを読み込んでもよい数
	
	do
	{
		s_backupreso++;//0-4の値をとるみたい
		backebn = 0;
		bRequestQueue.clear();//両端キューからすべての要素を削除する。
		initCountflag(s_backupreso,b_needInMain,b_existInMain);
		DDAblockFile(viewvec,abview,block,&s_backupreso,&backebn,Block::bRequestBlock,b_needInMain,b_existInMain,&bRequestQueue);
			
		
	}while(s_backupreso < NUMLEVEL-1 && (bRequestQueue.size() > BBLoadMargin || (bRequestQueue.size()-backebn) > BACKUPTHRE));
	
	//論文35ページあたり                               式４．４                  　式4.6（主メモリに読み込まなくてはいけないバックアップブロックの数＜1フレーム内にloadFile()してもよいブロックの個数）
	//なぜ　BACKUPTHRE　を小さくすると、「解像度変更後のバックアップブロックの読み込み量を抑えることが出来る」のか？
	// printf("backupreso[%d],bRequestQue.size(%d),backebn[%d],BBLoadMArgin[%d],BACKUPTHRE[%d]\n",backupreso,bRequestQueue.size(),backebn,BBLoadMargin,BACKUPTHRE);

	backnbnstr.str("");
	backnbnstr<<"need in Main blocks of backup reso:"<<bRequestQueue.size()<<"/"<<MAXBNUM;
	backnbn = bRequestQueue.size();
	
	backebnstr.str("");
	backebnstr<<"exist in Main blocks of backup reso:"<<backebn<<"/"<<MAXBNUM;

	backresostr.str("");
	backresostr<<"backup reso level:"<<s_backupreso;

	maxloadmainblocknum.str("");
	maxloadmainblocknum<<"max number of main load blocks:per 1frame[BACKUTHRE]"<<BACKUPTHRE;//filethreadのwhile1ループあたりに読み込めるブロック数の最大数

	filethreadtime.str("");
	filethreadtime<<"file thead time:(1 while)"<<m_File.getThreadTime();
	
	while(!bRequestQueue.empty())
	{/*bRequestQueue.front()がまだreqqueに入れられてないなら空いてる場所orreqqueが満杯なら優先度の一番低いところに入れる。bRequestQueue.front()がもうreqque、またはメインメモリに転送済みにあるなら優先度を更新する*/
		m_File.mainBlockRequest(modelMatrix.m,projMatrix.m,(Block)bRequestQueue.front(),m_NextFrustum,frustum<float>::back);
		bRequestQueue.pop_front();
	}
	
}
void CLod::SetClippingPlane(){
	nearSlice = eyeDis / ( 1.0f + Bclipeyeresio*( m_Bclipresio -1.0f));
	farSlice = nearSlice * m_Bclipresio;
	SetRenderFrustum();
	SetNextFrustum();
}
/// だんだんとただのタイトルバー出力にとどまらない機能を持ってきた。
void CLod::PrintInfoOnWindow(){
	
	maintotextime=m_File.getMainToTexTime();
		if(infoflag)
		{
			//if(mflag)
			//{
				frame_special++;
			//}
		}
	stringstream ssmessage;
	ssmessage<<"最適解像度"<<idealBlockNum<<" backup?"<<backupOrNot<<" レンダリングブロック数"<<renderBlockNum;
	glutSetWindowTitle((char*)ssmessage.str().c_str());//ウィンドウタイトルバーに文字を表示
	
}
void CLod::FileInit(string _path, string _dataname){
	m_File.Init(_path,_dataname);
	m_File.initMainLoad();//最初にメインメモリにいっぱいデータを載せておくか？
	
}
void CLod::Reshape(int _w,int _h){
	m_WinSize.set(_w,_h);
	glMatrixMode(GL_MODELVIEW);

	//FBO作り直し
	createFBO();

	glLoadIdentity();
	gluLookAt(m_wBasicCamPos.x, m_wBasicCamPos.y, m_wBasicCamPos.z,0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	/* 視点と注視点の指定 */
	angleDelta = 0.0;   
	m_Translate.x = 0.0;
	m_Translate.y = 0.0;
	m_Translate.z = 0.0;  
	widthstring.str("");
	widthstring<<"window width:"<<_w;
	heightstring.str("");
	heightstring<<"window height:"<<_h;

	aspect = (float)_w/(float)_h;

	if( _w < _h ){
		m_FovY = atan(tan(M_PI/12.0f)*_h/_w)*360.0f/M_PI;
	}
	else {
		m_FovY = 30.0;
	}
	eyePosition=modelMatrix*m_wBasicCamPos;
	eyeDis = (float)m_wBasicCamPos.z;
	lightPosition=modelMatrix*lightAPosition;
               
	glViewport(0, 0, _w,_h);                    /* Window上での描画領域設定  */
	glGetIntegerv(GL_VIEWPORT,m_ViewPort);//あとでgluUnProjectで使う。
	//ブロック前後クリッピング初期化
	nearSlice = INITNEAR;
	farSlice  = INITFAR;
	Bclipeyeresio = (eyeDis-nearSlice)/(farSlice - nearSlice);
}
void CLod::PrintRenderInfo()
{
	if(infoflag)
	{

		setOrthographicProjection(m_WinSize.x,m_WinSize.y);
		glPushMatrix();
		glLoadIdentity();


		glColor4f(0.0,0.0,0.0,0.5);
		glBegin(GL_QUADS);  
		glVertex2f(0.0,0.0);
		glVertex2f(320.0,0.0);
		glVertex2f(320.0,(GLfloat)m_WinSize.y);
		glVertex2f(0.0,(GLfloat)m_WinSize.y);
		glEnd();




		float aveg_RayCastTime = g_RayCastTime/(float)rbn;
		blockrendertime.str("");
		blockrendertime<<"avarage render each block time [msec]:"<<aveg_RayCastTime;

		//cout<<"render time:"<<g_RayCastTime<<endl;


		float aveg_OcculusionTime = g_OcculusionTime/(float)renderBlockNum;//←こっちが正しいと思う((float)normalnbn+obn);
		//オクルージョンクエリーにかかる時間　normalnbn=//描画に必要な現解像度ブロックの数
		oquerytime.str("");
		oquerytime<<"occlution query time [msec]:"<<aveg_OcculusionTime;

		//cout<<"average occlution time:"<<g_OcculusionTime<<endl;

		float aveg_TexProcessTime = g_TexProcessTime/(float)rbn;
		//CPUからGPUへのテクスチャ転送時間．
		tptime.str("");
		tptime<<"avarage texture process time [msec]:"<<aveg_TexProcessTime;
		
		//cout<<"texture time:"<<g_TexProcessTime<<endl;

		float aveg_TexRequestTime = g_TexRequestTime/(float)rbn;
		//loadMainToTexにかかる時間
		trtime.str("");
		trtime<<"avarage texture request time [msec]:"<<aveg_TexRequestTime;

		//cout<<"texture request time:"<< g_TexRequestTime<<endl;

		

		//
		// meter
		//
		float meterxorigin = 15.0;
		float meterlength = 285.0;
		float meteryorigin = 530.0;
		float meterwidth = 10.0;


		//比率計算
		float rcresio = g_RayCastTime;
		float ocresio = g_OcculusionTime;
		float tpresio = g_TexProcessTime;
		float trresio = g_TexRequestTime;
		

		float sumresio = rcresio + ocresio + tpresio + trresio;

		rcresio = rcresio/sumresio;
		ocresio = ocresio/sumresio;
		tpresio = tpresio/sumresio;
		trresio = trresio/sumresio;
		


		//raycasting time
		MEDIUM_SEA_GREEN.glColor();
		float rcend = meterxorigin + rcresio*meterlength;
		glBegin(GL_QUADS);  
		glVertex2f(meterxorigin,meteryorigin);
		glVertex2f(rcend,meteryorigin);
		glVertex2f(rcend,meteryorigin + meterwidth);
		glVertex2f(meterxorigin,meteryorigin + meterwidth);
		glEnd();

		//occlution culling time
		MEDIUM_BLUE.glColor();
		float ocorigin = rcend;
		float ocend = ocorigin + ocresio*meterlength;
		glBegin(GL_QUADS);  
		glVertex2f(ocorigin,meteryorigin);
		glVertex2f(ocend,meteryorigin);
		glVertex2f(ocend,meteryorigin + meterwidth);
		glVertex2f(ocorigin,meteryorigin + meterwidth);
		glEnd();


		//texture process time
		PINK.glColor();
		float tporigin =  ocend;
		float tpend = tporigin + tpresio*meterlength;
		glBegin(GL_QUADS);  
		glVertex2f(tporigin,meteryorigin);
		glVertex2f(tpend,meteryorigin);
		glVertex2f(tpend,meteryorigin + meterwidth);
		glVertex2f(tporigin,meteryorigin + meterwidth);
		glEnd();

		//texture request time
		SILVER.glColor();
		float trorigin =  tpend;
		float trend = trorigin + trresio*meterlength;
		glBegin(GL_QUADS);  
		glVertex2f(trorigin,meteryorigin);
		glVertex2f(trend,meteryorigin);
		glVertex2f(trend,meteryorigin + meterwidth);
		glVertex2f(trorigin,meteryorigin + meterwidth);
		glEnd();


		MEDIUM_PURPLE.glColor();
		renderBitmapString(15,15,font,(char*)g_FileLoadTime.str().c_str()); 
		renderBitmapString(15,35,font,(char*)g_TexLoadTime.str().c_str()); 
		MEDIUM_SPRING_GREEN.glColor();
		renderBitmapString(15,55,font,(char*)blockrendertime.str().c_str()); 
		renderBitmapString(15,75,font,(char*)oquerytime.str().c_str());
		renderBitmapString(15,95,font,(char*)tptime.str().c_str());
		renderBitmapString(15,115,font,(char*)trtime.str().c_str());
		renderBitmapString(15,135,font,(char*)mrtime.str().c_str());
		PLUM.glColor();
		renderBitmapString(15,195,font,(char*)fps.str().c_str());
		ALICE_BLUE.glColor();
		renderBitmapString(15,215,font,(char*)normalnbnstr.str().c_str());
		renderBitmapString(15,235,font,(char*)normalebnstr.str().c_str());
		renderBitmapString(15,255,font,(char*)backnbnstr.str().c_str());
		renderBitmapString(15,275,font,(char*)backebnstr.str().c_str());
		renderBitmapString(15,295,font,(char*)rendernbnstr.str().c_str());
		renderBitmapString(15,315,font,(char*)renderebnstr.str().c_str());
		renderBitmapString(15,335,font,(char*)normalresostr.str().c_str());
		renderBitmapString(15,355,font,(char*)backresostr.str().c_str());
		renderBitmapString(15,375,font,(char*)renderresostr.str().c_str());
		
		MEDIUM_SEA_GREEN.glColor();
		renderBitmapString(15,395,font,(char*)m_OcclutionNum.str().c_str());
		renderBitmapString(15,415,font,(char*)renderblocknum.str().c_str());
		renderBitmapString(15,435,font,(char*)loadtexblocknum.str().c_str());
		renderBitmapString(15,455,font,(char*)maxloadtexblocknum.str().c_str());
		renderBitmapString(15,475,font,(char*)maxloadmainblocknum.str().c_str());
		renderBitmapString(15,495,font,(char*)filethreadtime.str().c_str());
		PEACH_PUFF.glColor();
		renderBitmapString(15,515,font,(char*)widthstring.str().c_str());
		renderBitmapString(15,535,font,(char*)heightstring.str().c_str());
		glPopMatrix();
		resetPerspectiveProjection();

		frame++;
		////メーター初期化
		//g_RayCastTime = 0.0;
		//g_OcculusionTime  = 0.0;
		//g_TexProcessTime = 0.0;
		//g_TexRequestTime = 0.0;
		//g_MemRequestTime = 0.0;
	}

}
void CLod::Translate(float _shift){
	nearSlice -= _shift;
	farSlice = m_Bclipresio * nearSlice;
	Bclipeyeresio = (eyeDis-nearSlice)/(farSlice-nearSlice);
	SetRenderFrustum();
	SetNextFrustum();
}
void CLod::FileRun(){
	m_File.countMainLRU();
	m_File.countThreadTime();/*ハードディスク読込み時間測定用*/
	memcpy(Next_modelViewMatrix.m,Next_modelMatrix,sizeof(float)*16);
	//Next_modelMatrix[]はdisplay関数でゲットしたもの。
	Next_modelViewMatrix.inv(&Next_inv_modelView);	
	vec4<float> N_cam = Next_inv_modelView*ini_cam;
	vec3<float> N_l = Next_inv_modelView*ini_l;
	vec3<float> N_u = Next_inv_modelView*ini_u;
	//printf(",set file,");
	m_NextFrustum.setCamDef(N_cam.toVec3(),N_l,N_u);
	//N_cam.print("file");
	Block block=Block::GetRootBlock();
	vec3<float> vz=block.getBlockVec(Next_modelViewMatrix);
	vec3<float> abview = getViewvector(&vz);
	requestNormalFrustum(vz,abview,block);//ここで、必要な現解像度ブロック、バックアップ解像度ブロックの数とかがわかる。
	m_File.loadHDToMain();
	m_File.CheckLoadComplete();
		
}
void CLod::FileTexInit(){
	m_File.initTexLoad();//最初にテクスチャメモリにデータをいっぱい載せるか？
	
}
void CLod::Exit(){
	///*if(frame_special==0 ){frame_special=1;}
	//if(frame_special_occ==0){frame_special_occ=1;}
	//if(tex_req_frame==0){tex_req_frame=1;}
	//if(frame_tex_pro==0){frame_tex_pro=1;}*/
	//stringstream filename;
	//filename<<"texmemlog_alpha"<<HDMAINALPHA*10<<"_tex"<<g_MaxTexNum<<"fopeFPS"<<TEXLOADPARAMETER<<"win"<<winwidth<<"height"<<winheight<<".txt";
	//ofstream outPutFile(filename.str().c_str());
	//outPutFile.setf(ios_base::fixed,ios_base::floatfield);
	//outPutFile.precision(5);
	///*float aveg_RayCastTime = g_RayCastTime/(float)frame_special;
	//float aveg_OcculusionTime = g_OcculusionTime/((float)frame_special);
	//float aveg_TexRequestTime = g_TexRequestTime/(float)frame_special;
	//float averageg_TexProcessTime=g_TexProcessTime/(float)frame_special;
	//float averagetotalrendertime=totalrendertime_sum/(float)frame;*/
	////float averageg_MemRequestTime=g_MemRequestTime/(float)frame;
	////outPutFile<<"g_RayCastTime,g_OcculusionTime,g_TexProcessTime,g_TexRequestTime,totalrendertime,FPS\n";
	////outPutFile<<aveg_RayCastTime<<","<<aveg_OcculusionTime<<","<<averageg_TexProcessTime<<","<<aveg_TexRequestTime<<","<<averagetotalrendertime<<","<<fpsresult<<"\n";
	//outPutFile<<mainmemlog.str().c_str();
	////outPutFile<<"FPS,"<<fpsresult<<",MAXBNUM,"<<MAXBNUM<<",g_MaxTexNum,"<<g_MaxTexNum<<",alpha,"<<HDMAINALPHA<<"\n";
	////outPutFile<<"notexist,renderpreblock,frameout,nextreso,waitblock,renderfirstlowblock,rendersecondlowblock,renderthirdlowblock,renderfourthlowblock,occlusionculling\n";
	////outPutFile<<g_BlockStateCount[0]<<","<<g_BlockStateCount[1]<<","<<g_BlockStateCount[2]<<","<<g_BlockStateCount[3]<<","<<g_BlockStateCount[4]<<","<<g_BlockStateCount[5]<<","<<g_BlockStateCount[6]<<","<<g_BlockStateCount[7]<<","<<g_BlockStateCount[8]<<","<<g_BlockStateCount[9]<<"\n";
	////printf("notexist,renderpreblock,frameout,nextreso,waitblock,renderfirstlowblock,rendersecondlowblock,renderthirdlowblock,renderfourthlowblock,occlusionculling\n");
	///*for(int i=0;i<10;i++){
	//	printf("%d,",g_BlockStateCount[i]);
	//}*/
	/*stringstream filename2;
	filename2<<"blocktimelog_preTexLoad_alpha"<<HDMAINALPHA*10<<"_tex"<<g_MaxTexNum<<"fopeFPS"<<TEXLOADPARAMETER<<"win"<<winwidth<<"height"<<winheight<<".txt";
	ofstream outPutFile2(filename2.str().c_str());
	outPutFile2.setf(ios_base::fixed,ios_base::floatfield);
	outPutFile2.precision(5);
	outPutFile2<<timelog.str().c_str();
	outPutFile2.close();*/
	//outPutFile2<<mainmemParameterLog.str().c_str();
	//outPutFile<<g_RayCastTime<<","<<g_OcculusionTime<<","<<g_TexProcessTime<<","<<g_TexRequestTime<<","<<g_MemRequestTime<<"\n";
	
	m_File.deleteMemory();
}

void CLod::InitBflag()
{
	//とりあえず場所がよかったのでここに置いた。
	timelog<<"平均処理時間,テクスチャロードした割合,総ブロック数\n";
	
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
void CLod::destroyFBO()
{
	glDeleteFramebuffers(1,&fb);
	glDeleteTextures(1, &fbTex);
	glDeleteTextures(1, &fbDep);
	fb = 0;
	fbTex = 0;
	fbDep = 0;
}
void CLod::createFBO(){
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

	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB,0,GL_RGBA,m_WinSize.x,m_WinSize.y,0,GL_RGBA,GL_FLOAT,NULL);//これがないと画面が白くなる。
	 /* フレームバッファオブジェクトに２Dのテクスチャオブジェクトを結合する */
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB,fbTex, 0 );//これがないとボリュームがなくなる。
	glBindTexture(GL_TEXTURE_2D,0);
	  /* フレームバッファオブジェクトの結合を解除する */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	cgGLSetTextureParameter(cg.fdecalParam,fbTex);
	cgGLSetTextureParameter(cg.fdecal2Param,fbTex);//なくても問題ない。


	glGenTextures(1, &fbDep);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB,fbDep);

	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	//use bilinear filtering
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB,0,GL_DEPTH_COMPONENT24,m_WinSize.x,m_WinSize.y,0,GL_DEPTH_COMPONENT,GL_INT,NULL);//これがないとボリュームがなくなる。
	//テクスチャをフレームバッファにアタッチします。
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB,fbDep, 0 );

	glBindTexture(GL_TEXTURE_2D,0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	check_framebuffer_status();
}
void CLod::SetLighting(void) {                      /* ライトの指定 　　　　　　*/
	cg.SetMatrixParameter(cg.matModelViewNormalParam,CG_GL_MODELVIEW_MATRIX,CG_GL_MATRIX_INVERSE_TRANSPOSE);
	cg.SetParameter(cg.lightColorParam,lightColor);
	cg.SetParameter(cg.lightPositionParam,&lightPosition.x);
	cg.SetParameter(cg.globalAmbientParam,globalAmbient);
	cg.SetParameter(cg.eyePositionParam,&eyePosition.x);
	cg.SetParameter(cg.KeParam,Ke);
	cg.SetParameter(cg.KaParam,Ka);
	cg.SetParameter(cg.KdParam,Kd);
	cg.SetParameter(cg.KsParam,Ks);
	cg.SetParameter(cg.shininessParam,shininess);
}
void CLod::PasteFBOTexture(){
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	cgGLEnableTextureParameter(cg.transfer_functionParam);	cg.CheckCgError();
	
	// //FBO解除
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

	glTexCoord2i(0,m_WinSize.y);glVertex2f(-1.0, 1.0);

	glTexCoord2i(m_WinSize.x,m_WinSize.y);glVertex2f(1.0,1.0);

	glTexCoord2i(m_WinSize.x,0);glVertex2f(1.0, -1.0);
	glEnd();
	glDisable(GL_TEXTURE_RECTANGLE);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, 0);//これを消してもレンダリング結果は変わらない。
}
