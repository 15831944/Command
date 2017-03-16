#include"stdafx.h"
#include"CInterpolationCircle.h"
#define _USE_MATH_DEFINES
#include<math.h>

//#define MFC

CCircleFormula::CCircleFormula()
{

}

CCircleFormula::~CCircleFormula()
{

}
void CCircleFormula::ArcCutPoint(AxeSpace sp, AxeSpace mp, AxeSpace ep, std::vector<AxeSpace> &cutPointsArr, LONG backDistance)
{
    DPoint p1, p2, p3;
    p1.x = sp.x;
    p1.y = sp.y;
    p1.z = sp.z;
    p2.x = mp.x;
    p2.y = mp.y;
    p2.z = mp.z;
    p3.x = ep.x;
    p3.y = ep.y;
    p3.z = ep.z;

    //求法向量
    DOUBLE a, b, c, d;
    FindNormalVector(p1, p2, p3, a, b, c, d);
    //求轉換矩陣(正交矩陣)
    DOUBLE D[9] = { 0 };
    RotateTransformMatrix(a, b, c, d, D);
    //求三維圓心
    DPoint pCen;
    pCen = SpaceCircleCenterCalculation(p1, p2, p3);
    DOUBLE r = sqrt(pow(p1.x-pCen.x, 2)+pow(p1.y-pCen.y, 2)+pow(p1.z-pCen.z, 2));
    //圓心平移至原點
    DPoint Ps, Pm, Pm2;
    Ps.x = p1.x-pCen.x;
    Ps.y = p1.y-pCen.y;
    Ps.z = p1.z-pCen.z;
    Pm.x = p2.x-pCen.x;
    Pm = { p2.x-pCen.x,p2.y-pCen.y,p2.z-pCen.z };
    Pm2 = { p3.x-pCen.x,p3.y-pCen.y,p3.z-pCen.z };
    //進行歐拉轉換至2維平面，Pe為二維點
    DPoint Pse, Pme, Pm2e;
    Pse = EulerRotation(Ps, D, true);
    Pme = EulerRotation(Pm, D, true);
    Pm2e = EulerRotation(Pm2, D, true);

    if(backDistance!=0)
    {
        //設置距離，更改第一點
        //計算圓形正逆轉
        DOUBLE psRad, pmRad, pm2Rad;
        psRad = fmod(atan2(Pse.y, Pse.x)+M_PI*2, M_PI*2);
        pmRad = fmod(atan2(Pme.y, Pme.x)+M_PI*2, M_PI*2);
        pm2Rad = fmod(atan2(Pm2e.y, Pm2e.x)+M_PI*2, M_PI*2);
        DOUBLE Radms, Radm2s;
        Radms = fmod(pmRad-psRad+M_PI*2, M_PI*2);
        Radm2s = fmod(pm2Rad-psRad+M_PI*2, M_PI*2);
        BOOL bRev;
        bRev = (Radms<Radm2s) ? true : false;
        DOUBLE radian = (DOUBLE)backDistance/r;
        Pse = rotationMatrix2_2(Pse, (bRev) ? -radian : radian);
    }
    //計算圓形正逆轉
    DOUBLE psRad, pmRad, pm2Rad;
    psRad = fmod(atan2(Pse.y, Pse.x)+M_PI*2, M_PI*2);
    pmRad = fmod(atan2(Pme.y, Pme.x)+M_PI*2, M_PI*2);
    pm2Rad = fmod(atan2(Pm2e.y, Pm2e.x)+M_PI*2, M_PI*2);
    DOUBLE Radms, Radm2s;
    Radms = fmod(pmRad-psRad+M_PI*2, M_PI*2);
    Radm2s = fmod(pm2Rad-psRad+M_PI*2, M_PI*2);
    BOOL bRev;
    bRev = (Radms<Radm2s) ? true : false;
    //於平面做旋轉取值
    std::vector<DPoint> cutPtArr;
    cutPtArr.clear();
    //PlaneRotationGetArray(Pse, Pm2e, r, bRev, pm2Rad-psRad, cutPtArr);
    PlaneRotationGetArray(Pse, Pm2e, r, bRev, (bRev) ? Radm2s : M_PI*2-Radm2s, cutPtArr);
    //空間旋轉轉換取值
    SpaceRotationGetArray(D, cutPtArr);
    //Vector轉換，並且從原點平移回去
    cutPointsArr.clear();
    for(size_t i = 0; i<cutPtArr.size(); i++)
    {
        AxeSpace P = { 0 };
        P.x = (LONG)round(cutPtArr.at(i).x+pCen.x);
        P.y = (LONG)round(cutPtArr.at(i).y+pCen.y);
        P.z = (LONG)round(cutPtArr.at(i).z+pCen.z);
        P.w = 0;
        if(i==0)
        {
            cutPointsArr.push_back(P);
        }
        else
        {
            if(P.x==cutPointsArr.back().x&&P.y==cutPointsArr.back().y&&P.z==cutPointsArr.back().z&&P.w==cutPointsArr.back().w)
            {
                //篩選去掉重複的筆數
            }
            else
            {
                cutPointsArr.push_back(P);
            }
        }
    }
}
void CCircleFormula::CircleCutPoint(AxeSpace sp, AxeSpace mp1, AxeSpace mp2, std::vector<AxeSpace> &cutPointsArr, LONG backDistance)
{
    DPoint p1, p2, p3;
    p1.x = sp.x;
    p1.y = sp.y;
    p1.z = sp.z;
    p2.x = mp1.x;
    p2.y = mp1.y;
    p2.z = mp1.z;
    p3.x = mp2.x;
    p3.y = mp2.y;
    p3.z = mp2.z;

    //求法向量
    DOUBLE a, b, c, d;
    FindNormalVector(p1, p2, p3, a, b, c, d);
    //求轉換矩陣(正交矩陣)
    DOUBLE D[9] = { 0 };
    RotateTransformMatrix(a, b, c, d, D);
    //求三維圓心
    DPoint pCen;
    pCen = SpaceCircleCenterCalculation(p1, p2, p3);
    DOUBLE r = sqrt(pow(p1.x-pCen.x, 2)+pow(p1.y-pCen.y, 2)+pow(p1.z-pCen.z, 2));
    //圓心平移至原點
    DPoint Ps,Pm,Pm2;
    Ps.x = p1.x-pCen.x;
    Ps.y = p1.y-pCen.y;
    Ps.z = p1.z-pCen.z;
    Pm.x = p2.x-pCen.x;
    Pm = { p2.x-pCen.x,p2.y-pCen.y,p2.z-pCen.z };
    Pm2 = { p3.x-pCen.x,p3.y-pCen.y,p3.z-pCen.z };
    //進行歐拉轉換至2維平面，Pe為二維點
    DPoint Pse, Pme, Pm2e;
    Pse = EulerRotation(Ps, D, true);
    Pme = EulerRotation(Pm, D, true);
    Pm2e = EulerRotation(Pm2, D, true);
    //計算圓形正逆轉
    DOUBLE psRad, pmRad, pm2Rad;
    psRad = fmod(atan2(Pse.y, Pse.x)+M_PI*2, M_PI*2);
    pmRad = fmod(atan2(Pme.y, Pme.x)+M_PI*2, M_PI*2);
    pm2Rad = fmod(atan2(Pm2e.y, Pm2e.x)+M_PI*2, M_PI*2);
    DOUBLE Radms, Radm2s;
    Radms = fmod(pmRad-psRad+M_PI*2, M_PI*2);
    Radm2s = fmod(pm2Rad-psRad+M_PI*2, M_PI*2);
    BOOL bRev;
    bRev = (Radms<Radm2s) ? true : false;
    //於平面做旋轉取值
    std::vector<DPoint> cutPtArr;
    cutPtArr.clear();
    PlaneRotationGetArray(Pse, r, bRev, cutPtArr, backDistance);
    //空間旋轉轉換取值
    SpaceRotationGetArray(D, cutPtArr);
    //Vector轉換，並且從原點平移回去
    cutPointsArr.clear();
    for(size_t i = 0; i<cutPtArr.size(); i++)
    {
        AxeSpace P = { 0 };
        P.x = (LONG)round(cutPtArr.at(i).x+pCen.x);
        P.y = (LONG)round(cutPtArr.at(i).y+pCen.y);
        P.z = (LONG)round(cutPtArr.at(i).z+pCen.z);
        P.w = 0;
        if(i==0)
        {
            cutPointsArr.push_back(P);
        }
        else
        {
            if(P.x==cutPointsArr.back().x&&P.y==cutPointsArr.back().y&&P.z==cutPointsArr.back().z&&P.w==cutPointsArr.back().w)
            {
                //篩選去掉重複的筆數
            }
            else
            {
                cutPointsArr.push_back(P);
            }
        }
    }
}
/*
*type       true:圓/false:圓弧
*cutUnit    切線長度，單位micrometer[um]
*/
void CCircleFormula::CircleCutPath_2D_unit(AxeSpace P1, AxeSpace P2, AxeSpace P3, BOOL type, LONG cutUnit, std::vector<AxeSpace> &cutPathArray)
{
    CCircleFormula cirForm;
    DPoint dp1, dp2, dp3, dcenter;
    dp1.x = (DOUBLE)P1.x;
    dp1.y = (DOUBLE)P1.y;
    dp1.z = (DOUBLE)P1.z;
    dp2.x = (DOUBLE)P2.x;
    dp2.y = (DOUBLE)P2.y;
    dp2.z = (DOUBLE)P2.z;
    dp3.x = (DOUBLE)P3.x;
    dp3.y = (DOUBLE)P3.y;
    dp3.z = (DOUBLE)P3.z;
    dcenter = cirForm.SpaceCircleCenterCalculation(dp1, dp2, dp3);

    DOUBLE r = sqrt(pow(dp1.x-dcenter.x, 2)+pow(dp1.y-dcenter.y, 2)+pow(dp1.z-dcenter.z, 2));
    //圓心位移至原點
    DPoint dpt1, dpt2, dpt3;
    dpt1 = { dp1.x-dcenter.x,dp1.y-dcenter.y,dp1.z-dcenter.z,0.0 };
    dpt2 = { dp2.x-dcenter.x,dp2.y-dcenter.y,dp2.z-dcenter.z,0.0 };
    dpt3 = { dp3.x-dcenter.x,dp3.y-dcenter.y,dp3.z-dcenter.z,0.0 };
    //計算圓形正逆轉
    DOUBLE p1Rad, p2Rad, p3Rad;
    p1Rad = fmod(atan2(dpt1.y, dpt1.x)+M_PI*2, M_PI*2);
    p2Rad = fmod(atan2(dpt2.y, dpt2.x)+M_PI*2, M_PI*2);
    p3Rad = fmod(atan2(dpt3.y, dpt3.x)+M_PI*2, M_PI*2);

    DOUBLE Radms, Radm2s;
    Radms = fmod(p2Rad-p1Rad+M_PI*2, M_PI*2);
    Radm2s = fmod(p3Rad-p1Rad+M_PI*2, M_PI*2);
    BOOL bRev;
    bRev = (Radms<Radm2s) ? true : false;
    cutPathArray.clear();

    DPoint Pee;
    AxeSpace Pn;
    DOUBLE rad = 0.0;
    //以1mm，也就是1000um的弧長做切點
    DOUBLE deltaRad = cutUnit/r;
    DOUBLE dAng = 0.0;
    dAng = (type) ? M_PI*2 : ((bRev) ? Radm2s : M_PI*2-Radm2s);
    if(bRev)
    {
        while(rad<dAng)
        {
            Pee.x = dpt1.x*cos(rad)-dpt1.y*sin(rad);
            Pee.y = dpt1.x*sin(rad)+dpt1.y*cos(rad);
            Pee.z = 0.0;

            Pn.x = (LONG)round(Pee.x+dcenter.x);
            Pn.y = (LONG)round(Pee.y+dcenter.y);
            Pn.z = P1.z;
            cutPathArray.push_back(Pn);
            rad += deltaRad;
        }
    }
    else
    {
        while(rad>-dAng)
        {
            Pee.x = dpt1.x*cos(rad)-dpt1.y*sin(rad);
            Pee.y = dpt1.x*sin(rad)+dpt1.y*cos(rad);
            Pee.z = 0.0;

            Pn.x = (LONG)round(Pee.x+dcenter.x);
            Pn.y = (LONG)round(Pee.y+dcenter.y);
            Pn.z = P1.z;
            cutPathArray.push_back(Pn);
            rad -= deltaRad;
        }
    }

    if(Pn.x!=P1.x&&Pn.y!=P1.y)
    {
        cutPathArray.push_back((type) ? P1 : P3);
    }
}
void CCircleFormula::CircleCutPath_2D_radian(AxeSpace P1, AxeSpace P2, AxeSpace P3, BOOL type, DOUBLE cutRad, std::vector<AxeSpace> &cutPathArray)
{
    CCircleFormula cirForm;
    DPoint dp1, dp2, dp3, dcenter;
    dp1.x = P1.x;
    dp1.y = P1.y;
    dp1.z = P1.z;
    dp2.x = P2.x;
    dp2.y = P2.y;
    dp2.z = P2.z;
    dp3.x = P3.x;
    dp3.y = P3.y;
    dp3.z = P3.z;
    dcenter = cirForm.SpaceCircleCenterCalculation(dp1, dp2, dp3);

    DOUBLE r = sqrt(pow(dp1.x-dcenter.x, 2)+pow(dp1.y-dcenter.y, 2)+pow(dp1.z-dcenter.z, 2));
    //圓心位移至原點
    DPoint dpt1, dpt2, dpt3;
    dpt1 = { dp1.x-dcenter.x,dp1.y-dcenter.y,dp1.z-dcenter.z,0.0 };
    dpt2 = { dp2.x-dcenter.x,dp2.y-dcenter.y,dp2.z-dcenter.z,0.0 };
    dpt3 = { dp3.x-dcenter.x,dp3.y-dcenter.y,dp3.z-dcenter.z,0.0 };
    //計算圓形正逆轉
    DOUBLE p1Rad, p2Rad, p3Rad;
    p1Rad = fmod(atan2(dpt1.y, dpt1.x)+M_PI*2, M_PI*2);
    p2Rad = fmod(atan2(dpt2.y, dpt2.x)+M_PI*2, M_PI*2);
    p3Rad = fmod(atan2(dpt3.y, dpt3.x)+M_PI*2, M_PI*2);

    DOUBLE Radms, Radm2s;
    Radms = fmod(p2Rad-p1Rad+M_PI*2, M_PI*2);
    Radm2s = fmod(p3Rad-p1Rad+M_PI*2, M_PI*2);
    BOOL bRev;
    bRev = (Radms<Radm2s) ? true : false;
    cutPathArray.clear();

    DPoint Pee;
    AxeSpace Pn;
    DOUBLE rad = 0.0;
    //以1mm，也就是1000um的弧長做切點
    DOUBLE deltaRad = cutRad;
    DOUBLE dAng = 0.0;
    dAng = (type) ? M_PI*2 : ((bRev) ? Radm2s : M_PI*2-Radm2s);
    if(bRev)
    {
        while(rad<dAng)
        {
            Pee.x = dpt1.x*cos(rad)-dpt1.y*sin(rad);
            Pee.y = dpt1.x*sin(rad)+dpt1.y*cos(rad);
            Pee.z = 0.0;

            Pn.x = (LONG)round(Pee.x+dcenter.x);
            Pn.y = (LONG)round(Pee.y+dcenter.y);
            Pn.z = P1.z;
            cutPathArray.push_back(Pn);
            rad += deltaRad;
        }
    }
    else
    {
        while(rad>-dAng)
        {
            Pee.x = dpt1.x*cos(rad)-dpt1.y*sin(rad);
            Pee.y = dpt1.x*sin(rad)+dpt1.y*cos(rad);
            Pee.z = 0.0;

            Pn.x = (LONG)round(Pee.x+dcenter.x);
            Pn.y = (LONG)round(Pee.y+dcenter.y);
            Pn.z = P1.z;
            cutPathArray.push_back(Pn);
            rad -= deltaRad;
        }
    }

    if(Pn.x!=P1.x&&Pn.y!=P1.y)
    {
        cutPathArray.push_back((type) ? P1 : P3);
    }
}
DPoint CCircleFormula::rotationMatrix2_2(DPoint P, DOUBLE rad)
{
    DPoint Pn;
    Pn.x = P.x*cos(rad)-P.y*sin(rad);
    Pn.y = P.x*sin(rad)+P.y*cos(rad);
    return Pn;
}

