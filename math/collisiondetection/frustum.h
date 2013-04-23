#ifndef MIFFY_FRUSTUM
#define MIFFY_FRUSTUM
 
#pragma once
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <windows.h>
 
 
#include <stdlib.h>
 
#include <fstream>
#include <string>
#include <sstream>
#include <memory.h>
using namespace std;
//#include <GL/glew.h>
#ifndef  __FREEGLUT_H__
#include <GL/freeglut.h>
#endif
#ifndef MIFFY_VEC3
#include <miffy/math/vec3.h>
#endif
 #ifndef MIFFY_VEC4
#include <miffy/math/vec4.h>
#endif
#ifndef MIFFY_PLANE
#include <miffy/math/collisiondetection/plane.h>
#endif
 
#ifndef MIFFY_AABOX
#include <miffy/math/collisiondetection/aabox.h>
#endif
#ifndef MIFFY_MATRIX
#include <miffy/math/matrix.h>
#endif
namespace miffy{
  static const double PI = 6.0*asin( 0.5 );
    //行列の掛け算
  template <typename T>
    static void multMatrix(T matrix[], float org[4], float dst[4]) {
    float tmp[4];
    tmp[0] = org[0]; tmp[1] = org[1]; tmp[2] = org[2]; tmp[3] = org[3];
    for (int i = 0; i < 4; i++) {
        dst[i] = 0.0;
        for (int j = 0; j < 4; j++) {
            dst[i] += (float)matrix[j*4+i] * tmp[j];
        }
    }
}
/// getNearDistance Planeだとしたら、？
template <typename T>
static T getNDPlane(const mat4<T>& _projmatrix,T _plane)
{
    vec4<T> planePos(0.0f,0.0f,_plane,1.0f);
 
     
    planePos=_projmatrix*planePos;
     
    return planePos.z/planePos.w;
}
/// たぶん、視野角を求めると思われる。
template <typename T>
static void getNDRad(const mat4<T>& _projmatrix,vec3<T>* plane)
{
    vec4<T> planePos(*plane);
  
   planePos= _projmatrix*planePos;
     //長さ求めてるのか？？？？
    plane->x /= planePos.x;
    plane->y /= planePos.y;
    plane->z /= planePos.z;
}
template <typename T>
class frustum{
    private:
        enum {
            TOP = 0,
            BOTTOM,
            LEFT,
            RIGHT,
            NEARP,
            FARP
        };
        public:
 
    static enum {OUTSIDE, INTERSECT, INSIDE};
    static enum FrustumName{normal,back,render};
 
    plane<T> plane[6];
 
    vec3<T> camera;
    vec3<T> ntl,ntr,nbl,nbr,ftl,ftr,fbl,fbr;
    float nearD, farD, ratio, angle,tang;
    float nw,nh,fw,fh;
    float radious1,radious2;
    float dis1,dis2;
 
    frustum(){}
    ~frustum(){}
    /*!
    @brief 用は、gluPerspectiveと同じ引数で視錐台を作るってことだよね。
    */
    void setFromPerspective(const mat4<T>& _projmatrix,float _angle, float _ratio, float _nearD, float _farD){
        ratio = _ratio;
        angle = _angle;
        nearD = _nearD;
        farD = _farD;
 
        tang = (float)tan(_angle* PI/180.0 * 0.5) ;//もしかしたら定数のほうが速いかも
        nh = _nearD * tang;
        nw = nh * _ratio; 
        fh = _farD  * tang;
        fw = fh *_ratio;
 
 
        float NDnear = getNDPlane(_projmatrix,-nearD);
        float NDfar  = getNDPlane(_projmatrix,-farD);
 
        dis1 = NDnear + (NDfar-NDnear)/3.0f;
        dis2 = NDnear + 2.0f*(NDfar-NDnear)/3.0f;
     
        vec3<T> NDNearPos(nw,nh,-nearD);
      
        getNDRad(_projmatrix,&NDNearPos);
 
        if(NDNearPos.x >= NDNearPos.y){
            radious1 = NDNearPos.y/3.0f;
            radious2 = 2.0f*NDNearPos.y/3.0f;
        }
        else{
            radious1 = NDNearPos.x/3.0f;
            radious2 = 2.0f*NDNearPos.y/3.0f;
        }
}
 
