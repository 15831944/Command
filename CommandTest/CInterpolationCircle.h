/*
*date:2017/02/14
*/

#pragma once
#include<vector>

struct DPoint
{
    double x;
    double y;
    double z;
    double w;
};
struct AxeSpace
{
    long x;
    long y;
    long z;
    long w;
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

        long m_cuttingLength = 15;//um

        void ArcCutPoint(AxeSpace sp, AxeSpace mp, AxeSpace ep,std::vector<AxeSpace> &cutPointsArr);
        void CircleCutPoint(AxeSpace sp, AxeSpace mp1, AxeSpace mp2, std::vector<AxeSpace> &cutPointsArr);
        //type_true:圓/false:圓弧
        void CircleCutPath_2D_unit(AxeSpace P1, AxeSpace P2, AxeSpace P3, bool type, long cutUnit, std::vector<AxeSpace> &cutPathArray);
        void CircleCutPath_2D_radian(AxeSpace P1, AxeSpace P2, AxeSpace P3, bool type, double cutRad, std::vector<AxeSpace> &cutPathArray);
    public:
        //2*2旋轉矩陣
        DPoint rotationMatrix2_2(DPoint P, double rad);
        void rotationMatrix2_2(double *point, double rad);
        //三點求法向量
        void FindNormalVector(DPoint p1, DPoint p2, DPoint p3, double &a, double &b, double &c, double &d);
        void FindNormalVector(double *d1, double *d2, double *d3, double *normalVector);
        //3*3座標系旋轉矩陣 matrix3_3:1D array
        void RotateTransformMatrix(double a, double b, double c, double d, double *matrix3_3);
        //尤拉旋轉變換
        DPoint EulerRotation(DPoint startPoint, double *matrix3_3, bool mat_inverse);
        //於一平面旋轉一角度求切點，得圓弧切點矩陣
        void PlaneRotationGetArray(DPoint Pe, double r, bool Rev, std::vector<DPoint> &cutPtArr);
        void PlaneRotationGetArray(DPoint Pe, DPoint Pm2e, double r, bool Rev, double endRad, std::vector<DPoint> &cutPtArr);
        //使用3*3座標系旋轉矩陣，將平面座標系的座標點轉換回三維空間座標點
        void SpaceRotationGetArray(double *D, std::vector<DPoint> &cutPtArr);
        //求3*3矩陣的逆矩陣
        void MatrixInverse(DPoint p1, DPoint p2, DPoint p3, double *matrix3_3);
        //空間三點求圓心座標
        DPoint SpaceCircleCenterCalculation(DPoint p1, DPoint p2, DPoint p3);
        //2D空間求圓心、半徑、角度
        DPoint ArcCentCalculation_2D(DPoint p1, DPoint p2, DPoint p3, double &radius, double &angle);
        //空間三點求矩形座標
        BOOL SpaceRectanglePointCalculation(DPoint p1, DPoint p2, DPoint pA, RectangleSpace& Rectangle, LONG Type);
        //空間三點求矩形座標180度內
        BOOL SpaceRectanglePointCalculationInside180(DPoint p1, DPoint p2, DPoint pA, DPoint pCen, double radius, RectangleSpace& Rectangle);
        //空間三點求矩形座標180度外
        BOOL SpaceRectanglePointCalculationOutside180(DPoint p1, DPoint p2, DPoint pA, DPoint pCen, double radius, RectangleSpace& Rectangle);
        //空間三點求矩形座標180度
        BOOL SpaceRectanglePointCalculation180(DPoint p1, DPoint p2, DPoint pA, DPoint pCen, double radius, RectangleSpace& Rectangle);
        //空間三點求矩形座標360度
        BOOL SpaceRectanglePointCalculation360(DPoint p1, DPoint p2, DPoint pA, DPoint pCen, double radius, RectangleSpace& Rectangle);
};