void CCircleFormula::rotationMatrix2_2(DOUBLE *point, DOUBLE rad)
{
    DOUBLE pX, pY;
    pX = point[0];
    pY = point[1];
    point[0] = pX*cos(rad)-pY*sin(rad);
    point[1] = pX*sin(rad)+pY*cos(rad);
}

void CCircleFormula::FindNormalVector(DPoint p1, DPoint p2, DPoint p3, DOUBLE &a, DOUBLE &b, DOUBLE &c, DOUBLE &d)
{
    a = (p2.y-p1.y)*(p3.z-p1.z)-(p3.y-p1.y)*(p2.z-p1.z);
    b = (p2.z-p1.z)*(p3.x-p1.x)-(p2.x-p1.x)*(p3.z-p1.z);
    c = (p2.x-p1.x)*(p3.y-p1.y)-(p3.x-p1.x)*(p2.y-p1.y);
    d = -(a*p1.x+b*p1.y+c*p1.z);
    //ax+by+cz-d = 0
    //ax+by+cz = d
}

void CCircleFormula::FindNormalVector(DOUBLE *d1, DOUBLE *d2, DOUBLE *d3, DOUBLE *normalVector)
{
    //ax+by+cz-d = 0
    //ax+by+cz = d
    normalVector[0] = (d2[1]-d1[1])*(d3[2]-d1[2])-(d3[1]-d1[1])*(d2[2]-d1[2]);
    normalVector[1] = (d2[2]-d1[2])*(d3[0]-d1[0])-(d2[0]-d1[0])*(d3[2]-d1[2]);
    normalVector[2] = (d2[0]-d1[0])*(d3[1]-d1[1])-(d3[0]-d1[0])*(d2[1]-d1[1]);
    normalVector[3] = -(normalVector[0]*d1[0]+normalVector[1]*d1[1]+normalVector[2]*d1[2]);
}