    void setCamDef(vec3<T> &_p, vec3<T> &_l, vec3<T> &_u){//gluLookAtと同じ（p=カメラ、l=注視点、u=アッパーベクタ）//display関数の中で呼び出される
 
        vec3<T>  dir,nc,fc,X,Y,Z;
 
        //Z = p - l;
        Z = _l;//注視点
        Z.normalize();
 
        X =Z.cross(_u);// Z * u;//u=upper vector
        X.normalize();
 
        Y = -Z.cross(X);//(Z * X);
 
        //nc = p - Z * nearD;
        nc = _p + Z * nearD;
        //fc = p - Z * farD;
        fc = _p + Z * farD;
 
        //n for near
        ntl = nc + Y * nh - X * nw;//near top left
        ntr = nc + Y * nh + X * nw;//near top right
        nbl = nc - Y * nh - X * nw;//near bottom left
        nbr = nc - Y * nh + X * nw;//near bottom right
        //f for far
        ftl = fc + Y * fh - X * fw;//far top left
        ftr = fc + Y * fh + X * fw;//far top right
        fbl = fc - Y * fh - X * fw;//far bottom left
        fbr = fc - Y * fh + X * fw;//far bottom right
        //pl for plane(視錐台を構成する６つの平面)
        plane[TOP].set3Points(ntr,ntl,ftl);
        plane[BOTTOM].set3Points(nbl,nbr,fbr);
        plane[LEFT].set3Points(ntl,nbl,fbl);
        plane[RIGHT].set3Points(nbr,ntr,fbr);
        plane[NEARP].set3Points(ntl,ntr,nbr);
        plane[FARP].set3Points(ftr,ftl,fbl);
 
        camera = _p;
}
    int pointInFrustum(vec3<T> &_p) {
 
        int result = INSIDE;
        for(int i=0; i < 6; i++) {
 
            if (plane[i].distance(_p) < 0)
                return OUTSIDE;
        }
        return(result);
 
    }
    int sphereInFrustum(vec3<T> &_p, float _raio){
 
        int result = INSIDE;
        float distance;
 
        for(int i=0; i < 6; i++) {
            distance = pl[i].distance(_p);
            if (distance < -_raio)
                return OUTSIDE;
            else if (distance < _raio)
                result =  INTERSECT;
        }
        return(result);
 
    }
    //この判定大丈夫？
    int boxInFrustum(aabox<T> &_b)const {
        int result = INSIDE;
        for(int i=0; i < 6; i++) {
                 
            if (plane[i].getDistance(_b.getVertexP(plane[i].normal)) < 0){//P for positive
                return OUTSIDE;}
            else if (plane[i].getDistance(_b.getVertexN(plane[i].normal)) < 0){//N for negative
                result =  INTERSECT;
            }
        }
        return result;
    }
 
    void drawPoints() {
        glBegin(GL_POINTS);
 
            glVertex3f(ntl.x,ntl.y,ntl.z);
            glVertex3f(ntr.x,ntr.y,ntr.z);
            glVertex3f(nbl.x,nbl.y,nbl.z);
            glVertex3f(nbr.x,nbr.y,nbr.z);
 
            glVertex3f(ftl.x,ftl.y,ftl.z);
            glVertex3f(ftr.x,ftr.y,ftr.z);
            glVertex3f(fbl.x,fbl.y,fbl.z);
            glVertex3f(fbr.x,fbr.y,fbr.z);
 
        glEnd();
    }
 
