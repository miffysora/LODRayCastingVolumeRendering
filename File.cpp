#include <GL/glew.h>
#include "File.h"
#include "math.h"

using namespace std;

File::File() {}

File::~File() {}

void File::Init(string _path, string _dataname){
  
	
	m_Path = _path;
	m_DataName = _dataname;


	//配列ポインタ作成（NULL初期化)
	this->bs = new bstruct ***[NUMLEVEL];
	this->bstex = new btexstruct ***[NUMLEVEL];
	


	for(int a= 0; a < NUMLEVEL;a ++)
	{
		int num = (int)pow(2.0,(NUMLEVEL-1.0)-a);//
		int numx = INIBLX*num;
		int numy = INIBLY*num;
		int numz = INIBLZ*num;


		this->bs[a] = new bstruct **[numx];
		this->bstex[a] = new btexstruct **[numx];

	
		//indexblockファイル読み出し
		indexblock[a] = new unsigned int[numx*numy*numz];
		stringstream indexblockfile;
		indexblockfile<<m_Path<<m_DataName<<"_"<<BLX<<"_"<<BLY<<"_"<<BLZ<<"/"<<"level"<<a<<"/"<<"VertebralBody"<<"IndexBlock-"<<a<<".dff";
		//static string path =  "../../";
		//static string dataname = "VertebralBody_Brick";
		
		loadIndexFile(indexblockfile.str(),indexblock[a],2,numx,numy,numz);
		
		if(a>0){texCompressInfo[a-1]= new unsigned int[numx*numy*numz];
		stringstream texCompressfile;
		texCompressfile<<m_Path<<m_DataName<<"_"<<BLX<<"_"<<BLY<<"_"<<BLZ<<"/"<<"level"<<a<<"/"<<"VertebralBody"<<"TexCompress-"<<a<<".dff";
		loadIndexFile(texCompressfile.str(),texCompressInfo[a-1],0,numx,numy,numz);
		}
		//サブ画面用ブロック確保
		Block::nRequestBlock[a] = new unsigned int[numx*numy*numz];
		Block::bRequestBlock[a] = new unsigned int[numx*numy*numz];
		Block::rRequestBlock[a] = new unsigned int[numx*numy*numz];//様々なブロックステートを保持する


		for(int i=0 ;i < numx;i++)
		{
			this->bs[a][i] = new bstruct *[numy];
			this->bstex[a][i] = new btexstruct *[numy];

			for(int j=0;j < numy;j++)
			{
				this->bs[a][i][j] = new bstruct [numz];
				this->bstex[a][i][j] = new btexstruct[numz];


				for(int k=0;k <numz;k++)
				{
					this->bs[a][i][j][k].dataIndex = NULL;
					this->bs[a][i][j][k].queIndex = NULL;

					this->bstex[a][i][j][k].texdataIndex = NULL;

					Block::nRequestBlock[a][(numy*i+j)*numz+k] = 0;
					Block::bRequestBlock[a][(numy*i+j)*numz+k] = 0;
					Block::rRequestBlock[a][(numy*i+j)*numz+k] = 0;
				}
			}
		}
	}
	//メモリプール確保
	
	int size=MAXBNUM*(BLX+2)*(BLY+2)*(BLZ+2);
	//size=(int)(DATAPOOLALPHA*MAXBNUM*(BLX+2)*(BLY+2)*(BLZ+2));
	//MAXBNUM=DATAPOOLALPHA*MAXBNUM;//値更新
	if(size>=0x7fffffff){//0x7fffffff(2147483647バイトメモリ以上の配列はnew出来ない)
		//size=size/(4096*100);
		printf("メモリ0x7fffffff以上。%dメガバイト注意",MAXBNUM);
	}
	try{printf("datapool確保%dメガバイト\n",MAXBNUM);
	//this->datapool=(float *)calloc(MAXBNUM,sizeof(float));
	this->datapool = new float[MAXBNUM*(BLX+2)*(BLY+2)*(BLZ+2)];//ここでエラー
	}catch(bad_alloc){
		cerr<<"bad_alloc new失敗"<<MAXBNUM*(BLX+2)*(BLY+2)*(BLZ+2)*4/1024/1024<<"Mbytes"<<endl;
		abort();
	}
	catch(...){cerr<<"そのほか謎のエラー"<<endl;
	abort();
	}

	//printf("maxbnum%i\n",MAXBNUM);
	this->texName = new GLuint[g_MaxTexNum];
	
	
	//プールフラグ定義and初期化(false:ブロック入っていない,true：入っている)
	this->memblock = new MainMemManage[MAXBNUM];
	this->mTimeStart = new LARGE_INTEGER[MAXBNUM];
	this->mTimeEnd =new LARGE_INTEGER[MAXBNUM];
	this->temp_count = 1;
	for(int i=0;i<MAXBNUM;i++){
		this->memblock[i].block = new Block;
		this->memblock[i].block->level = -1;
		this->memblock[i].FromLoadTime = 1;
		this->memblock[i].data = NULL;
		this->memblock[i].rockflag = false;
		this->memblock[i].needflag = 0;
		this->memblock[i].loadstartflag = false;
		this->memblock[i].ol = new OVERLAPPED;
		this->memblock[i].Fl = NULL;
	}

	//テクスチャメモリフラグ定義and初期化
	this->texblock = new TexMemManage[g_MaxTexNum];
	for(int i= 0;i<g_MaxTexNum;i++)
	{
		this->texblock[i].block = new Block;
		this->texblock[i].block->level = -1;
		this->texblock[i].needflag = 0;
		this->texblock[i].texdata = NULL;
	}


	for(int i=0;i < NUMLEVEL;i++)
	{	
		Block::CLR[i][0] = CLR[i][0];
		Block::CLR[i][1] = CLR[i][1];
		Block::CLR[i][2] = CLR[i][2];
	}

	texLoadflag = false;

	this->reqque = new RequestQueue[QSIZE];//これからメインメモリにロードしたいものリスト
	for(int i=0;i<QSIZE;i++)
	{//初期化
		this->reqque[i].needflag = 0;
		this->reqque[i].block = new Block;
		this->reqque[i].block->level = -1;
	}
	

	}


