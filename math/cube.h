#pragma once
#ifndef MIFFYCUBE
#define MIFFYCUBE
#include "vec3.h"
#include "matrix.h"
namespace miffy{
  static	const int edgeList[8][12] = {
		{ 0,1,5,6,   4,8,11,9,  3,7,2,10 }, // v0 is front
		{ 0,4,3,11,  1,2,6,7,   5,9,8,10 }, // v1 is front
		{ 1,5,0,8,   2,3,7,4,   6,10,9,11}, // v2 is front
		{ 7,11,10,8, 2,6,1,9,   3,0,4,5  }, // v3 is front
		{ 8,5,9,1,   11,10,7,6, 4,3,0,2  }, // v4 is front
		{ 9,6,10,2,  8,11,4,7,  5,0,1,3  }, // v5 is front
		{ 9,8,5,4,   6,1,2,0,   10,7,11,3}, // v6 is front
		{ 10,9,6,5,  7,2,3,1,   11,4,8,0 }  // v7 is front
	}; 
	struct Edge {
	public:
		Edge(int i, int j) {nV1=i; nV2=j;}
		Edge() {nV1=0; nV2=0;}
		int nV1, nV2;
	};
	
	
	template <typename T>
	struct cube{
		vec3<T> corner[8];
		vec3<T> center;
		Edge     edges[12];//スライス面描画に使用
		