    void drawLines() {  
        vec3<float> pos = (ftr + ftl)/2.0f;
        vec3<float> dir = ftr - fbr;
        dir = dir*0.1f;
        glBegin(GL_LINE_LOOP);
            //near plane
            ntl.glVertex();
            ntr.glVertex();
            nbr.glVertex();
            nbl.glVertex();
        glEnd();
 
        glBegin(GL_LINE_LOOP);
            //far plane
            ftr.glVertex();
            ftl.glVertex();
            fbl.glVertex();
            fbr.glVertex();
        glEnd();
 
        glBegin(GL_LINES);
            camera.glVertex();
            ftr.glVertex();
            camera.glVertex();
            ftl.glVertex();
            camera.glVertex();
            fbl.glVertex();
            camera.glVertex();
            fbr.glVertex();
            pos.glVertex();
            //上を意味する線
            (pos+dir).glVertex();
             
        glEnd();
    }
    void drawPlanes() {
        glBegin(GL_QUADS);
 
        //near plane
            ntl.glVertex();
            ntr.glVertex();
            nbr.glVertex();
            nbl.glVertex();
        //far plane
            ftr.glVertex();
            ftl.glVertex();
            fbl.glVertex();
            fbr.glVertex();
 
        //bottom plane
            nbl.glVertex();
            nbr.glVertex();
            fbr.glVertex();
            fbl.glVertex();
        //top plane
            ntr.glVertex();
            ntl.glVertex();
            ftl.glVertex();
            ftr.glVertex();
        //left plane
            ntl.glVertex();
            nbl.glVertex();
            fbl.glVertex();
            ftl.glVertex();
        // right plane
            nbr.glVertex();
            ntr.glVertex();
            ftr.glVertex();
            fbr.glVertex();
        glEnd();
 
    }
    void drawNormals(){
 
        vec3<T> a,b;
 
        glBegin(GL_LINES);
 
            // near
            a = (ntr + ntl + nbr + nbl) * 0.25;
            b = a + plane[NEARP].normal;
            a.glVertex();
            b.glVertex();
 
            // far
            a = (ftr + ftl + fbr + fbl) * 0.25;
            b = a + plane[FARP].normal;
            a.glVertex();
            b.glVertex();
 
            // left
            a = (ftl + fbl + nbl + ntl) * 0.25;
            b = a + plane[LEFT].normal;
            a.glVertex();
            b.glVertex();
         
            // right
            a = (ftr + nbr + fbr + ntr) * 0.25;
            b = a + plane[RIGHT].normal;
            a.glVertex();
            b.glVertex();
         
            // top
            a = (ftr + ftl + ntr + ntl) * 0.25;
            b = a + pl[TOP].normal;
            a.glVertex();
            b.glVertex();
         
            // bottom
            a = (fbr + fbl + nbr + nbl) * 0.25;
            b = a + plane[BOTTOM].normal;
            a.glVertex();
            b.glVertex();
 
        glEnd();
 
 
    }
    /// deprecated I don't know why?
    bool IsLineInBox(vec3<T> &L1,vec3<T> &L2,aabox<T> &b){    
        // Put line in box space
        //CMatrix MInv = m_M.InvertSimple();
        Vec3 LB1 = L1 - b.corner;
        Vec3 LB2 = L2 - b.corner;
        Vec3 m_Extent = Vec3(b.x,b.y,b.z);
        // Get line midpoint and extent
        Vec3 LMid = (LB1 + LB2) * 0.5f; 
        Vec3 L = (LB1 - LMid);
        Vec3 LExt = Vec3( fabs(L.x), fabs(L.y), fabs(L.z) );
 
        // Use Separating Axis Test
        // Separation vector from box center to line center is LMid, since the line is in box space
        if ( fabs( LMid.x ) > m_Extent.x + LExt.x ) return false;
        if ( fabs( LMid.y ) > m_Extent.y + LExt.y ) return false;
        if ( fabs( LMid.z ) > m_Extent.z + LExt.z ) return false;
        // Crossproducts of line and each axis
        if ( fabs( LMid.y * L.z - LMid.z * L.y)  >  (m_Extent.y * LExt.z + m_Extent.z * LExt.y) ) return false;
        if ( fabs( LMid.x * L.z - LMid.z * L.x)  >  (m_Extent.x * LExt.z + m_Extent.z * LExt.x) ) return false;
        if ( fabs( LMid.x * L.y - LMid.y * L.x)  >  (m_Extent.x * LExt.y + m_Extent.y * LExt.x) ) return false;
        // No separating axis, the line intersects
        return true;
    }
    /// deprecated I don' know why?
    bool EdgeIntersectVF(vec3<T>& Start, vec3<T>& End){
          Vec3  Dir = End-Start;            // CALC DIRECTION VECTOR OF EDGE
          float InT=-99999, OutT=99999;      // INIT INTERVAL T-VAL ENDPTS TO -/+ INFINITY
          float NdotDir, NdotStart;          // STORAGE FOR REPEATED CALCS NEEDED FOR NewT CALC
          float NewT;
          for (int i=0; i<6; i++)            // CHECK INTERSECTION AGAINST EACH VF PLANE
          {
              NdotDir = plane[i].normal.innerProduct(Dir);
              NdotStart = plane[i].normal.innerProduct(Start);
            if (NdotDir == 0)                // CHECK IF RAY IS PARALLEL TO THE SLAB PLANES
            {
                if (NdotStart > plane[i].distance(Start)) return false; // IF STARTS "OUTSIDE", NO INTERSECTION
            }
            else
            {
              NewT = (plane[i].distance(Start) - NdotStart) / NdotDir;      // FIND HIT "TIME" (DISTANCE)
              if (NdotDir < 0) { if (NewT > InT) InT=NewT; }   // IF "FRONTFACING", MUST BE NEW IN "TIME"
                          else { if (NewT < OutT) OutT=NewT; } // IF "BACKFACING", MUST BE NEW OUT "TIME"
            }
            if (InT > OutT) return false;   // CHECK FOR EARLY EXITS (INTERSECTION INTERVAL "OUTSIDE")
          }
 
          // IF AT LEAST ONE THE Tvals ARE IN THE INTERVAL [0,1] WE HAVE INTERSECTION
          if (InT>=0 && InT<=1) return true;
          else if (OutT>=0 && OutT<=1) return true;
          else return false;
        }
};
}
#endif
