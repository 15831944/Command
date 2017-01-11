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

class CCircleFormula
{
    public:
        CCircleFormula();
        ~CCircleFormula();

        void ArcCutPoint(AxeSpace sp, AxeSpace mp, AxeSpace ep,std::vector<AxeSpace> &cutPointsArr);
        void CircleCutPoint(AxeSpace sp, AxeSpace mp1, AxeSpace mp2, std::vector<AxeSpace> &cutPointsArr);
        void CircleCutPath_2D(AxeSpace P1, AxeSpace P2, AxeSpace P3, bool type, long cutUnit, std::vector<AxeSpace> &cutPathArray);
    public:
        DPoint rotationMatrix2_2(DPoint P, double rad);
        void FindNormalVector(DPoint p1, DPoint p2, DPoint p3, double &a, double &b, double &c, double &d);
        void RotateTransformMatrix(double a, double b, double c, double d, double *matrix3_3);
        DPoint EulerRotation(DPoint startPoint, double *matrix3_3, bool mat_inverse);
        void PlaneRotationGetArray(DPoint Pe, double r, bool Rev, std::vector<DPoint> &cutPtArr);
        void PlaneRotationGetArray(DPoint Pe,DPoint Pm2e, double r, bool Rev, double endRad, std::vector<DPoint> &cutPtArr);
        void SpaceRotationGetArray(double *D, std::vector<DPoint> &cutPtArr);
        void MatrixInverse(DPoint p1, DPoint p2, DPoint p3, double *matrix3_3);
        DPoint SpaceCircleCenterCalculation(DPoint p1, DPoint p2, DPoint p3);
};
