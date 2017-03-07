/*
*date:2017/02/20
*/

#pragma once
#include<vector>

struct DPoint
{
    DOUBLE x;
    DOUBLE y;
    DOUBLE z;
    DOUBLE w;
};
struct AxeSpace
{
    LONG x;
    LONG y;
    LONG z;
    DOUBLE w;
};
struct RectangleSpace
{
    AxeSpace P1;
    AxeSpace P2;
    AxeSpace P3;
    AxeSpace P4;
};

class CCircleFormula
{
    public:
        CCircleFormula();
        ~CCircleFormula();

        LONG m_cuttingLength = 15;//um
        //3D 圓弧切點
        void ArcCutPoint(AxeSpace sp, AxeSpace mp, AxeSpace ep, std::vector<AxeSpace> &cutPointsArr, LONG backDistance = 0);
        //3D 圓切點
        void CircleCutPoint(AxeSpace sp, AxeSpace mp1, AxeSpace mp2, std::vector<AxeSpace> &cutPointsArr, LONG backDistance = 0);
        //type_true:圓/false:圓弧
        void CircleCutPath_2D_unit(AxeSpace P1, AxeSpace P2, AxeSpace P3, BOOL type, LONG cutUnit, std::vector<AxeSpace> &cutPathArray);
        //type_true:圓/false:圓弧
        void CircleCutPath_2D_radian(AxeSpace P1, AxeSpace P2, AxeSpace P3, BOOL type, DOUBLE cutRad, std::vector<AxeSpace> &cutPathArray);
    public:
        //2*2旋轉矩陣
        DPoint rotationMatrix2_2(DPoint P, DOUBLE rad);
        void rotationMatrix2_2(DOUBLE *point, DOUBLE rad);
        //三點求法向量
        void FindNormalVector(DPoint p1, DPoint p2, DPoint p3, DOUBLE &a, DOUBLE &b, DOUBLE &c, DOUBLE &d);
        void FindNormalVector(DOUBLE *d1, DOUBLE *d2, DOUBLE *d3, DOUBLE *normalVector);
        //3*3座標系旋轉矩陣 matrix3_3:1D array
        void RotateTransformMatrix(DOUBLE a, DOUBLE b, DOUBLE c, DOUBLE d, DOUBLE *matrix3_3);
        //尤拉旋轉變換
        DPoint EulerRotation(DPoint startPoint, DOUBLE *matrix3_3, BOOL mat_inverse);
        //於一平面旋轉一角度求切點，得圓弧切點矩陣
        void PlaneRotationGetArray(DPoint Pe, DOUBLE r, BOOL Rev, std::vector<DPoint> &cutPtArr, LONG backDistance = 0);
        void PlaneRotationGetArray(DPoint Pe, DPoint Pm2e, DOUBLE r, BOOL Rev, DOUBLE endRad, std::vector<DPoint> &cutPtArr);
        //使用3*3座標系旋轉矩陣，將平面座標系的座標點轉換回三維空間座標點
        void SpaceRotationGetArray(DOUBLE *D, std::vector<DPoint> &cutPtArr);
        //求3*3矩陣的逆矩陣
        void MatrixInverse(DPoint p1, DPoint p2, DPoint p3, DOUBLE *matrix3_3);
        //空間三點求圓心座標
        DPoint SpaceCircleCenterCalculation(DPoint p1, DPoint p2, DPoint p3);
        //空間三點求圓心座標、半徑、角度
        DPoint SpaceCircleCenterCalculation(DPoint p1, DPoint p2, DPoint p3, DOUBLE &radius, DOUBLE &angle);
        //2D空間求圓心、半徑、角度
        DPoint ArcCentCalculation_2D(DPoint p1, DPoint p2, DPoint p3, DOUBLE &radius, DOUBLE &angle);
        //2D空間求圓心、半徑、角度、正逆時針
        DPoint ArcCentCalculation_2D(DPoint p1, DPoint p2, DPoint p3, DOUBLE &radius, DOUBLE &angle, BOOL &rev);
        //空間三點求矩形座標
        BOOL SpaceRectanglePointCalculation(DPoint p1, DPoint p2, DPoint pA, RectangleSpace &Rectangle, LONG Type);
        //空間三點求矩形座標180度內
        BOOL SpaceRectanglePointCalculationInside180(DPoint p1, DPoint p2, DPoint pA, DPoint pCen, DOUBLE radius, RectangleSpace &Rectangle);
        //空間三點求矩形座標180度外
        BOOL SpaceRectanglePointCalculationOutside180(DPoint p1, DPoint p2, DPoint pA, DPoint pCen, DOUBLE radius, RectangleSpace &Rectangle);
        //空間三點求矩形座標180度
        BOOL SpaceRectanglePointCalculation180(DPoint p1, DPoint p2, DPoint pA, DPoint pCen, DOUBLE radius, RectangleSpace &Rectangle);
        //空間三點求矩形座標360度
        BOOL SpaceRectanglePointCalculation360(DPoint p1, DPoint p2, DPoint pA, DPoint pCen, DOUBLE radius, RectangleSpace &Rectangle);

};
