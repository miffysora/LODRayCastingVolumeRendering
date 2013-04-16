#ifndef MIFFY_MATRIX
#define MIFFY_MATRIX
namespace miffy{
  template <typename T>
	struct mat4{
		T m[16];//ここ。ポインタにすると色々エラーに成る
		mat4<T>(){
			m[0]=1.0;m[4]=0;m[8]=0; m[12]=0;
			m[1]=0;m[5]=1.0;m[9]=0; m[13]=0;
			m[2]=0;m[6]=0;m[10]=1.0;m[14]=0;
			m[3]=0;m[7]=0;m[11]=0;m[15]=1.0;
		}
		mat4<T>(T _m0,T _m1,T _m2,T _m3,T _m4,T _m5,T _m6,T _m7,T _m8,T _m9,T _m10,T _m11,T _m12,T _m13,T _m14,T _m15){	
			m[0]=_m0;m[4]=_m4;m[8]=_m8;m[12]=_m12;
			m[1]=_m1;m[5]=_m5;m[9]=_m9;m[13]=_m13;
			m[2]=_m2;m[6]=_m6;m[10]=_m10;m[14]=_m14;
			m[3]=_m3;m[7]=_m7;m[11]=_m11;m[15]=_m15;
		}
		~mat4<T>(){}
		T operator[](int _index)const{
			return m[_index];
		}