void CCircleFormula::RotateTransformMatrix(DOUBLE a, DOUBLE b, DOUBLE c, DOUBLE d, DOUBLE *matrix3_3)
{
    DOUBLE cosTheta, sinTheta, cosDelta, sinDelta;
    sinTheta = a/sqrt(a*a+b*b);
    cosTheta = -b/sqrt(a*a+b*b);
    sinDelta = sqrt(a*a+b*b)/sqrt(a*a+b*b+c*c);
    cosDelta = c/sqrt(a*a+b*b+c*c);
    matrix3_3[0] = cosTheta;
    matrix3_3[1] = -sinTheta*cosDelta;
    matrix3_3[2] = sinTheta*sinDelta;
    matrix3_3[3] = sinTheta;
    matrix3_3[4] = cosTheta*cosDelta;
    matrix3_3[5] = -cosTheta*sinDelta;
    matrix3_3[6] = 0;
    matrix3_3[7] = sinDelta;
    matrix3_3[8] = cosDelta;
}

DPoint CCircleFormula::EulerRotation(DPoint startPoint, DOUBLE *matrix3_3, BOOL mat_inverse)
{
    DPoint Pe;
    if(mat_inverse)
    {
        Pe.x = matrix3_3[0]*startPoint.x+matrix3_3[3]*startPoint.y+matrix3_3[6]*startPoint.z;
        Pe.y = matrix3_3[1]*startPoint.x+matrix3_3[4]*startPoint.y+matrix3_3[7]*startPoint.z;
        Pe.z = matrix3_3[2]*startPoint.x+matrix3_3[5]*startPoint.y+matrix3_3[8]*startPoint.z;
    }
    else
    {
        Pe.x = matrix3_3[0]*startPoint.x+matrix3_3[1]*startPoint.y+matrix3_3[2]*startPoint.z;
        Pe.y = matrix3_3[3]*startPoint.x+matrix3_3[4]*startPoint.y+matrix3_3[5]*startPoint.z;
        Pe.z = matrix3_3[6]*startPoint.x+matrix3_3[7]*startPoint.y+matrix3_3[8]*startPoint.z;
    }
    return Pe;
}

