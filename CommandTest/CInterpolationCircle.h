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
        //3D �꩷���I
        void ArcCutPoint(AxeSpace sp, AxeSpace mp, AxeSpace ep, std::vector<AxeSpace> &cutPointsArr, LONG backDistance = 0);
        //3D ����I
        void CircleCutPoint(AxeSpace sp, AxeSpace mp1, AxeSpace mp2, std::vector<AxeSpace> &cutPointsArr, LONG backDistance = 0);
        //type_true:��/false:�꩷
        void CircleCutPath_2D_unit(AxeSpace P1, AxeSpace P2, AxeSpace P3, BOOL type, LONG cutUnit, std::vector<AxeSpace> &cutPathArray);
        //type_true:��/false:�꩷
        void CircleCutPath_2D_radian(AxeSpace P1, AxeSpace P2, AxeSpace P3, BOOL type, DOUBLE cutRad, std::vector<AxeSpace> &cutPathArray);
    public:
        //2*2����x�}
        DPoint rotationMatrix2_2(DPoint P, DOUBLE rad);
        void rotationMatrix2_2(DOUBLE *point, DOUBLE rad);
        //�T�I�D�k�V�q
        void FindNormalVector(DPoint p1, DPoint p2, DPoint p3, DOUBLE &a, DOUBLE &b, DOUBLE &c, DOUBLE &d);
        void FindNormalVector(DOUBLE *d1, DOUBLE *d2, DOUBLE *d3, DOUBLE *normalVector);
        //3*3�y�Шt����x�} matrix3_3:1D array
        void RotateTransformMatrix(DOUBLE a, DOUBLE b, DOUBLE c, DOUBLE d, DOUBLE *matrix3_3);
        //�שԱ����ܴ�
        DPoint EulerRotation(DPoint startPoint, DOUBLE *matrix3_3, BOOL mat_inverse);
        //��@��������@���רD���I�A�o�꩷���I�x�}
        void PlaneRotationGetArray(DPoint Pe, DOUBLE r, BOOL Rev, std::vector<DPoint> &cutPtArr, LONG backDistance = 0);
        void PlaneRotationGetArray(DPoint Pe, DPoint Pm2e, DOUBLE r, BOOL Rev, DOUBLE endRad, std::vector<DPoint> &cutPtArr);
        //�ϥ�3*3�y�Шt����x�}�A�N�����y�Шt���y���I�ഫ�^�T���Ŷ��y���I
        void SpaceRotationGetArray(DOUBLE *D, std::vector<DPoint> &cutPtArr);
        //�D3*3�x�}���f�x�}
        void MatrixInverse(DPoint p1, DPoint p2, DPoint p3, DOUBLE *matrix3_3);
        //�Ŷ��T�I�D��߮y��
        DPoint SpaceCircleCenterCalculation(DPoint p1, DPoint p2, DPoint p3);
        //�Ŷ��T�I�D��߮y�СB�b�|�B����
        DPoint SpaceCircleCenterCalculation(DPoint p1, DPoint p2, DPoint p3, DOUBLE &radius, DOUBLE &angle);
        //2D�Ŷ��D��ߡB�b�|�B����
        DPoint ArcCentCalculation_2D(DPoint p1, DPoint p2, DPoint p3, DOUBLE &radius, DOUBLE &angle);
        //2D�Ŷ��D��ߡB�b�|�B���סB���f�ɰw
        DPoint ArcCentCalculation_2D(DPoint p1, DPoint p2, DPoint p3, DOUBLE &radius, DOUBLE &angle, BOOL &rev);
        //�Ŷ��T�I�D�x�ήy��
        BOOL SpaceRectanglePointCalculation(DPoint p1, DPoint p2, DPoint pA, RectangleSpace &Rectangle, LONG Type);
        //�Ŷ��T�I�D�x�ήy��180�פ�
        BOOL SpaceRectanglePointCalculationInside180(DPoint p1, DPoint p2, DPoint pA, DPoint pCen, DOUBLE radius, RectangleSpace &Rectangle);
        //�Ŷ��T�I�D�x�ήy��180�ץ~
        BOOL SpaceRectanglePointCalculationOutside180(DPoint p1, DPoint p2, DPoint pA, DPoint pCen, DOUBLE radius, RectangleSpace &Rectangle);
        //�Ŷ��T�I�D�x�ήy��180��
        BOOL SpaceRectanglePointCalculation180(DPoint p1, DPoint p2, DPoint pA, DPoint pCen, DOUBLE radius, RectangleSpace &Rectangle);
        //�Ŷ��T�I�D�x�ήy��360��
        BOOL SpaceRectanglePointCalculation360(DPoint p1, DPoint p2, DPoint pA, DPoint pCen, DOUBLE radius, RectangleSpace &Rectangle);

};