		mat4<T> operator =(const mat4<T> &_in){
			m[0]=_in[0];m[4]=_in[4]; m[8]= _in[8];m[12]=_in[12];
			m[1]=_in[1];m[5]=_in[5]; m[9]= _in[9];m[13]=_in[13];
			m[2]=_in[2];m[6]=_in[6];m[10]=_in[10];m[14]=_in[14];
			m[3]=_in[3];m[7]=_in[7];m[11]=_in[11];m[15]=_in[15];
			return *this;
		}
		void copy(const mat4<T> &_in){
			m[0]=_in[0];m[4]=_in[4]; m[8]= _in[8];m[12]=_in[12];
			m[1]=_in[1];m[5]=_in[5]; m[9]= _in[9];m[13]=_in[13];
			m[2]=_in[2];m[6]=_in[6];m[10]=_in[10];m[14]=_in[14];
			m[3]=_in[3];m[7]=_in[7];m[11]=_in[11];m[15]=_in[15];
		}
		mat4<T> operator*=(const mat4<T> &_r){//rightのr]
			m[0]=_r[0]*m[0]+_r[1]*m[4]+_r[2]*m[8]+_r[3]*m[12];
			m[1]=_r[0]*m[1]+_r[1]*m[5]+_r[2]*m[9]+_r[3]*m[13];
			m[2]=_r[0]*m[2]+_r[1]*m[6]+_r[2]*m[10]+_r[3]*m[14];
			m[3]=_r[0]*m[3]+_r[1]*m[7]+_r[2]*m[11]+_r[3]*m[15];

			m[4]=_r[4]*m[0]+_r[5]*m[4]+_r[6]*m[8]+_r[7]*m[12];
			m[5]=_r[4]*m[1]+_r[5]*m[5]+_r[6]*m[9]+_r[7]*m[13];
			m[6]=_r[4]*m[2]+_r[5]*m[6]+_r[6]*m[10]+_r[7]*m[14];
			m[7]=_r[4]*m[3]+_r[5]*m[7]+_r[6]*m[11]+_r[7]*m[15];

			m[8]=_r[8]*m[0]+_r[9]*m[4]+_r[10]*m[8]+_r[11]*m[12];
			m[9]=_r[8]*m[1]+_r[9]*m[5]+_r[10]*m[9]+_r[11]*m[13];
			m[10]=_r[8]*m[2]+_r[9]*m[6]+_r[10]*m[10]+_r[11]*m[14];
			m[11]=_r[8]*m[3]+_r[9]*m[7]+_r[10]*m[11]+_r[11]*m[15];

			m[12]=_r[12]*m[0]+_r[13]*m[4]+_r[14]*m[8]+_r[15]*m[12];
			m[13]=_r[12]*m[1]+_r[13]*m[5]+_r[14]*m[9]+_r[15]*m[13];
			m[14]=_r[12]*m[2]+_r[13]*m[6]+_r[14]*m[10]+_r[15]*m[14];
			m[15]=_r[12]*m[3]+_r[13]*m[7]+_r[14]*m[11]+_r[15]*m[15];
			return *this;
		}
		mat4<T> operator*(const mat4<T> &_r)const{//rightのr]
			return mat4<T>(
				_r[0]*m[0]+_r[1]*m[4]+_r[2]*m[8]+_r[3]*m[12],
				_r[0]*m[1]+_r[1]*m[5]+_r[2]*m[9]+_r[3]*m[13],
				_r[0]*m[2]+_r[1]*m[6]+_r[2]*m[10]+_r[3]*m[14],
				_r[0]*m[3]+_r[1]*m[7]+_r[2]*m[11]+_r[3]*m[15],

				_r[4]*m[0]+_r[5]*m[4]+_r[6]*m[8]+_r[7]*m[12],
				_r[4]*m[1]+_r[5]*m[5]+_r[6]*m[9]+_r[7]*m[13],
				_r[4]*m[2]+_r[5]*m[6]+_r[6]*m[10]+_r[7]*m[14],
				_r[4]*m[3]+_r[5]*m[7]+_r[6]*m[11]+_r[7]*m[15],

				_r[8]*m[0]+_r[9]*m[4]+_r[10]*m[8]+_r[11]*m[12],
				_r[8]*m[1]+_r[9]*m[5]+_r[10]*m[9]+_r[11]*m[13],
				_r[8]*m[2]+_r[9]*m[6]+_r[10]*m[10]+_r[11]*m[14],
				_r[8]*m[3]+_r[9]*m[7]+_r[10]*m[11]+_r[11]*m[15],

				_r[12]*m[0]+_r[13]*m[4]+_r[14]*m[8]+_r[15]*m[12],
				_r[12]*m[1]+_r[13]*m[5]+_r[14]*m[9]+_r[15]*m[13],
				_r[12]*m[2]+_r[13]*m[6]+_r[14]*m[10]+_r[15]*m[14],
				_r[12]*m[3]+_r[13]*m[7]+_r[14]*m[11]+_r[15]*m[15]);
		}
#ifdef MIFFY_VEC4
		vec4<T> operator*(const vec4<T> &_r)const{//rightのr]
			return vec4<T>(
				m[0]*(_r.x)+m[4]*(_r.y)+m[8]*(_r.z)+m[12]*(_r.w),
				m[1]*(_r.x)+m[5]*(_r.y)+m[9]*(_r.z)+m[13]*(_r.w),
				m[2]*(_r.x)+m[6]*(_r.y)+m[10]*(_r.z)+m[14]*(_r.w),
				m[3]*(_r.x)+m[7]*(_r.y)+m[11]*(_r.z)+m[15]*(_r.w));
		}
#endif
#ifdef MIFFY_VEC3
		vec3<T> operator*(const vec3<T> &_r){//rightのr]
			return vec3<T>(
				m[0]*(_r.x)+m[4]*(_r.y)+m[8]*(_r.z),
				m[1]*(_r.x)+m[5]*(_r.y)+m[9]*(_r.z),
				m[2]*(_r.x)+m[6]*(_r.y)+m[10]*(_r.z));
				//m[3]*(_r.x)+m[7]*(_r.y)+m[11]*(_r.z)+m[15]*((T)1.0));
		}
#endif
		void glFrustum(T _l,T _r,T _b,T _t,T _n,T _f){
			m[0]=2.0f*_n/(_r-_l);	m[4]=0.0f;				m[8]=(_r+_l)/(_r-_l);	m[12]=0.0f;
			m[1]=0.0f;				m[5]=2.0*_n/(_t-_b);	m[9]=(_t+_b)/(_t-_b);	m[13]=0.0f;
			m[2]=0.0f;				m[6]=0.0f;				m[10]=-(_f+_n)/(_f-_n);	m[14]=-(2.0*_f*_n)/(_f-_n);
			m[3]=0.0f;				m[7]=0.0f;				m[11]=-1.0f;			m[15]=0.0f;
		}
		void glScale(T _x,T _y,T _z){
			mat4<T> temp(
					_x,0.0,0.0,0.0,
					0.0,_y,0.0,0.0,
					0.0,0.0,_z,0.0,
					0.0,0.0,0.0,1.0);
			*this=(*this)*temp;//temp*(*this);//
		}
		void LoadIdentity(){
			for(int i=0;i<16;i++){
				if(i%5==0){
					m[i]=1.0f;
				}else{
					m[i]=0.0f;
				}
			}
		}
		float getDeterminant() //行列式を得る
		{
			return(
				m[0] * getSubDeterminant(0,0)
				- m[1*4+0] * getSubDeterminant(1,0)
				+ m[2*4+0] * getSubDeterminant(2,0)
				- m[3*4+0] * getSubDeterminant(3,0)
				);
		}
		//小行列の行列式を得る
		float getSubDeterminant(const unsigned short rowIndex_in,const unsigned short colIndex_in)
		{
			if( (rowIndex_in > 3) || (colIndex_in > 3) )
				throw;
			float subMatrix[3][3];
			int srcRow;
			int srcCol;

			srcRow = 0;
			for(int dstRow=0; dstRow<3; dstRow++)
			{
				if(dstRow == rowIndex_in)
					srcRow++;

				srcCol = 0;
				for(int dstCol=0; dstCol<3; dstCol++)
				{
					if(dstCol == colIndex_in)
						srcCol++;

					subMatrix[dstRow][dstCol] = m[srcRow*4+srcCol];

					srcCol++;
				}

				srcRow++;
			}

			float dSubDet = 
				subMatrix[0][0] * subMatrix[1][1] * subMatrix[2][2]
			+ subMatrix[1][0] * subMatrix[2][1] * subMatrix[0][2]
			+ subMatrix[0][1] * subMatrix[1][2] * subMatrix[2][0]
			-    subMatrix[2][0] * subMatrix[1][1] * subMatrix[0][2]
			-    subMatrix[2][1] * subMatrix[1][2] * subMatrix[0][0]
			-    subMatrix[1][0] * subMatrix[0][1] * subMatrix[2][2];

			return dSubDet;
		}
		//TODO:直行行列で楽にできる方法ないかなぁー。
		bool inv(mat4<T>* _dest){//自身を逆行列に変える
			mat4<T> copied=*this;
			float det;
			det = getDeterminant();//行列式を得る

			if(fabs(det) ==0.0){//行列式=0なら逆行列なし
				return false;
			}

			//サラスの公式
			_dest->m[0]=getSubDeterminant(0,0) / det;
			_dest->m[1]= -getSubDeterminant(1,0) / det;
			_dest->m[2]=getSubDeterminant(2,0) / det;
			_dest->m[3]=-getSubDeterminant(3,0) / det;

			_dest->m[4]=-getSubDeterminant(0,1) / det;
			_dest->m[5]=getSubDeterminant(1,1) / det;
			_dest->m[6]=-getSubDeterminant(2,1) / det;
			_dest->m[7]=getSubDeterminant(3,1) / det;

			_dest->m[8]=getSubDeterminant(0,2) / det;
			_dest->m[9]=-getSubDeterminant(1,2) / det;
			_dest->m[10]=getSubDeterminant(2,2) / det;
			_dest->m[11]=-getSubDeterminant(3,2) / det;

			_dest->m[12]=-getSubDeterminant(0,3) / det;
			_dest->m[13]=getSubDeterminant(1,3) / det;
			_dest->m[14]=-getSubDeterminant(2,3) / det;
			_dest->m[15]=getSubDeterminant(3,3) / det;
			return true;
		}