void CCircleFormula::PlaneRotationGetArray(DPoint Pe, DOUBLE r, BOOL Rev, std::vector<DPoint> &cutPtArr, LONG backDistance)
{
    DPoint Pee;
    DOUBLE rad = 0.0;
    if(backDistance!=0)
    {
        rad = (Rev) ? -(DOUBLE)backDistance/r : (DOUBLE)backDistance/r;
    }
    //以1mm，也就是1000um的弧長做切點
    DOUBLE deltaRad = (DOUBLE)m_cuttingLength/r;
    if(Rev)
    {
        while(rad<M_PI*2)
        {
            Pee.x = Pe.x*cos(rad)-Pe.y*sin(rad);
            Pee.y = Pe.x*sin(rad)+Pe.y*cos(rad);
            Pee.z = 0.0;
            cutPtArr.push_back(Pee);
            rad += deltaRad;
        }
    }
    else
    {
        while(rad>-M_PI*2)
        {
            Pee.x = Pe.x*cos(rad)-Pe.y*sin(rad);
            Pee.y = Pe.x*sin(rad)+Pe.y*cos(rad);
            Pee.z = 0.0;
            cutPtArr.push_back(Pee);
            rad -= deltaRad;
        }
    }
    cutPtArr.push_back(Pe);
}

