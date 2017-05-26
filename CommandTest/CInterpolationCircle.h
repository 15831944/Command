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
        //type_true:��/false:�꩷
        void CircleCutPath_2D_unit(AxeSpace P1, AxeSpace P2, AxeSpace P3, bool type, long cutUnit, std::vector<AxeSpace> &cutPathArray);
        void CircleCutPath_2D_radian(AxeSpace P1, AxeSpace P2, AxeSpace P3, bool type, double cutRad, std::vector<AxeSpace> &cutPathArray);
    public:
        //2*2����x�}
        DPoint rotationMatrix2_2(DPoint P, double rad);
        void rotationMatrix2_2(double *point, double rad);
        //�T�I�D�k�V�q
        void FindNormalVector(DPoint p1, DPoint p2, DPoint p3, double &a, double &b, double &c, double &d);
        void FindNormalVector(double *d1, double *d2, double *d3, double *normalVector);
        //3*3�y�Шt����x�} matrix3_3:1D array
        void RotateTransformMatrix(double a, double b, double c, double d, double *matrix3_3);
        //�שԱ����ܴ�
        DPoint EulerRotation(DPoint startPoint, double *matrix3_3, bool mat_inverse);
        //��@��������@���רD���I�A�o�꩷���I�x�}
        void PlaneRotationGetArray(DPoint Pe, double r, bool Rev, std::vector<DPoint> &cutPtArr);
        void PlaneRotationGetArray(DPoint Pe, DPoint Pm2e, double r, bool Rev, double endRad, std::vector<DPoint> &cutPtArr);
        //�ϥ�3*3�y�Шt����x�}�A�N�����y�Шt���y���I�ഫ�^�T���Ŷ��y���I
        void SpaceRotationGetArray(double *D, std::vector<DPoint> &cutPtArr);
        //�D3*3�x�}���f�x�}
        void MatrixInverse(DPoint p1, DPoint p2, DPoint p3, double *matrix3_3);
        //�Ŷ��T�I�D��߮y��
        DPoint SpaceCircleCenterCalculation(DPoint p1, DPoint p2, DPoint p3);
        //2D�Ŷ��D��ߡB�b�|�B����
        DPoint ArcCentCalculation_2D(DPoint p1, DPoint p2, DPoint p3, double &radius, double &angle);
        //�Ŷ��T�I�D�x�ήy��
        BOOL SpaceRectanglePointCalculation(DPoint p1, DPoint p2, DPoint pA, RectangleSpace& Rectangle, LONG Type);
        //�Ŷ��T�I�D�x�ήy��180�פ�
        BOOL SpaceRectanglePointCalculationInside180(DPoint p1, DPoint p2, DPoint pA, DPoint pCen, double radius, RectangleSpace& Rectangle);
        //�Ŷ��T�I�D�x�ήy��180�ץ~
        BOOL SpaceRectanglePointCalculationOutside180(DPoint p1, DPoint p2, DPoint pA, DPoint pCen, double radius, RectangleSpace& Rectangle);
        //�Ŷ��T�I�D�x�ήy��180��
        BOOL SpaceRectanglePointCalculation180(DPoint p1, DPoint p2, DPoint pA, DPoint pCen, double radius, RectangleSpace& Rectangle);
        //�Ŷ��T�I�D�x�ήy��360��
        BOOL SpaceRectanglePointCalculation360(DPoint p1, DPoint p2, DPoint pA, DPoint pCen, double radius, RectangleSpace& Rectangle);
};