void File::deleteMemory(btexstruct* bstex)
{
	bstex->texdataIndex = NULL;
}
void File::deleteMemory(bstruct* bs)
{
	bs->dataIndex = NULL;
}
LARGE_INTEGER File::getLaddress(long long address)
{
	address = address * sizeof(float);

	int lower; 
	int upper;
	memcpy(&lower,(unsigned char*)&address,   sizeof(int));
	memcpy(&upper,(unsigned char*)&address  + sizeof(int) ,   sizeof(int));

	LARGE_INTEGER laddr;
	laddr.LowPart = lower;
	laddr.HighPart = upper;

	return laddr;
}
void File::loadFile(string file,float* data,int header,int fx, int fy, int fz,MainMemManage* mmm)
{
	HANDLE handle;
	
	DWORD dwWriteSize;
	//wchar_t *wc = new wchar_t[file.size()+1];
	//mbstowcs(wc,file.data(),file.size()+1);

	handle = CreateFile(file.c_str(),GENERIC_READ,NULL,NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,NULL);
	
	if(handle == INVALID_HANDLE_VALUE)
	{
		LPVOID lpMsgBuf;
		//ここにチェックしたい処理を書く

		FormatMessage(				//エラー表示文字列作成
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			(LPTSTR) &lpMsgBuf, 0, NULL);

		MessageBox(NULL, (LPCSTR)lpMsgBuf, NULL, MB_OK);	//メッセージ表示

		LocalFree(lpMsgBuf);

		fprintf(stderr,"open failed");
	}
	LARGE_INTEGER laddr;
	//初期化
	laddr = getLaddress(header);
		
//SetFilePointerEx(handle,laddr,NULL,FILE_BEGIN);//ヘッダの読み飛ばし。なぜかあってもなくても変わらない。・。。！！？

	mmm->Fl = handle;
	ZeroMemory(mmm->ol,sizeof(*mmm->ol));
	mmm->ol->Offset = laddr.LowPart;
	mmm->ol->OffsetHigh = 0;
	mmm->ol->hEvent = NULL;
	
	BOOL bRet = ReadFile(handle,data,sizeof(float)*fx*fy*fz,&dwWriteSize, mmm->ol);
	

	
	DWORD dwGle = GetLastError();
    if(!bRet && !(!bRet && ERROR_IO_PENDING == dwGle))
	{
		cout<<"Error file load"<<endl;
		abort();

	}

	//delete []wc;
	
}
void File::loadIndexFile(string file,unsigned int* data,int header,int fx, int fy, int fz)//indexファイル読み込み用
{
	HANDLE Fl;
	DWORD dwWriteSize;
	//wchar_t *wc = new wchar_t[file.size()+1];
	//mbstowcs(wc,file.data(),file.size()+1);

	Fl = CreateFile(file.c_str(),GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(Fl == INVALID_HANDLE_VALUE)
	{
		LPVOID lpMsgBuf;
		//ここにチェックしたい処理を書く

		FormatMessage(				//エラー表示文字列作成
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			(LPTSTR) &lpMsgBuf, 0, NULL);
		cerr<< (const char*)lpMsgBuf<<endl;
		abort();
		MessageBox(NULL, (LPCSTR)lpMsgBuf, NULL, MB_OK);	//メッセージ表示

		LocalFree(lpMsgBuf);

		fprintf(stderr,"open failed");
	}
	LARGE_INTEGER laddr;
	//初期化
	laddr = getLaddress(header);
	SetFilePointerEx(Fl,laddr,NULL,FILE_BEGIN);
	ReadFile(Fl,data,sizeof(int)*fx*fy*fz, &dwWriteSize, NULL);
	//delete []wc;
	CloseHandle(Fl);
}


GLuint* File::getTexaddress(int index){
	return &this->texName[index];
}
int File::getTexCompressInfo(Block bl){
	bl=bl.returnParent();
	int id=this->texCompressInfo[bl.level][(bl.x*bl.bnumy+bl.y)*bl.bnumz+bl.z];
	printf("id=%d,",id);
	return id;
}


void File::loadMainToTex(Block bl,Cg* Cg,CGparameter decal)
{

	LARGE_INTEGER temp1, temp2;
	LARGE_INTEGER freq;
	
	this->RockBlock(bl);
	
	int minIndex;
	Block minbl;
	ULONGLONG minneedflag = 0xffffffffffffffff;
	for(int i = 0;i <g_MaxTexNum;i++)
	{		
		ULONGLONG needvalue = this->texblock[i].needflag;
		if(needvalue == 0)
		{
			minIndex = i;
			break;
		}
		else if(needvalue <= minneedflag)
		{
			minneedflag = needvalue;		
			minIndex = i;
		}
	}
	if(texblock[minIndex].block->level != -1)
	{
		//書き込まれるブロック確定	
		memcpy(&minbl,texblock[minIndex].block,sizeof(Block));
		//書き込まれる側のブロック削除
		this->deleteMemory(&this->bstex[minbl.level][minbl.x][minbl.y][minbl.z]);	
	}

	//ブロック登録
	this->texblock[minIndex].texdata = getTexaddress(minIndex);
	*this->texblock[minIndex].block = bl;
	this->texblock[minIndex].needflag = this->bs[bl.level][bl.x][bl.y][bl.z].dataIndex->needflag;

	LARGE_INTEGER temp2_1, temp2_2;
	LARGE_INTEGER freq2;
	static float temp_sum2;
	static int temp_count = 1;
	static int    count2= 1;

	//時間測定用．ないほうが速いかも！！
	if(count2 >= SAMPLEINTERVAL)
	{
		glFinish();
		QueryPerformanceCounter(&temp2_1);
		QueryPerformanceFrequency(&freq2);
	}


	glBindTexture(GL_TEXTURE_3D, *this->texblock[minIndex].texdata);

	

	MainMemManage* mb = this->bs[bl.level][bl.x][bl.y][bl.z].dataIndex;

glTexSubImage3D(GL_TEXTURE_3D,0,0,0,0,(BLX+2),(BLY+2),(BLZ+2),GL_ALPHA,GL_FLOAT,mb->data);//これがないとボリュームがなくなる。

	glBindTexture(GL_TEXTURE_2D, 0);//これはなくても支障がない。	



	//ブロック登録
	this->bstex[bl.level][bl.x][bl.y][bl.z].texdataIndex = &this->texblock[minIndex];	



	if(count2 >= SAMPLEINTERVAL)
	{
		//時間測定用．ないほうが速いかも！！
		glFinish();
		QueryPerformanceCounter(&temp2_2);
		temp_sum2 += (float)(temp2_2.QuadPart-temp2_1.QuadPart)*1000/(float)freq2.QuadPart;

		this->texLoadTime =  temp_sum2/temp_count;
		//CPUからGPUへのテクスチャ転送時間．
		count2 = 0;
		this->texLoadflag = true;

		temp_count++;
	}
	count2++;

	this->UnRockBlock(bl);
	
}
void File::initTexLoad(){
	//初めにテクスチャメモリにいっぱいロードするぞ
	int counter=g_MaxTexNum-1;
	for(int level=4;level>=0;level--){
		int numx=(int)pow(2.0,(double)(NUMLEVEL-level));
		int numy=(int)pow(2.0,(double)(NUMLEVEL-level));
		int numz=(int)pow(2.0,(double)(NUMLEVEL-level-1));
		//x:9,y=18,z=1でダメになる。たぶん、GPUメモリのせい。
		for(int x=0;x<numx;x++){
			for(int y=0;y<numy;y++){
				for(int z=0;z<numz;z++){
					Block block(x,y,z,level,numx,numy,numz);
					int id=getIndexblock(block);
					if(counter>=0){
						if(id!=0){
							RockBlock(block);
								//ブロック登録
								texblock[counter].texdata = getTexaddress(counter);//texName[counter]
								*texblock[counter].block = block;
								texblock[counter].needflag = 0;//this->bs[block.level][block.x][block.y][block.z].dataIndex->needflag;
								glBindTexture(GL_TEXTURE_3D, *this->texblock[counter].texdata);
								MainMemManage* mb =this->bs[block.level][block.x][block.y][block.z].dataIndex; 
								glTexSubImage3D(GL_TEXTURE_3D,0,0,0,0,(BLX+2),(BLY+2),(BLZ+2),GL_ALPHA,GL_FLOAT,mb->data);
								//	block.Info("initialTexLoad",counter);
								//ブロック登録
								bstex[block.level][block.x][block.y][block.z].texdataIndex = &this->texblock[counter];	
							UnRockBlock(block);
							counter--;
						}
					}
				}
			}
		}
	}//end level
}
void File::initMainLoad(){
	//初期状態としてめいっぱい、低解像度順にロードしておく
	int counter=0;
	for(int level=4;level>=0;level--){
	int numx=(int)pow(2.0,(double)(NUMLEVEL-level));
	int numy=(int)pow(2.0,(double)(NUMLEVEL-level));
	int numz=(int)pow(2.0,(double)(NUMLEVEL-level-1));
	
		for(int x=0;x<numx;x++){
			for(int y=0;y<numy;y++){
				for(int z=0;z<numz;z++){
					Block block(x,y,z,level,numx,numy,numz);
					int id=this->getIndexblock(block);
					if(counter<MAXBNUM){
					if(id!=0){
					//ブロック登録
				memblock[counter].data = &datapool[counter*(BLX+2)*(BLY+2)*(BLZ+2)];//minIndex=0～４まである。
				*this->memblock[counter].block = block;
				memblock[counter].FromLoadTime = 1;	
				memblock[counter].needflag =reqque[counter].needflag;
					stringstream filename;
					filename<<m_Path<<m_DataName<<"_"<<BLX<<"_"<<BLY<<"_"<<BLZ<<"/level"<<level<<"/VertebralBody-"<<level<<"_"<<x<<"_"<<y<<"_"<<z<<".dff";
					//printf("%s\n",filename.str().c_str());
					loadFile(filename.str(),this->memblock[counter].data,8,BLX+2,BLY+2,BLZ+2,&memblock[counter]);
					//this->CheckLoadComplete(counter);
					
					memblock[counter].loadstartflag = true;
				//ロードし終わったら、キューから消去
				//this->reqque[counter].block->level = -1;//ここでエラー
				//this->bs[block.level][block.x][block.y][block.z].queIndex = NULL;
				counter++;
					}
					}
		}

		}

		}
	}//end level
	this->CheckLoadComplete();
}
void File::setTexBlock(Cg* Cg,CGparameter decal,Block block)//ブロックごとのテクスチャ名をシェーダに渡したりする。
{
	TexMemManage* tb = this->bstex[block.level][block.x][block.y][block.z].texdataIndex;
	Cg->SetTextureParameter(Cg->vdecalParam,*tb->texdata);//texdataにはテクスチャ名（GLuint）が入ってる。1～4の値がえんえんと入る。
	
}


void File::loadHDToMain()
{//printf("bnumx[%d]\n",reqque[3].block->bnumx);

	ULONGLONG maxneedflag = 0;
	int maxIndex = -1;
	Block maxbl;
	//書き込むブロック選択
	for(int i=0;i<QSIZE;i++)//QSIZEは1219ぐらい。
	{
		ULONGLONG needvalue;
		needvalue = this->reqque[i].needflag;
		if(needvalue >= maxneedflag && this->reqque[i].block->level != -1)
		{
			maxneedflag = needvalue;
			maxIndex = i;//printf("%d\n",i);
		}//一番必要とされているブロックのインデックスを割り出す
	}




	if(maxIndex != -1)
	{
		maxbl = *this->reqque[maxIndex].block;//この段階でmaxblに具体的な値が入るっぽい。//printf("[%d][%d][%d]\n",maxbl.x,maxbl.y,maxbl.z);
		//maxbl＝今一番必要とされているブロック
		//maxbsがメインメモリに乗っていなければ
		if(this->bs[maxbl.level][maxbl.x][maxbl.y][maxbl.z].dataIndex == NULL)//で、その一番必要とされてるブロックがまだメインブロックにロードされてないならこれからロードする
		{
			int minIndex = -1;
			Block minbl;
			ULONGLONG minneedflag = 0xffffffffffffffff;
			for(int i = 0;i <MAXBNUM;i++)//memblock[i]のneedflagの中で最小のものを探す
			{//もし上書きするなら、最も要らないブロックに上書きしようってことか。
				ULONGLONG needvalue = this->memblock[i].needflag;

				if(!this->memblock[i].rockflag)//上書き禁止ではない
				{
					if(needvalue == 0)
					{
						minIndex = i;
						break;
					}
					else if(needvalue <= minneedflag)
					{
						minneedflag = needvalue;		
						minIndex = i;
					}
				}//if(!this->memblock[i].rockflag)
			}//for(int i = 0;i <MAXBNUM;i++)

			//書き込み先が確定できたら読込み
			if(minIndex != -1)
			{
				//書き込み先にブロックが登録されていたら削除（これか上書きするからってことよね）
				if(memblock[minIndex].block->level != -1)
				{
					//書き込まれるブロック確定	
					memcpy(&minbl,memblock[minIndex].block,sizeof(Block));//minblにmemblock[minIndex].blockをコピーする
					//書き込まれる側のブロック削除
					this->deleteMemory(&this->bs[minbl.level][minbl.x][minbl.y][minbl.z]);	
				}


				//ブロック登録
				this->memblock[minIndex].data = &this->datapool[minIndex*(BLX+2)*(BLY+2)*(BLZ+2)];//minIndex=0～４まである。
				*this->memblock[minIndex].block = maxbl;
				this->memblock[minIndex].FromLoadTime = 1;	
				this->memblock[minIndex].needflag = 0x0f00000000000000;


				//printf("[%d][%d][%d]\n",maxbl.x,maxbl.y,maxbl.z);
				//この時点ではmaxbl.xとかはそれぞれ詳細度が変わったら順を追って読み込んでいる。
				//ハードディスクからブロック読み込み
				stringstream volumefile;
				volumefile<<m_Path<<m_DataName<<"_"<<BLX<<"_"<<BLY<<"_"<<BLZ<<"/"<<"level"<<maxbl.level<<"/"<<"VertebralBody"<<"-"<<maxbl.level<<"_"<<maxbl.x<<"_"<<maxbl.y<<"_"<<maxbl.z<<".dff";
				
				//時間測定用．ないほうが速いかも！！
				QueryPerformanceCounter(&this->mTimeStart[minIndex]);
				QueryPerformanceFrequency(&this->freq);
				
				
				loadFile(volumefile.str(),this->memblock[minIndex].data,8,BLX+2,BLY+2,BLZ+2,&this->memblock[minIndex]);
				
				this->memblock[minIndex].loadstartflag = true;
				//ロードし終わったら、キューから消去
				this->reqque[maxIndex].block->level = -1;
				this->bs[maxbl.level][maxbl.x][maxbl.y][maxbl.z].queIndex = NULL;
			}//if(minIndex !=-1)
		}//if(this->bs[maxbl.level][maxbl.x][maxbl.y][maxbl.z].dataIndex == NULL)
	}//else{printf("nothing has to be loaded\n");}//if(maxIndex != -1)新しくロードすべきものが何もないとき
}//void File::loadHDToMain()



void File::countMainLRU(void)
{
	LARGE_INTEGER temp;
	LARGE_INTEGER freq;
	static float time,timebase = 0.0f;//printf("timebase=%.4f\n",timebase);
	QueryPerformanceCounter(&temp);
	QueryPerformanceFrequency(&freq);
	time = (float)(temp.QuadPart)*1000/(float)freq.QuadPart;
	//timeは普通に、プログラム起動からの経過時間？なんでそんなもの使うの
	//static変数だからbasetimeの値が保持されるっぽい
	if (time - timebase > this->loadTime) { //平均ロード時間で1ビット繰り下げ　this->loadTimeは、HD toCPUの平均時間　ふつうにwhileの１ループ終わったら１ビット繰り下げと考えてよいのかな。。。？
		for(int i= 0; i < MAXBNUM;i ++)
			this->memblock[i].needflag = this->memblock[i].needflag >> 5;

		for(int i=0;  i < QSIZE;i++)
		{
			this->reqque[i].needflag = this->reqque[i].needflag >> 5;
			//フラグの値がゼロになったらキューから追い出す
			if(this->reqque[i].needflag == 0)//reqqueはまだメインメモリにロードしてない必要なブロックリスト
			{
				if(this->reqque[i].block->level != -1)
				{//前のブロックの情報が残ってるようだったら消すってことかな
					Block bl = *this->reqque[i].block;
					this->bs[bl.level][bl.x][bl.y][bl.z].queIndex = NULL;
				}
				
				this->reqque[i].block->level = -1;
			}
		}//if (time - timebase > this->loadTime) 
		
		timebase = time;		//while loop１個ごとに値が更新される
	}
}


void File::countTexLRU(void)
{
	LARGE_INTEGER temp;
	LARGE_INTEGER freq;
	static float time,timebase = 0.0f;
	QueryPerformanceCounter(&temp);
	QueryPerformanceFrequency(&freq);
	time = (float)(temp.QuadPart)*1000/(float)freq.QuadPart;
	if ((double)time - (double)timebase > this->texLoadTime) { //平均ロード時間で1ビット繰り下げ
		for(int i= 0; i < g_MaxTexNum;i ++)
			this->texblock[i].needflag = this->texblock[i].needflag >> 5;

		timebase = time;
	}
}



int File::getIndexblock(Block bl)
{
	int id;
	id = this->indexblock[bl.level][(bl.x*bl.bnumy+bl.y)*bl.bnumz+bl.z];//idは0か1の値 この３次元配列変！？ でもこっちが正しいみたい。

	

	return id;
}

ULONGLONG File::getReqbit(double* modelmatrix,double* projmatrix,Block bl,frustum<float> fr,frustum<float>::FrustumName fname)
{
	
	float blockPosition[4];
	//ブロックの中心点計算
	float x = (2.0f*bl.x-bl.bnumx+1.0f)*Block::brX*Block::iniX/bl.bnumx;
	float y = (2.0f*bl.y-bl.bnumy+1.0f)*Block::brY*Block::iniY/bl.bnumy;
	float z = (2.0f*bl.z-bl.bnumz+1.0f)*Block::brZ*Block::iniZ/bl.bnumz;

	blockPosition[0] = x;
	blockPosition[1] = y;
	blockPosition[2] = z;
	blockPosition[3] = 1.0f;
	
	Block::mMtx(modelmatrix, blockPosition, blockPosition);
	Block::mMtx(projmatrix,  blockPosition, blockPosition);
	//なんでprojMatrixをかける？？
	float projx = blockPosition[0]/blockPosition[3];
	float projy = blockPosition[1]/blockPosition[3];
	float projz = blockPosition[2]/blockPosition[3];

	float blockdis =  sqrt(projx*projx+projy*projy);
	
	
	//初期化
	ULONGLONG reqbit = 0;


	//高，低フラスタムフラグ(バックアップ優先)
	if(fname == frustum<float>::back)
		reqbit = reqbit|0x8000000000000000;//0が15+8
	

	//距離フラグ
	if(projz > fr.dis2)
	{//領域3 01
		reqbit = reqbit|0x2000000000000000;//
	}
	else if(projz > fr.dis1)
	{//領域２ 10
		reqbit = reqbit|0x4000000000000000;
	}
	else
	{//領域1 11（一番必要）
		reqbit = reqbit|0x2000000000000000;
		reqbit = reqbit|0x4000000000000000;
	}

	//画面中心からの距離フラグ	スクリーン中心分割
	if(blockdis > fr.radious2)
	{//領域3　01
		reqbit = reqbit|0x0800000000000000;
	}
	else if(blockdis > fr.radious1)
	{//領域2　10
		reqbit = reqbit|0x100000000000000;
	}
	else
	{//領域１ 11(最も必要)
		reqbit = reqbit|0x0800000000000000;
		reqbit = reqbit|0x100000000000000;
	}



	return reqbit;
}


void File::mainBlockRequest(double* modelmatrix,double* projmatrix,Block bl,frustum<float> fr,frustum<float>::FrustumName fname)
{
	ULONGLONG reqbit = this->getReqbit(modelmatrix,projmatrix,bl,fr,fname);
	


	MainMemManage* mb = this->bs[bl.level][bl.x][bl.y][bl.z].dataIndex;
	RequestQueue*  qb = this->bs[bl.level][bl.x][bl.y][bl.z].queIndex;
	if(mb != NULL) //メモリにあるとき
	{
		mb->needflag = mb->needflag|reqbit;
	}
	else if(qb == NULL) //メモリになくてキューにも入っていないときキューに入れる
	{
		//最も少ないカウントのブロックに転送
		int minIndex;
		Block minbl;
		ULONGLONG minneedflag = 0xffffffffffffffff;
		
		bool fullflag = true;
		for(int i = 0;i <QSIZE;i++)
		{		
			if(this->reqque[i].block->level == -1)
			{
				minIndex = i;
				fullflag = false;
				break;
			}
		}
	
		if(fullflag)
		{
			for(int i = 0;i < QSIZE;i++)
			{
				ULONGLONG needvalue;
				needvalue = this->reqque[i].needflag;
				if(this->reqque[i].block->level != -1 && needvalue<= minneedflag)
				{
					minneedflag = needvalue;
					minIndex = i;	
				}		
			}
			//書き込まれるブロック確定
			memcpy(&minbl,this->reqque[minIndex].block,sizeof(Block));
			this->bs[minbl.level][minbl.x][minbl.y][minbl.z].queIndex = NULL;
		}	


		//ブロック登録 ここ大事
		qb = &this->reqque[minIndex];
		*this->reqque[minIndex].block = bl;
		this->reqque[minIndex].needflag = 0;
		this->reqque[minIndex].needflag = this->reqque[minIndex].needflag|reqbit;//新しい情報を入れる
		this->bs[bl.level][bl.x][bl.y][bl.z].queIndex = &this->reqque[minIndex];	//あとでloadHDtoMainで使う
	}
	else //メモリにはないがキューにはあるとき
	{	
		qb->needflag = qb->needflag|reqbit;
	}

}
	
void File::texBlockRequest(double* modelmatrix,double* projmatrix,Block bl,frustum<float> fr,frustum<float>::FrustumName fname)
{
	
	ULONGLONG reqbit = this->getReqbit(modelmatrix,projmatrix,bl,fr,fname);
	
	TexMemManage* tb = this->bstex[bl.level][bl.x][bl.y][bl.z].texdataIndex;
	tb->needflag = tb->needflag|reqbit;
	

}
	


bool File::blockExist(Block bl)
{
	if(this->bs[bl.level][bl.x][bl.y][bl.z].dataIndex == NULL)
		return false;
	else
		return true;
}

Block File::lowBlockExist(Block bl)
{
	do{
		bl = bl.getLowblock();
	}while(this->bs[bl.level][bl.x][bl.y][bl.z].dataIndex == NULL && bl.level != NUMLEVEL - 1);

	if(this->bs[bl.level][bl.x][bl.y][bl.z].dataIndex != NULL)
		return bl;
	else
	{
		Block noblock;
		noblock.level = -1;
		return noblock;
	}
}

Block File::lowBlockExistFromRoot(Block bl)
{
	do{
		bl = bl.getMultiLowblock((NUMLEVEL - 1) - bl.level);
	}while(this->bs[bl.level][bl.x][bl.y][bl.z].dataIndex == NULL && bl.level != 0);

	if(this->bs[bl.level][bl.x][bl.y][bl.z].dataIndex != NULL)
		return bl;
	else
	{
		Block noblock;
		noblock.level = -1;
		return noblock;
	}
}




bool File::highBlockExist(Block bl)
{
	bool flag = false;

	if(bl.level != 0)
	{
		if(this->blockExist(Block(2*bl.x  ,2*bl.y  ,2*bl.z       ,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2)) == true
			||	this->blockExist(Block(2*bl.x+1,2*bl.y  ,2*bl.z  ,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2)) == true
			||  this->blockExist(Block(2*bl.x  ,2*bl.y+1,2*bl.z  ,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2)) == true
			||  this->blockExist(Block(2*bl.x  ,2*bl.y  ,2*bl.z+1,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2)) == true
			||  this->blockExist(Block(2*bl.x+1,2*bl.y+1,2*bl.z  ,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2)) == true
			||  this->blockExist(Block(2*bl.x  ,2*bl.y+1,2*bl.z+1,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2)) == true
			||  this->blockExist(Block(2*bl.x+1,2*bl.y  ,2*bl.z+1,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2)) == true
			||  this->blockExist(Block(2*bl.x+1,2*bl.y+1,2*bl.z+1,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2)) == true)
			flag = true;
	}
	return flag;
}



bool File::texBlockExist(Block bl)
{
	if(this->bstex[bl.level][bl.x][bl.y][bl.z].texdataIndex != NULL)
		return true;
	else
		return false;
}


Block File::texLowBlockExist(Block bl)
{
	do{
		bl = bl.getLowblock();
	}while(this->bstex[bl.level][bl.x][bl.y][bl.z].texdataIndex == NULL && bl.level != NUMLEVEL - 1);

	if(this->bstex[bl.level][bl.x][bl.y][bl.z].texdataIndex != NULL)
		return bl;
	else
	{
		Block noblock;
		noblock.level = -1;
		return noblock;
	}
}



GLuint File::getTexName(int index)
{
	return this->texName[index];
}


void File::RockBlock(Block bl){
	MainMemManage* mb = this->bs[bl.level][bl.x][bl.y][bl.z].dataIndex;
	mb->rockflag = true;
}
void File::UnRockBlock(Block bl){
	MainMemManage* mb = this->bs[bl.level][bl.x][bl.y][bl.z].dataIndex;
	mb->rockflag = false;
}


float File::getTexLoadTime()
{
	return this->texLoadTime;
}


bool  File::getTexLoadflag()
{
	return this->texLoadflag;
}


void File::CheckLoadComplete()
{
	for(int i = 0 ; i < MAXBNUM ; i++)
	{
		Block bl = *this->memblock[i].block;
		if(this->memblock[i].loadstartflag)
		{
			DWORD cbNumberOfBytesTransferred = 0;//読み込まれたバイト数を受け取る変数。
			if(GetOverlappedResult(this->memblock[i].Fl,this->memblock[i].ol,&cbNumberOfBytesTransferred,TRUE))
			{
				//ファイル転送完了したらフラグ立てる
				this->memblock[i].loadstartflag = false;//loadstartflag=falseだとファイル転送完了って意味らしい。
				this->bs[bl.level][bl.x][bl.y][bl.z].dataIndex = &this->memblock[i];
				//dataIndexメインメモリ上のどこに存在するかのアドレス
				CloseHandle(this->memblock[i].Fl);	

				QueryPerformanceCounter(&this->mTimeEnd[i]);
				this->temp_sum += (float)(this->mTimeEnd[i].QuadPart-this->mTimeStart[i].QuadPart)*1000.0f/(float)this->freq.QuadPart;
				this->loadTime =  this->temp_sum/this->temp_count;
				//ハードディスクからブロック読み込み
				this->temp_count++;
			}	
		}
	}
}
void File::CheckLoadComplete(int i)
{
	/*for(int i = 0 ; i < MAXBNUM ; i++)
	{*/
		Block bl = *this->memblock[i].block;
		if(this->memblock[i].loadstartflag)
		{
			DWORD cbNumberOfBytesTransferred = 0;//読み込まれたバイト数を受け取る変数。
			if(GetOverlappedResult(this->memblock[i].Fl,this->memblock[i].ol,&cbNumberOfBytesTransferred,TRUE))
			{
				//ファイル転送完了したらフラグ立てる
				this->memblock[i].loadstartflag = false;//loadstartflag=falseだとファイル転送完了って意味らしい。
				this->bs[bl.level][bl.x][bl.y][bl.z].dataIndex = &this->memblock[i];
				//dataIndexメインメモリ上のどこに存在するかのアドレス
				CloseHandle(this->memblock[i].Fl);	

				QueryPerformanceCounter(&this->mTimeEnd[i]);
				this->temp_sum += (float)(this->mTimeEnd[i].QuadPart-this->mTimeStart[i].QuadPart)*1000.0f/(float)this->freq.QuadPart;
				this->loadTime =  this->temp_sum/this->temp_count;
				//ハードディスクからブロック読み込み
				this->temp_count++;
			}	
		}
	//}
}
float File::getMainLoadTime()
{
	return this->loadTime;
}


void File::countThreadTime()
{/*ハードディスク読込み時間測定用らしい*/
	QueryPerformanceCounter(&this->fTime);
	QueryPerformanceFrequency(&freq);
	this->fileThreadTime = (float)(this->fTime.QuadPart-this->fTimeprev.QuadPart)*1000.0f/(float)freq.QuadPart;
	this->fTimeprev = this->fTime;
}

float File::getThreadTime()//論文のp35 (file.getThreadTime()=1フレーム描画するのにかかる時間！！？？
{
	return this->fileThreadTime;/*ハードディスク読込み時間測定用らしい*/
}

double File::getMainToTexTime(){
	return this->maintotextime;
}
void File::deleteMemory(){
	delete[] datapool;
}
int File::returnMainMemInfo(int _i){
	return this->memblock[_i].block->level;
}
int File::returnTexMemInfo(int _i){
	return this->texblock[_i].block->level;
}