void CCircleFormula::PlaneRotationGetArray(DPoint Pe, DPoint Pm2e, DOUBLE r, BOOL Rev, DOUBLE endRad, std::vector<DPoint> &cutPtArr)
{
    DPoint Pee;
    DOUBLE rad = 0.0;
    //以1mm，也就是1000um的弧長做切點
    DOUBLE deltaRad = (DOUBLE)m_cuttingLength/r;
    if(Rev)
    {
        while(rad<endRad)
        {
            Pee.x = Pe.x*cos(rad)-Pe.y*sin(rad);
            Pee.y = Pe.x*sin(rad)+Pe.y*cos(rad);
            Pee.z = 0.0;
            cutPtArr.push_back(Pee);
            rad += deltaRad;
        }
    }
    else
    {
        while(rad>-endRad)
        {
            Pee.x = Pe.x*cos(rad)-Pe.y*sin(rad);
            Pee.y = Pe.x*sin(rad)+Pe.y*cos(rad);
            Pee.z = 0.0;
            cutPtArr.push_back(Pee);
            rad -= deltaRad;
        }
    }
    cutPtArr.push_back(Pm2e);
}

void CCircleFormula::SpaceRotationGetArray(DOUBLE *D, std::vector<DPoint> &cutPtArr)
{
    std::vector<DPoint>::iterator pDP;
    for(pDP = cutPtArr.begin(); pDP!=cutPtArr.end(); pDP++)
    {
        *pDP = EulerRotation(*pDP, D, false);
    }
}

void CCircleFormula::MatrixInverse(DPoint p1, DPoint p2, DPoint p3, DOUBLE *matrix3_3)
{
    DOUBLE detM = 0.0;
    detM = p1.x*p2.y*p3.z+p2.x*p3.y*p1.z+p3.x*p1.y*p2.z-p1.z*p2.y*p3.x-p1.y*p2.x*p3.z-p1.x*p2.z*p3.y;
    matrix3_3[0] = 1.0/detM*(p2.y*p3.z-p2.z*p3.y);
    matrix3_3[1] = -1.0/detM*(p1.y*p3.z-p1.z*p3.y);
    matrix3_3[2] = 1.0/detM*(p1.y*p2.z-p1.z*p2.y);
    matrix3_3[3] = -1.0/detM*(p2.x*p3.z-p2.z*p3.x);
    matrix3_3[4] = 1.0/detM*(p1.x*p3.z-p1.z*p3.x);
    matrix3_3[5] = -1.0/detM*(p1.x*p2.z-p1.z*p2.x);
    matrix3_3[6] = 1.0/detM*(p2.x*p3.y-p2.y*p3.x);
    matrix3_3[7] = -1.0/detM*(p1.x*p3.y-p1.y*p3.x);
    matrix3_3[8] = 1.0/detM*(p1.x*p2.y-p1.y*p2.x);
}