		cube(){
			set(vec3<T>(0,0,0),1.0);
			//スライス面を書くのに使用
			edges[0]  = Edge(0,1);
			edges[1]  = Edge(1,2);
			edges[2]  = Edge(2,3);
			edges[3]  = Edge(3,0);
			edges[4]  = Edge(0,4);
			edges[5]  = Edge(1,5);
			edges[6]  = Edge(2,6);
			edges[7]  = Edge(3,7);
			edges[8]  = Edge(4,5);
			edges[9]  = Edge(5,6);
			edges[10] = Edge(6,7);
			edges[11] = Edge(7,4);
		}
		~cube(){}
		virtual void set(vec3<T> &_center,T _size){//中心点とサイズからキューブを作る。
			center=_center;
			corner[0]=_center+vec3<T>(-_size,-_size,-_size);
			corner[1]=_center+vec3<T>( _size,-_size,-_size);
			corner[2]=_center+vec3<T>( _size, _size,-_size);
			corner[3]=_center+vec3<T>(-_size, _size,-_size);

			corner[4]=_center+vec3<T>(-_size,-_size, _size);
			corner[5]=_center+vec3<T>( _size,-_size, _size);
			corner[6]=_center+vec3<T>( _size, _size, _size);
			corner[7]=_center+vec3<T>(-_size, _size, _size);
		}
		void setFromCorner(vec3<T> &_corner,T _size){
			corner[0]=_corner;
			corner[1]=corner[0]+vec3<T>( _size,0,0);
			corner[2]=corner[0]+vec3<T>( _size, _size,0);
			corner[3]=corner[0]+vec3<T>(0, _size,0);

			corner[4]=corner[0]+vec3<T>(0,0, _size);
			corner[5]=corner[0]+vec3<T>( _size,0, _size);
			corner[6]=corner[0]+vec3<T>( _size, _size, _size);
			corner[7]=corner[0]+vec3<T>(0, _size, _size);
		}
		//z方向だけ違うサイズにしたい時。
		void set(vec3<T> &_center,T _size,T _sizeZ){//中心点とサイズからキューブを作る。
			center=_center;
			corner[0]=_center+vec3<T>(-_size,-_size,-_sizeZ);
			corner[1]=_center+vec3<T>( _size,-_size,-_sizeZ);
			corner[2]=_center+vec3<T>( _size, _size,-_sizeZ);
			corner[3]=_center+vec3<T>(-_size, _size,-_sizeZ);

			corner[4]=_center+vec3<T>(-_size,-_size, _sizeZ);
			corner[5]=_center+vec3<T>( _size,-_size, _sizeZ);
			corner[6]=_center+vec3<T>( _size, _size, _sizeZ);
			corner[7]=_center+vec3<T>(-_size, _size, _sizeZ);
		}
#ifdef MIFFY_VEC2
		vec2<T> projectedsize(const mat4<T>& _modelmatrix,const mat4<T>& _projmatrix,const vec2<int>& _winsize){
			double dis_from_camera;//カメラから頂点への距離
			double max_distance = -DBL_MAX;//カメラから頂点への距離の最大値
			
			vec4<T> local_pos;//オブジェクトの中心が原点な座標系
			vec4<T> view_pos;//視点が原点な座標系
			vec4<T> clip_pos;//クリップ座標での位置
			vec2<T> normalized_device_pos;
			//キューブの中で投影された中でも一番右上と左下を求めて最大の大きさを知る
			vec2<double> minPixel(DBL_MAX,DBL_MAX),maxPixel(-DBL_MAX,-DBL_MAX);
			for(int i = 0;i  < 8;i++)//
			{
				local_pos.set(corner[i]);
				view_pos=_modelmatrix*local_pos;
				clip_pos=_projmatrix*view_pos;
				//wで割って正規化デバイス座標になる
				//正規化デバイス座標はX座標が -1.0f から1.0f,Y座標が-1.0f から　1.0f ,Z軸が0.0f から 1.0f 
				normalized_device_pos.x =clip_pos.x/ clip_pos.w;
				normalized_device_pos.y =clip_pos.y/ clip_pos.w;
				if(normalized_device_pos<minPixel){
					minPixel=normalized_device_pos;
				}
				if(normalized_device_pos>maxPixel){
					maxPixel=normalized_device_pos;
				}
			
			}
			return vec2<T>(maxPixel-minPixel);//引き算する＝距離・サイズ
		}
#endif
#ifdef  __FREEGLUT_H__
		void DrawWireCube(){
			
			glBegin(GL_LINE_LOOP);
			corner[0].glVertex();corner[1].glVertex();corner[2].glVertex();corner[3].glVertex();
			glEnd();
			glBegin(GL_LINE_LOOP);
			corner[4].glVertex();corner[5].glVertex();corner[6].glVertex();corner[7].glVertex();
			glEnd();
			glBegin(GL_LINES);
			corner[0].glVertex();corner[4].glVertex(); 
			corner[1].glVertex();corner[5].glVertex();
			corner[2].glVertex();corner[6].glVertex();
			corner[3].glVertex();corner[7].glVertex();
			glEnd();
			
		}
		//手前の線は描かない！　挫折中
		void DrawOnlyBackWire(float _camX,float _camY,float _camZ){//
			vec3<float> viewVec(_camX,_camY,_camZ);
 
			float dMaxDist = viewVec.innerProduct(corner[0]);//内積を求める
			float dMinDist = dMaxDist;
			int nFrontIdx = 0;
			for(int i = 1; i < 8; ++i) {
				float dist = viewVec.innerProduct(corner[i]);//各頂点との内積を求める
				if ( dist > dMaxDist) {
					dMaxDist = dist;
				}
				if ( dist < dMinDist) {
					dMinDist = dist;
					nFrontIdx = i;//マウスで箱を動かすとここの値が変わる。初期値では0
				}
 
			}//
			//nFrontIdxが手前かな
			//nFrontIdxに属さない頂点だけを描く。
			//あるいは、nFrontIdxだけ完全透明にするか？
			static unsigned short loop_index[]={0,1,2,3, 4,5,6,7};
			static unsigned short lines_index[]={0,4,1,5,2,6,3,7};
			//glBegin(GL_LINE_LOOP);
			//glEnableClientState(GL_VERTEX_ARRAY);
			//glVertexPointer(3,GL_FLOAT,sizeof(vec3<float>),&corner[0].x);
			//glEnableClientState(GL_COLOR_ARRAY);
			static float col[8*4];//ある特定の頂点を完全透明にするため
			for(int i=0;i<8*4;i++){
				col[i]=1.0;
			}
			col[nFrontIdx*4+3]=0.0f;
			//glColorPointer(4,GL_FLOAT,0,col);
			/*glDrawElements(GL_LINE_LOOP,4,GL_UNSIGNED_SHORT,loop_index);
			glDrawElements(GL_LINE_LOOP,4,GL_UNSIGNED_SHORT,&loop_index[4]);
			glDrawElements(GL_LINES,8,GL_UNSIGNED_SHORT,lines_index);
			glDisableClientState(GL_COLOR_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);*/
			glBegin(GL_LINE_LOOP);
			for(int i=0;i<4;i++){
				glColor4fv(&col[loop_index[i]*4]);
				corner[loop_index[i]].glVertex();
			}
			glEnd();
			glBegin(GL_LINE_LOOP);
			for(int i=0;i<4;i++){
				glColor4fv(&col[loop_index[(4+i)]*4]);
				corner[loop_index[4+i]].glVertex();
			}
			glEnd();
			glBegin(GL_LINES);
			for(int i=0;i<8;i++){
				glColor4fv(&col[lines_index[(i)]*4]);
				corner[lines_index[i]].glVertex();
			}
			glEnd();
			//corner[0].glVertex();corner[1].glVertex();corner[2].glVertex();corner[3].glVertex();
			//glEnd();
			//glBegin(GL_LINE_LOOP);
			//corner[4].glVertex();corner[5].glVertex();corner[6].glVertex();corner[7].glVertex();
			//glEnd();
			/*glBegin(GL_LINES);
			corner[0].glVertex();corner[4].glVertex(); 
			corner[1].glVertex();corner[5].glVertex();
			corner[2].glVertex();corner[6].glVertex();
			corner[3].glVertex();corner[7].glVertex();
			glEnd();*/
		}
		void DrawQuads(){
			glBegin(GL_QUADS);
			corner[0].glVertex();corner[3].glVertex();corner[2].glVertex();corner[1].glVertex();
			corner[0].glVertex();corner[1].glVertex();corner[5].glVertex();corner[4].glVertex();
			corner[0].glVertex();corner[4].glVertex();corner[7].glVertex();corner[3].glVertex();
			
			corner[6].glVertex();corner[2].glVertex();corner[3].glVertex();corner[7].glVertex();
			corner[6].glVertex();corner[7].glVertex();corner[4].glVertex();corner[5].glVertex();
			corner[6].glVertex();corner[5].glVertex();corner[1].glVertex();corner[2].glVertex();
			glEnd();
		}
		void DrawQuadsWithNormal(){
			glBegin(GL_QUADS);
			glNormal3f(0.0,0.0,-1.0);
			corner[0].glVertex();corner[3].glVertex();corner[2].glVertex();corner[1].glVertex();
			glNormal3f(0.0,-1.0,0.0);
			corner[0].glVertex();corner[1].glVertex();corner[5].glVertex();corner[4].glVertex();
			glNormal3f(-1.0,0.0,0.0);
			corner[0].glVertex();corner[4].glVertex();corner[7].glVertex();corner[3].glVertex();
			
			glNormal3f(0.0,1.0,0.0);
			corner[6].glVertex();corner[2].glVertex();corner[3].glVertex();corner[7].glVertex();
			glNormal3f(0.0,0.0,1.0);
			corner[6].glVertex();corner[7].glVertex();corner[4].glVertex();corner[5].glVertex();
			glNormal3f(1.0,0.0,0.0);
			corner[6].glVertex();corner[5].glVertex();corner[1].glVertex();corner[2].glVertex();
			glEnd();
		}
		bool CalcClipPlaneVerts(const mat4<T>& _modelview,float _distCam,vec3<T> intersection[6]){
			vec3<float> viewVec(-_modelview.m[2],-_modelview.m[6],-_modelview.m[10]);

			float dMaxDist = viewVec.innerProduct( corner[0]);
			float dMinDist = dMaxDist;
			int nMaxIdx = 0;
			for(int i = 1; i < 8; ++i) {
				float dist = viewVec.innerProduct( corner[i]);
				if ( dist > dMaxDist) {
					dMaxDist = dist;
					nMaxIdx = i;
				}
				if ( dist < dMinDist) {
					dMinDist = dist;
				}

			}
			vec3<float> vecStart[12];
			vec3<float> vecDir[12];
			float lambda[12];
			float denom;

			dMinDist += FLT_EPSILON;
			dMaxDist -= FLT_EPSILON;
			
			float dVertices[12*3];
			float dEdges[12*3];

			for(int i = 0; i < 12; i++) {
				vecStart[i] = corner[edges[edgeList[nMaxIdx][i]].nV1];
				vecDir[i]   = corner[edges[edgeList[nMaxIdx][i]].nV2] - corner[edges[edgeList[nMaxIdx][i]].nV1];

				denom = vecDir[i].innerProduct(viewVec);

				if (1.0 + denom != 1.0) {
					lambda[i]     = (_distCam - vecStart[i].innerProduct(viewVec))/denom;
				} else {
					lambda[i]     = -1.0;
				}

				dVertices[3*i]   = vecStart[i].x;
				dVertices[3*i+1] = vecStart[i].y;
				dVertices[3*i+2] = vecStart[i].z;
				dEdges[3*i]      = vecDir[i].x;
				dEdges[3*i+1]    = vecDir[i].y;
				dEdges[3*i+2]    = vecDir[i].z;
			};

			bool visible=true;
			//lmb:辺のどのあたりに交点があるのかの、割合。

			if      ((lambda[0] >= 0.0) && (lambda[0] < 1.0)) intersection[0] = vecStart[0] + vecDir[0]*lambda[0] ;
			else if ((lambda[1] >= 0.0) && (lambda[1] < 1.0)) intersection[0] = vecStart[1] + vecDir[1]* lambda[1] ;
			else if ((lambda[3] >= 0.0) && (lambda[3] < 1.0)) intersection[0] = vecStart[3] + vecDir[3]* lambda[3] ;
			else {return false;}//このボックスにクリッピング面は交差していません。

			if	    ((lambda[2] >= 0.0) && (lambda[2] < 1.0)) intersection[1] = vecStart[2] + vecDir[2]*lambda[2] ;
			else if ((lambda[0] >= 0.0) && (lambda[0] < 1.0)) intersection[1] = vecStart[0] + vecDir[0]*lambda[0] ;
			else if ((lambda[1] >= 0.0) && (lambda[1] < 1.0)) intersection[1] = vecStart[1] + vecDir[1]*lambda[1] ;
			else intersection[1] = vecStart[3] + vecDir[3] * lambda[3];

			if      ((lambda[4] >= 0.0) && (lambda[4] < 1.0)) intersection[2] = vecStart[4] +vecDir[4]* lambda[4] ;
			else if ((lambda[5] >= 0.0) && (lambda[5] < 1.0)) intersection[2] = vecStart[5] +  vecDir[5]*lambda[5] ;
			else intersection[2] = vecStart[7] + vecDir[7]*lambda[7]  ;

			if	    ((lambda[6] >= 0.0) && (lambda[6] < 1.0)) intersection[3] = vecStart[6] + vecDir[6]*lambda[6]  ;
			else if ((lambda[4] >= 0.0) && (lambda[4] < 1.0)) intersection[3] = vecStart[4] +  vecDir[4]*lambda[4] ;
			else if ((lambda[5] >= 0.0) && (lambda[5] < 1.0)) intersection[3] = vecStart[5] +  vecDir[5]*lambda[5] ;
			else intersection[3] = vecStart[7] + vecDir[7]*lambda[7];

			if	    ((lambda[8] >= 0.0) && (lambda[8] < 1.0)) intersection[4] = vecStart[8] +  vecDir[8]*lambda[8]  ;
			else if ((lambda[9] >= 0.0) && (lambda[9] < 1.0)) intersection[4] = vecStart[9] +  vecDir[9]*lambda[9]  ;
			else intersection[4] = vecStart[11]+ vecDir[11]*lambda[11];

			if	    ((lambda[10]>= 0.0) && (lambda[10]< 1.0)) intersection[5] = vecStart[10]+ vecDir[10]*lambda[10]; 
			else if ((lambda[8] >= 0.0) && (lambda[8] < 1.0)) intersection[5] = vecStart[8] + vecDir[8]*lambda[8]  ;
			else if ((lambda[9] >= 0.0) && (lambda[9] < 1.0)) intersection[5] = vecStart[9] + vecDir[9]*lambda[9]   ;
			else intersection[5] = vecStart[11]+ vecDir[11]*lambda[11];

			return visible;

		}
		//_distCam=視点と平面の距離　0にしたら箱の真中に平面が来るけど。。？？
		bool CalcClipPlaneVerts(double _clipEqn[4],vec3<float> intersection[6]){
			vec3<float> viewVec((float)_clipEqn[0],(float)_clipEqn[1],(float)_clipEqn[2]);

			float dMaxDist = viewVec.innerProduct( corner[0]);
			float dMinDist = dMaxDist;
			int nMaxIdx = 0;
			for(int i = 1; i < 8; ++i) {
				float dist = viewVec.innerProduct( corner[i]);
				if ( dist > dMaxDist) {
					dMaxDist = dist;
					nMaxIdx = i;
				}
				if ( dist < dMinDist) {
					dMinDist = dist;
				}

			}
			vec3<float> vecStart[12];
			vec3<float> vecDir[12];
			float lambda[12];
			float denom;

			dMinDist += FLT_EPSILON;
			dMaxDist -= FLT_EPSILON;
			
			float dVertices[12*3];
			float dEdges[12*3];

			for(int i = 0; i < 12; i++) {
				vecStart[i] = corner[edges[edgeList[nMaxIdx][i]].nV1];
				vecDir[i]   = corner[edges[edgeList[nMaxIdx][i]].nV2] - corner[edges[edgeList[nMaxIdx][i]].nV1];

				denom = vecDir[i].innerProduct(viewVec);

				if (1.0f + denom != 1.0f) {
					lambda[i]     = (-(float)_clipEqn[3] - vecStart[i].innerProduct(viewVec))/denom;//
				} else {
					lambda[i]     = -1.0f;
				}

				dVertices[3*i]   = vecStart[i].x;
				dVertices[3*i+1] = vecStart[i].y;
				dVertices[3*i+2] = vecStart[i].z;
				dEdges[3*i]      = vecDir[i].x;
				dEdges[3*i+1]    = vecDir[i].y;
				dEdges[3*i+2]    = vecDir[i].z;
			};

			bool visible=true;
			//lmb:辺のどのあたりに交点があるのかの、割合。

			if      ((lambda[0] >= 0.0) && (lambda[0] < 1.0)) intersection[0] = vecStart[0] + vecDir[0]*lambda[0] ;
			else if ((lambda[1] >= 0.0) && (lambda[1] < 1.0)) intersection[0] = vecStart[1] + vecDir[1]* lambda[1] ;
			else if ((lambda[3] >= 0.0) && (lambda[3] < 1.0)) intersection[0] = vecStart[3] + vecDir[3]* lambda[3] ;
			else {return false;}//このボックスにクリッピング面は交差していません。

			if	    ((lambda[2] >= 0.0) && (lambda[2] < 1.0)) intersection[1] = vecStart[2] + vecDir[2]*lambda[2] ;
			else if ((lambda[0] >= 0.0) && (lambda[0] < 1.0)) intersection[1] = vecStart[0] + vecDir[0]*lambda[0] ;
			else if ((lambda[1] >= 0.0) && (lambda[1] < 1.0)) intersection[1] = vecStart[1] + vecDir[1]*lambda[1] ;
			else intersection[1] = vecStart[3] + vecDir[3] * lambda[3];

			if      ((lambda[4] >= 0.0) && (lambda[4] < 1.0)) intersection[2] = vecStart[4] +vecDir[4]* lambda[4] ;
			else if ((lambda[5] >= 0.0) && (lambda[5] < 1.0)) intersection[2] = vecStart[5] +  vecDir[5]*lambda[5] ;
			else intersection[2] = vecStart[7] + vecDir[7]*lambda[7]  ;

			if	    ((lambda[6] >= 0.0) && (lambda[6] < 1.0)) intersection[3] = vecStart[6] + vecDir[6]*lambda[6]  ;
			else if ((lambda[4] >= 0.0) && (lambda[4] < 1.0)) intersection[3] = vecStart[4] +  vecDir[4]*lambda[4] ;
			else if ((lambda[5] >= 0.0) && (lambda[5] < 1.0)) intersection[3] = vecStart[5] +  vecDir[5]*lambda[5] ;
			else intersection[3] = vecStart[7] + vecDir[7]*lambda[7];

			if	    ((lambda[8] >= 0.0) && (lambda[8] < 1.0)) intersection[4] = vecStart[8] +  vecDir[8]*lambda[8]  ;
			else if ((lambda[9] >= 0.0) && (lambda[9] < 1.0)) intersection[4] = vecStart[9] +  vecDir[9]*lambda[9]  ;
			else intersection[4] = vecStart[11]+ vecDir[11]*lambda[11];

			if	    ((lambda[10]>= 0.0) && (lambda[10]< 1.0)) intersection[5] = vecStart[10]+ vecDir[10]*lambda[10]; 
			else if ((lambda[8] >= 0.0) && (lambda[8] < 1.0)) intersection[5] = vecStart[8] + vecDir[8]*lambda[8]  ;
			else if ((lambda[9] >= 0.0) && (lambda[9] < 1.0)) intersection[5] = vecStart[9] + vecDir[9]*lambda[9]   ;
			else intersection[5] = vecStart[11]+ vecDir[11]*lambda[11];

			return visible;

		}
			//_distCam=視点と平面の距離　0にしたら箱の真中に平面が来るけど。。？？
		bool CalcClipPlaneVerts(float _modelview[16],float _distCam,vec3<T> intersection[6]){
			vec3<float> viewVec(-_modelview[2],-_modelview[6],-_modelview[10]);

			float dMaxDist = viewVec.innerProduct( corner[0]);
			float dMinDist = dMaxDist;
			int nMaxIdx = 0;
			for(int i = 1; i < 8; ++i) {
				float dist = viewVec.innerProduct( corner[i]);
				if ( dist > dMaxDist) {
					dMaxDist = dist;
					nMaxIdx = i;
				}
				if ( dist < dMinDist) {
					dMinDist = dist;
				}

			}
			vec3<float> vecStart[12];
			vec3<float> vecDir[12];
			float lambda[12];
			float denom;

			dMinDist += FLT_EPSILON;
			dMaxDist -= FLT_EPSILON;
			
			float dVertices[12*3];
			float dEdges[12*3];

			for(int i = 0; i < 12; i++) {
				vecStart[i] = corner[edges[edgeList[nMaxIdx][i]].nV1];
				vecDir[i]   = corner[edges[edgeList[nMaxIdx][i]].nV2] - corner[edges[edgeList[nMaxIdx][i]].nV1];

				denom = vecDir[i].innerProduct(viewVec);

				if (1.0 + denom != 1.0) {
					lambda[i]     = (_distCam - vecStart[i].innerProduct(viewVec))/denom;//
				} else {
					lambda[i]     = -1.0;
				}

				dVertices[3*i]   = vecStart[i].x;
				dVertices[3*i+1] = vecStart[i].y;
				dVertices[3*i+2] = vecStart[i].z;
				dEdges[3*i]      = vecDir[i].x;
				dEdges[3*i+1]    = vecDir[i].y;
				dEdges[3*i+2]    = vecDir[i].z;
			};

			bool visible=true;
			//lmb:辺のどのあたりに交点があるのかの、割合。

			if      ((lambda[0] >= 0.0) && (lambda[0] < 1.0)) intersection[0] = vecStart[0] + vecDir[0]*lambda[0] ;
			else if ((lambda[1] >= 0.0) && (lambda[1] < 1.0)) intersection[0] = vecStart[1] + vecDir[1]* lambda[1] ;
			else if ((lambda[3] >= 0.0) && (lambda[3] < 1.0)) intersection[0] = vecStart[3] + vecDir[3]* lambda[3] ;
			else {return false;}//このボックスにクリッピング面は交差していません。

			if	    ((lambda[2] >= 0.0) && (lambda[2] < 1.0)) intersection[1] = vecStart[2] + vecDir[2]*lambda[2] ;
			else if ((lambda[0] >= 0.0) && (lambda[0] < 1.0)) intersection[1] = vecStart[0] + vecDir[0]*lambda[0] ;
			else if ((lambda[1] >= 0.0) && (lambda[1] < 1.0)) intersection[1] = vecStart[1] + vecDir[1]*lambda[1] ;
			else intersection[1] = vecStart[3] + vecDir[3] * lambda[3];

			if      ((lambda[4] >= 0.0) && (lambda[4] < 1.0)) intersection[2] = vecStart[4] +vecDir[4]* lambda[4] ;
			else if ((lambda[5] >= 0.0) && (lambda[5] < 1.0)) intersection[2] = vecStart[5] +  vecDir[5]*lambda[5] ;
			else intersection[2] = vecStart[7] + vecDir[7]*lambda[7]  ;

			if	    ((lambda[6] >= 0.0) && (lambda[6] < 1.0)) intersection[3] = vecStart[6] + vecDir[6]*lambda[6]  ;
			else if ((lambda[4] >= 0.0) && (lambda[4] < 1.0)) intersection[3] = vecStart[4] +  vecDir[4]*lambda[4] ;
			else if ((lambda[5] >= 0.0) && (lambda[5] < 1.0)) intersection[3] = vecStart[5] +  vecDir[5]*lambda[5] ;
			else intersection[3] = vecStart[7] + vecDir[7]*lambda[7];

			if	    ((lambda[8] >= 0.0) && (lambda[8] < 1.0)) intersection[4] = vecStart[8] +  vecDir[8]*lambda[8]  ;
			else if ((lambda[9] >= 0.0) && (lambda[9] < 1.0)) intersection[4] = vecStart[9] +  vecDir[9]*lambda[9]  ;
			else intersection[4] = vecStart[11]+ vecDir[11]*lambda[11];

			if	    ((lambda[10]>= 0.0) && (lambda[10]< 1.0)) intersection[5] = vecStart[10]+ vecDir[10]*lambda[10]; 
			else if ((lambda[8] >= 0.0) && (lambda[8] < 1.0)) intersection[5] = vecStart[8] + vecDir[8]*lambda[8]  ;
			else if ((lambda[9] >= 0.0) && (lambda[9] < 1.0)) intersection[5] = vecStart[9] + vecDir[9]*lambda[9]   ;
			else intersection[5] = vecStart[11]+ vecDir[11]*lambda[11];

			return visible;

		}
#endif
	};
	template <typename T>
	struct RoomCube:cube<T>{
		RoomCube(){
			set(vec3<T>(0.0,0.0,0.0),(T)0.5);
		}
		RoomCube(T _size,vec3<T>& _offset){
			for(int i=0;i<8;i++){corner[i]=corner[i]*_size+_offset;}
		}
		GLuint mTexId[6];
		void set(vec3<T> &_center,T _size){cube::set(_center,_size);}
		void DrawQuadsWithTexture(){
			 glEnable(GL_TEXTURE_2D);
				
				//positive x
				glBindTexture(GL_TEXTURE_2D , mTexId[0]);
				glBegin(GL_QUADS);
				glTexCoord2i(0,0);corner[5].glVertex();glTexCoord2i(0,1);corner[6].glVertex();
				glTexCoord2i(1,1);corner[2].glVertex();glTexCoord2i(1,0);corner[1].glVertex();
				glEnd();
				//negative x
				glBindTexture(GL_TEXTURE_2D , mTexId[1]);
				glBegin(GL_QUADS);
				glTexCoord2i(0,0);corner[0].glVertex();glTexCoord2i(0,1);corner[3].glVertex();
				glTexCoord2i(1,1);corner[7].glVertex();glTexCoord2i(1,0);corner[4].glVertex();
				glEnd();
				//positive y
				glBindTexture(GL_TEXTURE_2D , mTexId[2]);
				glBegin(GL_QUADS);
				glTexCoord2i(0,0);corner[7].glVertex();glTexCoord2i(0,1);corner[3].glVertex();
				glTexCoord2i(1,1);corner[2].glVertex();glTexCoord2i(1,0);corner[6].glVertex();
				glEnd();
				//negative y
				glBindTexture(GL_TEXTURE_2D , mTexId[3]);
				glBegin(GL_QUADS);
				glTexCoord2i(0,0);corner[0].glVertex();glTexCoord2i(0,1);corner[4].glVertex();
				glTexCoord2i(1,1);corner[5].glVertex();glTexCoord2i(1,0);corner[1].glVertex();
				glEnd();
				//positive z
				glBindTexture(GL_TEXTURE_2D , mTexId[4]);
				glBegin(GL_QUADS);
				glTexCoord2i(0,0);corner[4].glVertex();glTexCoord2i(0,1);corner[7].glVertex();
				glTexCoord2i(1,1);corner[6].glVertex();glTexCoord2i(1,0);corner[5].glVertex();
				glEnd();
				//negative z
				glBindTexture(GL_TEXTURE_2D , mTexId[5]);
				glBegin(GL_QUADS);
				glTexCoord2i(0,0);corner[1].glVertex();glTexCoord2i(0,1);corner[2].glVertex();
				glTexCoord2i(1,1);corner[3].glVertex();glTexCoord2i(1,0);corner[0].glVertex();
				glEnd();
			 glDisable(GL_TEXTURE_2D);
		}
	};
}
#endif