		void glOrtho(T _left,T _right,T _bottom,T _top,T _near,T _far){
			m[0]=2.0f/(_right-_left);m[4]=0.0f;				m[8]=0.0f;				m[12]=-(_right+_left)/(_right-_left);			
			m[1]=0.0f;				m[5]=2.0f/(_top-_bottom);m[9]=0.0f;				m[13]=-(_top+_bottom)/(_top-_bottom);
			m[2]=0.0f;				m[6]=0.0f;				m[10]=-2.0/(_far-_near);m[14]=-(_far+_near)/(_far-_near);
			m[3]=0.0f;				m[7]=0.0f;				m[11]=0.0f;				m[15]=1.0f;
			
			
			
		}
		/// 透視投影行列を作るかのような引数から正投影行列を作成する。
		/*!
			透視投影行列とスムーズに変換できて便利なはず
			@param _aspect height/widthを渡す
			@param _range なんだか忘れたけど-m_Zoom+mEyeCam.zがおすすめ
		*/
		void glOrtho(T _fovy,T _aspect,T _near,T _far,T _range){
			T orthol=_near*(T)tan(_fovy*(T)M_PI/360.0);
			T size=orthol/_near*(_range);
			if(_aspect>(T)1.0){
				glOrtho( size*_aspect,-size*_aspect,size       ,-size       ,_near,_far);
			}else{
				glOrtho(size       ,-size       ,size*(1.0f/_aspect),-size*(1.0/_aspect),_near,_far);
			}
		}
		/// この関数ではデフォルトが、x=画面左 y=画面下 z=画面手前になる
		/// まぁこれは本家のglPerspectiveと同じ仕様だ。
		void gluPerspective(T _fovy,T _aspect,T _near,T _far){
			T xmin, xmax, ymin, ymax;

			ymax = _near * (T)tan(_fovy * M_PI / 360.0);
			ymin = -ymax;
			xmin = ymin * _aspect;
			xmax = ymax * _aspect;
			//printf("left=%f,right=%f,bottom=%f,top=%f\n",xmin,xmax,ymin,ymax);
			glFrustum(xmin, xmax ,ymin,ymax,_near,_far);
		}
		void glTranslate(float x,float y,float z){
			m[12]+=x;			
			m[13]+=y;
			m[14]+=z;

		}
		void glTranslate(vec3<float>& _tra){
			m[12]=_tra.x;			
			m[13]=_tra.y;
			m[14]=_tra.z;

		}
		void gluLookAt(float eyex, float eyey, float eyez,
			float centerx, float centery, float centerz,
			float upx, float upy, float upz)
		{
			float x[3], y[3], z[3];
			float mag;
			mat4<T> model;//モデル行列
			mat4<T> view;//ビュー行列

			/* Make rotation matrix */

			/* 座標軸Z vector */
			z[0] = eyex - centerx;
			z[1] = eyey - centery;
			z[2] = eyez - centerz;
			mag = (float)sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
			if (mag) {			/* mpichler, 19950515 */
				z[0] /= mag;
				z[1] /= mag;
				z[2] /= mag;
			}

			/* 座標軸Y vector */
			y[0] = upx;
			y[1] = upy;
			y[2] = upz;

			/* X vector = Y cross Z *///座標軸のxはy軸とz軸の外積から求めることができる。
			x[0] =  y[1] * z[2] - y[2] * z[1];
			x[1] = -y[0] * z[2] + y[2] * z[0];
			x[2] =  y[0] * z[1] - y[1] * z[0];

			/* Recompute Y = Z cross X */
			y[0] =  z[1] * x[2] - z[2] * x[1];
			y[1] = -z[0] * x[2] + z[2] * x[0];
			y[2] =  z[0] * x[1] - z[1] * x[0];

			/* mpichler, 19950515 */
			/* cross product gives area of parallelogram, which is < 1.0 for
			* non-perpendicular unit-length vectors; so normalize x, y here
			*/
			//各座標軸を正規化
			mag = (float)sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
			if (mag) {
				x[0] /= mag;
				x[1] /= mag;
				x[2] /= mag;
			}

			mag = (float)sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
			if (mag) {
				y[0] /= mag;
				y[1] /= mag;
				y[2] /= mag;
			}

			#define M(row,col)  model.m[col*4+row]
						M(0, 0) = x[0];		M(0, 1) = x[1];		M(0, 2) = x[2];		M(0, 3) = 0.0;
						M(1, 0) = y[0];		M(1, 1) = y[1];		M(1, 2) = y[2];		M(1, 3) = 0.0;
						M(2, 0) = z[0];		M(2, 1) = z[1];		M(2, 2) = z[2];		M(2, 3) = 0.0;
						M(3, 0) = 0.0;		M(3, 1) = 0.0;		M(3, 2) = 0.0;		M(3, 3) = 1.0;
			#undef M
						/* Translate Eye to Origin */
						//glTranslatex((GLfixed)(-eyex * 65536),(GLfixed)(-eyey * 65536),(GLfixed)(-eyez * 65536));//と同じ
						view.m[0]=1;view.m[4]=0;view.m[ 8]=0;view.m[12]=-eyex;
						view.m[1]=0;view.m[5]=1;view.m[ 9]=0;view.m[13]=-eyey;
						view.m[2]=0;view.m[6]=0;view.m[10]=1;view.m[14]=-eyez;
						view.m[3]=0;view.m[7]=0;view.m[11]=0;view.m[15]=1;
						//view.print("view");model.print("model");
						*this=(model*view);
		}
		void glRotateX(float radian){
			m[5]=cos(radian);m[9]=-sin(radian);
			m[6]=sin(radian);m[10]=cos(radian);
		}
		void glRotateY(float radian){
			m[0]=cos(radian);m[8]=sin(radian);
			m[2]=-sin(radian);m[10]=cos(radian);
		}
		void glRotateZ(float radian){
			m[0]=cos(radian);m[4]=-sin(radian);
			m[1]=sin(radian);m[5]=cos(radian);
		}
#ifdef _IOSTREAM_
		friend std::ostream& operator<<<>(std::ostream& _s,const mat4<T>& _in);
#endif
#ifdef _INC_STDIO
		void print(const char* _message){
			printf("%s\n",_message);
			printf("%f,%f,%f,%f\n",m[0],m[4],m[8],m[12]);
			printf("%f,%f,%f,%f\n",m[1],m[5],m[9],m[13]);
			printf("%f,%f,%f,%f\n",m[2],m[6],m[10],m[14]);
			printf("%f,%f,%f,%f\n",m[3],m[7],m[11],m[15]);

		}
#endif
	};//end of struct mat4
#ifdef _IOSTREAM_
		//非メンバ関数
		template <typename T>
		 std::ostream& operator<<<>(std::ostream& _s,const mat4<T>& _in)
			{
				_s<<_in.m[0]<<","<<_in.m[4]<<","<<_in.m[8]<<","<<_in.m[12]<<"\n";
				_s<<_in.m[1]<<","<<_in.m[5]<<","<<_in.m[9]<<","<<_in.m[13]<<"\n";
				_s<<_in.m[2]<<","<<_in.m[6]<<","<<_in.m[10]<<","<<_in.m[14]<<"\n";
				_s<<_in.m[3]<<","<<_in.m[7]<<","<<_in.m[11]<<","<<_in.m[15]<<"\n";
				return _s;
			}
#endif
}//end of namespace miffy
#endif