DPoint CCircleFormula::SpaceCircleCenterCalculation(DPoint p1, DPoint p2, DPoint p3)
{
    DOUBLE a, b, c, d;
    a = (p2.y-p1.y)*(p3.z-p1.z)-(p3.y-p1.y)*(p2.z-p1.z);
    b = (p2.z-p1.z)*(p3.x-p1.x)-(p2.x-p1.x)*(p3.z-p1.z);
    c = (p2.x-p1.x)*(p3.y-p1.y)-(p3.x-p1.x)*(p2.y-p1.y);
    d = -(a*p1.x+b*p1.y+c*p1.z);

    DOUBLE a2, a3, b2, b3, c2, c3, d2, d3;
    a2 = 2.0*(p2.x-p1.x);
    b2 = 2.0*(p2.y-p1.y);
    c2 = 2.0*(p2.z-p1.z);
    d2 = p1.x*p1.x+p1.y*p1.y+p1.z*p1.z-p2.x*p2.x-p2.y*p2.y-p2.z*p2.z;

    a3 = 2.0*(p3.x-p1.x);
    b3 = 2.0*(p3.y-p1.y);
    c3 = 2.0*(p3.z-p1.z);
    d3 = p1.x*p1.x+p1.y*p1.y+p1.z*p1.z-p3.x*p3.x-p3.y*p3.y-p3.z*p3.z;

    DPoint Pi, Pj, Pk, Pl;
    Pi.x = a;
    Pi.y = b;
    Pi.z = c;
    Pj.x = a2;
    Pj.y = b2;
    Pj.z = c2;
    Pk.x = a3;
    Pk.y = b3;
    Pk.z = c3;
    Pl.x = -d;
    Pl.y = -d2;
    Pl.z = -d3;
    //[P] = -inv[MAT][Pl];
    DOUBLE MAT[9] = { 0 };
    //求逆矩陣
    MatrixInverse(Pi, Pj, Pk, MAT);
    //P = MAT‧Pl //C已經是inverse MAT
    return EulerRotation(Pl, MAT, false);
}

DPoint CCircleFormula::SpaceCircleCenterCalculation(DPoint p1, DPoint p2, DPoint p3, DOUBLE &radius, DOUBLE &angle)
{
    //求法向量
    DOUBLE a, b, c, d;
    FindNormalVector(p1, p2, p3, a, b, c, d);
    //求轉換矩陣(正交矩陣)
    DOUBLE D[9] = { 0 };
    RotateTransformMatrix(a, b, c, d, D);
    //求三維圓心
    DPoint pCen;
    pCen = SpaceCircleCenterCalculation(p1, p2, p3);
    DOUBLE r = sqrt(pow(p1.x-pCen.x, 2)+pow(p1.y-pCen.y, 2)+pow(p1.z-pCen.z, 2));
    //圓心平移至原點
    DPoint Ps, Pm, Pm2;
    Ps.x = p1.x-pCen.x;
    Ps.y = p1.y-pCen.y;
    Ps.z = p1.z-pCen.z;
    Pm.x = p2.x-pCen.x;
    Pm = { p2.x-pCen.x,p2.y-pCen.y,p2.z-pCen.z };
    Pm2 = { p3.x-pCen.x,p3.y-pCen.y,p3.z-pCen.z };
    //進行歐拉轉換至2維平面，Pe為二維點
    DPoint Pse, Pme, Pm2e;
    Pse = EulerRotation(Ps, D, true);
    Pme = EulerRotation(Pm, D, true);
    Pm2e = EulerRotation(Pm2, D, true);


    //計算圓形正逆轉
    DOUBLE psRad, pmRad, pm2Rad;
    psRad = fmod(atan2(Pse.y, Pse.x)+M_PI*2, M_PI*2);
    pmRad = fmod(atan2(Pme.y, Pme.x)+M_PI*2, M_PI*2);
    pm2Rad = fmod(atan2(Pm2e.y, Pm2e.x)+M_PI*2, M_PI*2);
    DOUBLE Radms, Radm2s;
    Radms = fmod(pmRad-psRad+M_PI*2, M_PI*2);
    Radm2s = fmod(pm2Rad-psRad+M_PI*2, M_PI*2);
    BOOL bRev;
    bRev = (Radms<Radm2s) ? true : false;

    DOUBLE radian = (bRev) ? Radm2s : M_PI*2-Radm2s;
    radius = r;
    angle = radian*180.0/M_PI;
    return pCen;
}

DPoint CCircleFormula::ArcCentCalculation_2D(DPoint p1, DPoint p2, DPoint p3, DOUBLE &radius, DOUBLE &angle)
{
    //求三維圓心
    DPoint pCen;
    pCen = SpaceCircleCenterCalculation(p1, p2, p3);
    radius = sqrt(pow(pCen.x-p1.x, 2)+pow(pCen.y-p1.y, 2));

    //圓心位移至原點
    DPoint dpt1, dpt2, dpt3;
    dpt1 = { p1.x-pCen.x,p1.y-pCen.y,p1.z-pCen.z,0.0 };
    dpt2 = { p2.x-pCen.x,p2.y-pCen.y,p2.z-pCen.z,0.0 };
    dpt3 = { p3.x-pCen.x,p3.y-pCen.y,p3.z-pCen.z,0.0 };
    //計算圓形正逆轉
    DOUBLE p1Rad, p2Rad, p3Rad;
    p1Rad = fmod(atan2(dpt1.y, dpt1.x)+M_PI*2, M_PI*2);
    p2Rad = fmod(atan2(dpt2.y, dpt2.x)+M_PI*2, M_PI*2);
    p3Rad = fmod(atan2(dpt3.y, dpt3.x)+M_PI*2, M_PI*2);

    DOUBLE Radms, Radm2s;
    Radms = fmod(p2Rad-p1Rad+M_PI*2, M_PI*2);
    Radm2s = fmod(p3Rad-p1Rad+M_PI*2, M_PI*2);
    BOOL bRev;
    bRev = (Radms<Radm2s) ? true : false;

    angle = ((bRev) ? Radm2s : M_PI*2-Radm2s)*180.0/M_PI;
    return pCen;
}

DPoint CCircleFormula::ArcCentCalculation_2D(DPoint p1, DPoint p2, DPoint p3, DOUBLE &radius, DOUBLE &angle, BOOL &rev)
{
    //求三維圓心
    DPoint pCen;
    pCen = SpaceCircleCenterCalculation(p1, p2, p3);
    radius = sqrt(pow(pCen.x-p1.x, 2)+pow(pCen.y-p1.y, 2));

    //圓心位移至原點
    DPoint dpt1, dpt2, dpt3;
    dpt1 = { p1.x-pCen.x,p1.y-pCen.y,p1.z-pCen.z,0.0 };
    dpt2 = { p2.x-pCen.x,p2.y-pCen.y,p2.z-pCen.z,0.0 };
    dpt3 = { p3.x-pCen.x,p3.y-pCen.y,p3.z-pCen.z,0.0 };
    //計算圓形正逆轉
    DOUBLE p1Rad, p2Rad, p3Rad;
    p1Rad = fmod(atan2(dpt1.y, dpt1.x)+M_PI*2, M_PI*2);
    p2Rad = fmod(atan2(dpt2.y, dpt2.x)+M_PI*2, M_PI*2);
    p3Rad = fmod(atan2(dpt3.y, dpt3.x)+M_PI*2, M_PI*2);

    DOUBLE Radms, Radm2s;
    Radms = fmod(p2Rad-p1Rad+M_PI*2, M_PI*2);
    Radm2s = fmod(p3Rad-p1Rad+M_PI*2, M_PI*2);
    BOOL bRev;
    bRev = (Radms<Radm2s) ? true : false;
    rev = bRev;
    angle = ((bRev) ? Radm2s : M_PI*2-Radm2s)*180.0/M_PI;
    return pCen;
}
BOOL CCircleFormula::SpaceRectanglePointCalculation(DPoint p1, DPoint p2, DPoint pA, RectangleSpace &Rectangle,LONG Type)
{
    DPoint pCen = { 0,0,0,0 };
    DOUBLE radius = 0;
    DOUBLE angle = 0;
    pCen = ArcCentCalculation_2D(p1, pA, p2, radius, angle);
    if(Type)
    {
        if((int)round(angle) < 180)
        {
            SpaceRectanglePointCalculationInside180(p1, p2, pA, pCen, radius, Rectangle);
        }
        else if((int)round(angle) == 180)
        {
            SpaceRectanglePointCalculation180(p1, p2, pA, pCen, radius, Rectangle);
        }
        else if((int)round(angle) > 180)
        {
            SpaceRectanglePointCalculationOutside180(p1, p2, pA, pCen, radius, Rectangle);
        }
    }
    else
    {
        SpaceRectanglePointCalculation360(p1, p2, pA, pCen, radius, Rectangle);
    }
    return 0;
}

BOOL CCircleFormula::SpaceRectanglePointCalculationInside180(DPoint p1, DPoint p2, DPoint pA, DPoint pCen, DOUBLE radius, RectangleSpace &Rectangle)
{
    DPoint MPoint,TPoint,p3,p4;
    MPoint.x = (p1.x + p2.x) / 2;
    MPoint.y = (p1.y + p2.y) / 2;
    TPoint.x = pCen.x + (((MPoint.x - pCen.x)*radius) / sqrt(pow(MPoint.x - pCen.x, 2) + pow(MPoint.y - pCen.y, 2)));
    TPoint.y = pCen.y + (((MPoint.y - pCen.y)*radius) / sqrt(pow(MPoint.x - pCen.x, 2) + pow(MPoint.y - pCen.y, 2)));
    p3.x = TPoint.x + p2.x - MPoint.x;
    p3.y = TPoint.y + p2.y - MPoint.y;
    p4.x = TPoint.x + p1.x - MPoint.x;
    p4.y = TPoint.y + p1.y - MPoint.y;
    Rectangle.P1 = { static_cast<LONG>(p1.x + 0.5),static_cast<LONG>(p1.y + 0.5),0,0 };
    Rectangle.P2 = { static_cast<LONG>(p2.x + 0.5),static_cast<LONG>(p2.y + 0.5),0,0 };
    Rectangle.P3 = { static_cast<LONG>(p3.x + 0.5),static_cast<LONG>(p3.y + 0.5),0,0 };
    Rectangle.P4 = { static_cast<LONG>(p4.x + 0.5),static_cast<LONG>(p4.y + 0.5),0,0 };
    return 0;
}

BOOL CCircleFormula::SpaceRectanglePointCalculationOutside180(DPoint p1, DPoint p2, DPoint pA, DPoint pCen, DOUBLE radius, RectangleSpace &Rectangle)
{
    DPoint MPoint, TPoint, P1P2U;
    MPoint.x = (p1.x + p2.x) / 2;
    MPoint.y = (p1.y + p2.y) / 2;
    TPoint.x = pCen.x + (((pCen.x - MPoint.x)*radius) / sqrt(pow(pCen.x - MPoint.x, 2) + pow(pCen.y - MPoint.y, 2)));
    TPoint.y = pCen.y + (((pCen.y - MPoint.y)*radius) / sqrt(pow(pCen.x - MPoint.x, 2) + pow(pCen.y - MPoint.y, 2)));
    P1P2U.x = (p2.x - p1.x) / sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
    P1P2U.y = (p2.y - p1.y) / sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
    Rectangle.P1 = { static_cast<LONG>(MPoint.x - radius*P1P2U.x + 0.5),static_cast<LONG>(MPoint.y - radius*P1P2U.y + 0.5),0,0 };
    Rectangle.P2 = { static_cast<LONG>(MPoint.x + radius*P1P2U.x + 0.5),static_cast<LONG>(MPoint.y + radius*P1P2U.y + 0.5),0,0 };
    Rectangle.P1 = { static_cast<LONG>(MPoint.x - radius*P1P2U.x),static_cast<LONG>(MPoint.y - radius*P1P2U.y),0,0 };
    Rectangle.P3 = { static_cast<LONG>(TPoint.x - radius*P1P2U.x + 0.5),static_cast<LONG>(TPoint.y - radius*P1P2U.y + 0.5),0,0 };
    Rectangle.P4 = { static_cast<LONG>(TPoint.x + radius*P1P2U.x + 0.5),static_cast<LONG>(TPoint.y + radius*P1P2U.y + 0.5),0,0 };
    return 0;
}

BOOL CCircleFormula::SpaceRectanglePointCalculation180(DPoint p1, DPoint p2, DPoint pA, DPoint pCen, DOUBLE radius, RectangleSpace &Rectangle)
{
    DPoint MPoint1, MPoint2;
    DOUBLE D1, D2;
    MPoint1.x = pCen.x - p2.y + pCen.y;
    MPoint1.y = pCen.y + p2.x - pCen.x;
    MPoint2.x = pCen.x + p2.y - pCen.y;
    MPoint2.y = pCen.y - p2.x + pCen.x;
    D1 = sqrt(pow(MPoint1.x - pA.x, 2) + pow(MPoint1.y - pA.y, 2));
    D2 = sqrt(pow(MPoint2.x - pA.x, 2) + pow(MPoint2.y - pA.y, 2));
    Rectangle.P1 = { static_cast<LONG>(p1.x + 0.5),static_cast<LONG>(p1.y + 0.5),0,0 };
    Rectangle.P2 = { static_cast<LONG>(p2.x + 0.5),static_cast<LONG>(p2.y + 0.5),0,0 };
    if(D1 < D2)
    {
        Rectangle.P3 = { static_cast<LONG>(MPoint1.x + p2.x - pCen.x + 0.5),static_cast<LONG>(MPoint1.y + p2.y - pCen.y + 0.5),0,0 };
        Rectangle.P4 = { static_cast<LONG>(MPoint1.x - p2.x + pCen.x + 0.5),static_cast<LONG>(MPoint1.y - p2.y + pCen.y + 0.5),0,0 };
    }
    else if(D2 < D1)
    {
        Rectangle.P3 = { static_cast<LONG>(MPoint2.x + p2.x - pCen.x + 0.5),static_cast<LONG>(MPoint2.y + p2.y - pCen.y + 0.5),0,0 };
        Rectangle.P4 = { static_cast<LONG>(MPoint2.x - p2.x + pCen.x + 0.5),static_cast<LONG>(MPoint2.y - p2.y + pCen.y + 0.5),0,0 };
    }
    else
    {
#ifdef MFC
        AfxMessageBox(L"嚴重計算錯誤!");
#endif
    }
    return 0;
}

BOOL CCircleFormula::SpaceRectanglePointCalculation360(DPoint p1, DPoint p2, DPoint pA, DPoint pCen, DOUBLE radius, RectangleSpace &Rectangle)
{
    Rectangle.P1 = { static_cast<LONG>(pCen.x - radius + 0.5),static_cast<LONG>(pCen.y - radius + 0.5),0,0 };
    Rectangle.P2 = { static_cast<LONG>(pCen.x + radius + 0.5),static_cast<LONG>(pCen.y - radius + 0.5),0,0 };
    Rectangle.P3 = { static_cast<LONG>(pCen.x + radius + 0.5),static_cast<LONG>(pCen.y + radius + 0.5),0,0 };
    Rectangle.P4 = { static_cast<LONG>(pCen.x - radius + 0.5),static_cast<LONG>(pCen.y + radius + 0.5),0,0 };
    return 0;
}

